// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#pragma once

#include <pthread.h>
#include <stdbool.h>

#include <netinet/in.h>
#include <sys/socket.h>

typedef struct state_t state_t;

typedef struct {
	bool slot_used;

	state_t* state;
	pthread_t thread;

	int sock;
	struct sockaddr_in addr;
	socklen_t addr_len;
} conn_t;

void* conn_thread(void* arg);
