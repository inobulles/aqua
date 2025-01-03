// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#include "conn.h"
#include "internal.h"
#include "query.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>

void* conn_thread(void* arg) {
	conn_t* const conn = arg;

	packet_t buf;
	int len;

	while ((len = recv(conn->sock, &buf, sizeof buf, 0)) > 0) {
		switch (buf.header.type) {
		case ELP:
			fprintf(stderr, "Received ELP from %s:0x%x (host %" PRIx64 ") on TCP. This should not happen!\n", inet_ntoa(conn->addr.sin_addr), ntohs(conn->addr.sin_port), buf.elp.host);
			break;
		case QUERY:
			if (query_res(conn) < 0) {
				goto stop;
			}

			break;
		case CONN_VDEV:
			printf("TODO: GV_CONN_VDEV\n");
			break;
		default:
			break;
		}
	}

stop:

	close(conn->sock);
	conn->slot_used = false;

	return NULL;
}
