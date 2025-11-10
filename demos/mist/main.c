// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/vr.h>
#include <aqua/wm.h>

#include <umber.h>

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TILES_X 64
#define TILES_Y 64
#define TILE_COUNT (TILES_X * TILES_Y)

// #define NO_VR

typedef struct {
	wm_win_t win;
	char* app_id;
	uint32_t x_res;
	uint32_t y_res;
	void* fb;

	uint64_t tile_hashes[TILE_COUNT];
} thin_win_t;

typedef struct {
	size_t win_count;
	thin_win_t* wins;
	vr_ctx_t vr_ctx;
} state_t;

// Thinking about vitrification:
// Initially, get_win_fb will just return a BUF (already make NALus?).
// We will want it to eventually return a PTR, however.
// This pointer might be in the client memory's memory space, or it might be some handle to a DMA-BUF.
// TODO Should pointers contain the memory space they are in? Or should they be completely generic? (E.g. just pointing to some VDEV-defined struct, which represents both client memory or the DMA-BUF fd to support both easily.)
// This pointer will be sent over to the .vr device, at which point *it* will vitrify it I think.
// Since this pointer contains the host (+ connection?) it is on, the .vr device will be able to know to establish a connection with that host, so this would work even if .wm was on a different GV node (this will be the tricky part).
// Over this connection, .vr will say e.g. "oh I am perfectly fine with lossy encoding of this data".
// .wm will say "bet", and then on its side will turn that PTR into a stream of NALus (over UDP or better yet RTP), encoding the DMA-BUFs with VA-API (notice how we never had to drop down to client memory).
// Alternatively, if .vr said "no, I want proper frames", then .wm would have to simply turn its DMA-BUFs into regular memory and send that over in order (i.e. TCP not UDP).
// But I won't support that situation for now as that can anyway be handled with just passing BUFs around.
// Okay so .vr receives those NALus on its side, and then can immediately decode them into GPU textures using whatever (e.g. Android media decoder).

static void new_win(wm_t wm, wm_win_t win, char const* app_id, void* data) {
	state_t* const s = data;

	(void) wm;

	s->wins = realloc(s->wins, ++s->win_count * sizeof *s->wins);
	assert(s->wins != NULL);

	thin_win_t* const thin_win = &s->wins[s->win_count - 1];
	memset(thin_win, 0, sizeof *thin_win);

	thin_win->win = win;
	thin_win->app_id = strdup(app_id == NULL ? "" : app_id);
	assert(thin_win->app_id != NULL);

	printf("new win %s\n", app_id);
}

static void destroy_win(wm_t wm, wm_win_t win, void* data) {
	state_t* const s = data;

	(void) wm;
	(void) win;
	(void) s;

	printf("%s: TODO\n", __func__);

	vr_destroy_win(s->vr_ctx, win);
}

