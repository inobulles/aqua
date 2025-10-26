// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/font.h>

#include <umber.h>

#include <string.h>

// XXX Note that this is only meant to test the font library and VDRIVER!
// We're *not* trying to test Pango too or anything else implementation specific, so the exact numbers for e.g. font resolutions can be a little squishy so small changes to Pango don't make these tests flaky or anything.

int main(void) {
	umber_class_t const* const cls = umber_class_new("aquabsd.black.font.tests", UMBER_LVL_VERBOSE, "aquabsd.black.font Font VDRIVER tests.");

	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		LOG_F(cls, "Failed to initialize AQUA library.");
		return EXIT_FAILURE;
	}

	// Get the best font VDEV.

	kos_vdev_descr_t* const font_vdev = aqua_get_best_vdev(font_init(ctx));

	if (font_vdev == NULL) {
		LOG_F(cls, "No font VDEV found.");
		return EXIT_FAILURE;
	}

	LOG_V(cls, "Using font VDEV \"%s\".", (char*) font_vdev->human);
	font_ctx_t font_ctx = font_conn(font_vdev);

	if (font_ctx == NULL) {
		LOG_F(cls, "Failed to connect to font VDEV.");
		return EXIT_FAILURE;
	}

	// XXX Note that there is no situation where font_from_str would actually return NULL in practice, even with a bogus string representation of a font.

	LOG_I(cls, "font_from_str: Testing font creation...");

	font_t font = font_from_str(font_ctx, "Montserrat");

	if (font == NULL) {
		LOG_F(cls, "Expected a font object.");
		return EXIT_FAILURE;
	}

	LOG_I(cls, "font_layout_create: Testing layout creation...");

	font_layout_t layout = font_layout_create(font, "Hello world!");

	if (layout == NULL) {
		LOG_F(cls, "Expected a layout object.");
		return EXIT_FAILURE;
	}

	LOG_I(cls, "font_layout_get_res: Testing getting font resolution...");

	uint32_t x_res, y_res;
	font_layout_get_res(layout, &x_res, &y_res);

	if (
		x_res <= y_res ||             // A priori our text should be wider than it is high.
		x_res < 50 || y_res < 10 ||   // This would be quite small.
		x_res > 500 || y_res > 100 || // This would be quite big.
		0
	) {
		LOG_F(cls, "Got weird layout resolution (%ux%u).", x_res, y_res);
		return EXIT_FAILURE;
	}

	LOG_I(cls, "font_layout_set_text: Testing setting text...");

	char const* const new_text = "Hello world! and some other stuff";
	font_layout_set_text(layout, new_text);
	uint32_t new_x_res, new_y_res;
	font_layout_get_res(layout, &new_x_res, &new_y_res);

	if (new_y_res != y_res || new_x_res <= x_res) {
		LOG_F(cls, "Layout resolution changed unexpectedly (%ux%u -> %ux%u, Y resolution should stay the same and X resolution should increase).", x_res, y_res, new_x_res, new_y_res);
		return EXIT_FAILURE;
	}

	LOG_I(cls, "font_layout_set_limits: Testing setting limits to font size...");

	font_layout_set_limits(layout, new_x_res - 10, new_y_res);
	font_layout_get_res(layout, &x_res, &y_res);

	if (x_res > new_x_res - 10 || y_res < new_y_res) {
		LOG_F(cls, "Layout resolution changed unexpectedly (%ux%u -> %ux%u, Y resolution should stay the same or incrase and X resolution should decrease by at least 10 pixels).", x_res, y_res, new_x_res, new_y_res);
		return EXIT_FAILURE;
	}

	LOG_I(cls, "font_layout_pos_to_index: Testing turning position to index (beginning)...");

	int32_t index = font_layout_pos_to_index(layout, 0, 0);

	if (index != 0) {
		LOG_F(cls, "Expected index for (0, 0) to be at 0th character (was at %dth).", index);
		return EXIT_FAILURE;
	}

	LOG_I(cls, "font_layout_pos_to_index: Testing turning position to index (end)...");

	font_layout_set_limits(layout, -1, -1);
	font_layout_get_res(layout, &x_res, &y_res);

	index = font_layout_pos_to_index(layout, x_res - 1, y_res - 1);
	int32_t expected_index = strlen(new_text) - 1;

	if (index != expected_index) {
		LOG_F(cls, "Expected index for (%u, %u) to be at %dth character (was at %dth).", x_res - 1, y_res - 1, expected_index, index);
		return EXIT_FAILURE;
	}

	LOG_I(cls, "font_layout_index_to_pos: Testing turning index to position (beginning and end)...");

	uint32_t x_start, y_start, x_end, y_end;
	font_layout_index_to_pos(layout, 0, &x_start, &y_start);
	font_layout_get_res(layout, &x_res, &y_res);
	font_layout_index_to_pos(layout, strlen(new_text) - 1, &x_end, &y_end);

	if (x_end <= x_start) {
		LOG_F(cls, "Expected end index X (%u) to be greater than start index X (%u).", x_end, x_start);
		return EXIT_FAILURE;
	}

	if (y_start > y_res || y_end > y_res) {
		LOG_F(cls, "Index positions (%u, %u) or (%u, %u) are outside layout bounds (%ux%u).", x_start, y_start, x_end, y_end, x_res, y_res);
		return EXIT_FAILURE;
	}

	LOG_I(cls, "font_layout_render: Testing rendering layout into buffer...");

	size_t const buf_size = x_res * y_res * 4;
	uint8_t* const buf = calloc(1, buf_size);

	if (buf == NULL) {
		LOG_F(cls, "Failed to allocate %zu bytes for render buffer.", buf_size);
		return EXIT_FAILURE;
	}

	font_layout_render(layout, buf);

	bool non_zero = false;

	for (size_t i = 0; i < buf_size; ++i) {
		if (buf[i] != 0) {
			non_zero = true;
			break;
		}
	}

	free(buf);

	if (!non_zero) {
		LOG_F(cls, "Rendered buffer appears to be empty (all zeroes).");
		return EXIT_FAILURE;
	}

	font_layout_destroy(layout);
	font_destroy(font);

	LOG_I(cls, "All tests passed!");

	return EXIT_SUCCESS;
}
