// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/root.h>

#include <umber.h>

int main(void) {
	umber_class_t const* const cls = umber_class_new("aquabsd.black.font.tests", UMBER_LVL_VERBOSE, "aquabsd.black.font Font VDRIVER tests.");

	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		LOG_F(cls, "Failed to initialize AQUA library.");
		return EXIT_FAILURE;
	}

	// TODO Actual tests.

	LOG_I(cls, "All tests passed!");

	return EXIT_SUCCESS;
}
