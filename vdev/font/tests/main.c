// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/font.h>

#include <umber.h>

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

	font_destroy(font);

	LOG_I(cls, "All tests passed!");

	return EXIT_SUCCESS;
}
