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

	LOG_I(cls, "Using font VDEV \"%s\".", (char*) font_vdev->human);
	font_ctx_t font_ctx = font_conn(font_vdev);

	if (font_ctx == NULL) {
		LOG_F(cls, "Failed to connect to font VDEV.");
		return EXIT_FAILURE;
	}

	// TODO Actual tests.

	LOG_I(cls, "All tests passed!");

	return EXIT_SUCCESS;
}
