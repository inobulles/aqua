// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/test.h>

#include <umber.h>

int main(void) {
	umber_class_t const* const cls = umber_class_new("test", UMBER_LVL_VERBOSE, "Testing machine");

	aqua_ctx_t const ctx = aqua_init();

	if (ctx == NULL) {
		LOG_F(cls, "Failed to initialize AQUA library.");
		return EXIT_FAILURE;
	}

	// Get the best test VDEV.

	kos_vdev_descr_t* const test_vdev = aqua_get_best_vdev(test_init(ctx));

	if (test_vdev == NULL) {
		LOG_F(cls, "No test VDEVs found.");
		return EXIT_FAILURE;
	}

	LOG_V(cls, "Using test VDEV \"%s\".", (char*) test_vdev->human);
	test_ctx_t const test_ctx = test_conn(test_vdev);

	if (test_ctx == NULL) {
		LOG_F(cls, "Failed to connect to test VDEV.");
		return EXIT_FAILURE;
	}

	// Test adding 69.

	int const res = test_add(test_ctx, 420);

	if (res != 420 + 69) {
		LOG_F(cls, "Got unexpected result from calling test_add: %d", res);
		test_disconn(test_ctx);
		return EXIT_FAILURE;
	}

	LOG_I(cls, "Tests passed!");

	test_disconn(test_ctx);
	return EXIT_SUCCESS;
}
