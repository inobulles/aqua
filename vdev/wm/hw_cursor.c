// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

#include "wm.h"

#include <wlr/interfaces/wlr_output.h>

// Size to use when outputs don't report any size constraints (e.g. when nested).

#define HW_CURSOR_UNCONSTRAINED_SIZE 256

static void output_get_hw_cursor_max_size(struct wlr_output* output, uint32_t* x_res, uint32_t* y_res) {
	*x_res = *y_res = 0;

	if (output->impl->set_cursor == NULL || output->software_cursor_locks > 0) {
		return;
	}

	if (output->impl->get_cursor_sizes == NULL) {
		*x_res = *y_res = HW_CURSOR_UNCONSTRAINED_SIZE;
		return;
	}

	// Choose largest size cursor plane accepts by area.

	size_t len = 0;
	struct wlr_output_cursor_size const* const sizes = output->impl->get_cursor_sizes(output, &len);

	if (sizes == NULL) {
		return;
	}

	for (size_t i = 0; i < len; i++) {
		uint64_t const area = (uint64_t) sizes[i].width * sizes[i].height;

		if (area > (uint64_t) *x_res * *y_res) {
			*x_res = sizes[i].width;
			*y_res = sizes[i].height;
		}
	}
}

void wm_vdev_get_hw_cursor_max_size(wm_t* wm, uint32_t* x_res, uint32_t* y_res) {
	// Go through outputs and find the smallest size supported by all of em.
	// XXX I guess it's technically possible for a max size on output A to be smaller than a max size on output B but that that size is not in output->impl->get_cursor_sizes() but eh.

	*x_res = *y_res = UINT32_MAX;
	output_t* output;

	wl_list_for_each(output, &wm->outputs, link) {
		uint32_t output_x_res, output_y_res;
		output_get_hw_cursor_max_size(output->output, &output_x_res, &output_y_res);

		*x_res = output_x_res < *x_res ? output_x_res : *x_res;
		*y_res = output_y_res < *y_res ? output_y_res : *y_res;
	}

	if (*x_res == UINT32_MAX || *x_res == 0 || *y_res == 0) {
		*x_res = *y_res = 0;
	}
}
