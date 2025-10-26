// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/font.h>

#include <umber.h>

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

	font_layout_set_text(layout, "Hello world! and some other stuff");
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

	font_layout_destroy(layout);
	font_destroy(font);

	LOG_I(cls, "All tests passed!");

	return EXIT_SUCCESS;
}