static void redraw_win(wm_t wm, wm_win_t win, uint32_t x_res, uint32_t y_res, void* data) {
	state_t* const s = data;
	thin_win_t* thin_win = NULL;

	for (size_t i = 0; i < s->win_count; i++) {
		thin_win = &s->wins[i];

		if (thin_win->win == win) {
			break;
		}
	}

	assert(thin_win != NULL);

	// If resolution changed, toss old buffer and create new one.
	// This triggers a full refresh of all tiles.

	bool full_refresh = false;

	if (x_res != thin_win->x_res || y_res != thin_win->y_res) {
		full_refresh = true;

		free(thin_win->fb);
		thin_win->fb = malloc(x_res * y_res * 4);
		assert(thin_win->fb != NULL);

		thin_win->x_res = x_res;
		thin_win->y_res = y_res;
	}

	// Then, get new window.

	uint32_t* const fb = thin_win->fb;
	wm_get_win_fb(wm, win, fb);

	// At this point we gotta check which tiles have changed by comparing their current and previous hashes.
	// If they have changed add them to our buffer already.
	// TODO EXCEPT if we gotta refresh all tiles obviously.

	(void) full_refresh;

	uint64_t* const tile_bitmap = calloc(1, TILE_COUNT / 8);
	assert(tile_bitmap != NULL);

	size_t excepted_update_count = 0;

	size_t buf_sz = 0;
	void* buf = NULL;

	size_t const tile_x_res = x_res / TILES_X;
	size_t const tile_y_res = y_res / TILES_Y;

	for (size_t i = 0; i < TILES_Y; i++) {
		for (size_t j = 0; j < TILES_X; j++) {
			uint64_t hash = 0;

			for (size_t y = tile_y_res * i; y < tile_y_res * (i + 1); y++) {
				for (size_t x = tile_x_res * j; x < tile_x_res * (j + 1); x++) {
					uint32_t const pixel = fb[y * x_res + x];
					hash = hash * 33 + pixel;
				}
			}

			size_t const tile_index = i * TILES_X + j;

			if (thin_win->tile_hashes[tile_index] == hash) {
				continue;
			}

			tile_bitmap[tile_index / 64] |= 1ull << (tile_index % 64);
			thin_win->tile_hashes[tile_index] = hash;

			// Write updated tile to our buffer.

			size_t counter = buf_sz;
			buf_sz += tile_x_res * tile_y_res * 4;
			buf = realloc(buf, buf_sz);
			assert(buf != NULL);

			for (size_t y = tile_y_res * i; y < tile_y_res * (i + 1); y++) {
				for (size_t x = tile_x_res * j; x < tile_x_res * (j + 1); x++) {
					*((uint32_t*) (buf + counter)) = fb[y * x_res + x];
					counter += 4;
				}
			}

			assert(counter == buf_sz);
			excepted_update_count++;
		}
	}

	size_t tile_update_count = 0;

	for (size_t i = 0; i < TILES_X * TILES_Y / sizeof *tile_bitmap / 8; i++) {
		tile_update_count += __builtin_popcountll(tile_bitmap[i]);
	}

	assert(excepted_update_count == tile_update_count);

	// Send over to VR device.

#if !defined(NO_VR)
	vr_send_win(s->vr_ctx, win, x_res, y_res, TILES_X, TILES_Y, tile_bitmap, buf_sz, buf);
	kos_flush(true);
#endif

	free(buf);
	free(tile_bitmap);
}

int main(void) {
	state_t state = {};

	umber_class_t const* const cls = umber_class_new("mist", UMBER_LVL_VERBOSE, "Mist WM demo");

	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		LOG_F(cls, "Failed to initialize AQUA library.");
		return EXIT_FAILURE;
	}

	// Get the best VR VDEV.

#if !defined(NO_VR)
	kos_vdev_descr_t* const vr_vdev = aqua_get_best_vdev(vr_init(ctx));

	if (vr_vdev == NULL) {
		LOG_F(cls, "No VR VDEV found.");
		return EXIT_FAILURE;
	}

	LOG_V(cls, "Using VR VDEV \"%s\".", (char*) vr_vdev->human);
	state.vr_ctx = vr_conn(vr_vdev);

	if (state.vr_ctx == NULL) {
		LOG_F(cls, "Failed to connect to VR VDEV.");
		return EXIT_FAILURE;
	}
#endif

	// Get the best WM VDEV.

	kos_vdev_descr_t* const wm_vdev = aqua_get_best_vdev(wm_init(ctx));

	if (wm_vdev == NULL) {
		LOG_F(cls, "No WM VDEV found.");
		return EXIT_FAILURE;
	}

	LOG_V(cls, "Using WM VDEV \"%s\".", (char*) wm_vdev->human);
	wm_ctx_t const wm_ctx = wm_conn(wm_vdev);

	if (wm_ctx == NULL) {
		LOG_F(cls, "Failed to connect to WM VDEV.");
		return EXIT_FAILURE;
	}

	// Create WM.

	int rv = EXIT_FAILURE;
	wm_t const wm = wm_create(wm_ctx);

	if (wm == NULL) {
		LOG_F(cls, "Failed to create WM.");
		goto disconn;
	}

	// Loop.

	wm_register_new_win_cb(wm, new_win, &state);
	wm_register_destroy_win_cb(wm, destroy_win, &state);
	wm_register_redraw_win_cb(wm, redraw_win, &state);

	wm_loop(wm);

	// Clean up.

	wm_destroy(wm);

	rv = EXIT_SUCCESS;

disconn:

	wm_disconn(wm_ctx);

	return rv;
}
