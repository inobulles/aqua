// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include "agent.h"

#include <aqua/gv_proto.h>
#include <aqua/kos.h>

#include <umber.h>

#include <unistd.h>

static umber_class_t const* cls;

int main(int argc, char* argv[]) {
	cls = umber_class_new("gv.agent", UMBER_LVL_WARN, "GrapeVine KOS agent (CLI).");

	int sock = 3; // Set by gvd when spawning us.
	uint64_t vid = -1ull;
	char const* spec = NULL;

	int c;

	while ((c = getopt(argc, argv, "s:v:")) != -1) {
		switch (c) {
		case 's':
			spec = optarg;
			break;
		case 'v':
			vid = atoi(optarg);
			break;
		default:
			LOG_F(cls, "Unknown option: %c", c);
			return EXIT_FAILURE;
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 0) {
		LOG_F(cls, "More arguments than expected.");
		return EXIT_FAILURE;
	}

	if (spec == NULL) {
		LOG_F(cls, "Expected spec argument (-s).");
		return EXIT_FAILURE;
	}

	if (vid == -1ull) {
		LOG_F(cls, "Expected VID argument (-v).");
		return EXIT_FAILURE;
	}

	gv_agent_t* const agent = gv_agent_create(sock, spec, vid);

	if (agent == NULL) {
		return EXIT_FAILURE;
	}

	gv_agent_loop(agent);
	gv_agent_destroy(agent);

	return EXIT_SUCCESS;
}
