// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#pragma once

#include "vdev.h"

typedef struct action action_t;

typedef void (*action_cb_t)(kos_cookie_t cookie, struct action* action);

struct action {
	kos_cookie_t cookie;
	action_cb_t cb;

	union {
		struct {
			uint64_t host_id;
			uint64_t vdev_id;
		} conn;

		struct {
			vdriver_t* vdriver;
			uint64_t conn_id;
			uint32_t fn_id;
			void const* args;
		} call;
	};
};

#define ACTION_QUEUE_SIZE 16

static action_t action_queue[ACTION_QUEUE_SIZE];
static size_t action_queue_head = 0;
static size_t action_queue_tail = 0;

#define QUEUE_INDEX(i) (action_queue[(i) % ACTION_QUEUE_SIZE])

#define PUSH_QUEUE(val)                       \
	do {                                       \
		QUEUE_INDEX(action_queue_tail) = (val); \
		action_queue_tail++;                    \
	} while (0)

#define POP_QUEUE(res)                        \
	do {                                       \
		(res) = QUEUE_INDEX(action_queue_head); \
		action_queue_head++;                    \
	} while (0)
