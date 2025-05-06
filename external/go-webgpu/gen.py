# This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
# Copyright (c) 2025 Aymeric Wibo

import os
import re

GIT_URL = "https://github.com/obiwac/webgpu"
GIT_BRANCH = "feature/v24"

# Try to build at least once to ensure dependencies are downloaded.
# This might fail, which is okay.

os.system("bob build")

# Clone the Go WebGPU bindings.

if not os.path.exists("webgpu"):
	os.system(f"git clone {GIT_URL} --depth 1 --branch {GIT_BRANCH} webgpu")

# Pull out the non-"_js.go" files from wgpu/ (but not vendor.go).

os.system("find webgpu/wgpu -type f ! -name '*_js.go' -exec cp {} . \\;")
os.remove("vendor.go")

# Replace includes to WebGPU headers.

for f in os.listdir("."):
	if not f.endswith(".go"):
		continue

	print(f"Processing {f}...")

	with open(f, "r") as file:
		code = file.read()

	# Replace includes.

	code = re.sub(
		r"#include \"\./lib/w??gpu\.h\"",
		"#include <aqua/wgpu.h>",
		code
	)

	# Replace WebGPU C calls.

	code = re.sub(
		r"C.wgpu([A-Z]\w+)\s*\(",
		lambda m: f"C.aqua_wgpu{m.group(1)}(global_ctx.ctx, ",
		code
	)

	# Write out the modified code.

	with open(f, "w") as file:
		file.write(code)

# Make sure everything builds.

if os.system("bob build") != 0:
	print("Building the Go WebGPU bindings failed!")
	exit(1)
