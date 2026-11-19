# This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
# Copyright (c) 2026 Aymeric Wibo

# Generates the AQUA constants shared between the WM VDEV and its library.
# Run this from the 'vdev/wm' directory: python gen.py

import os

# Modifiers, as (AQUA name, VDEV value) tuples.

MODS = (
	("SHIFT", "WLR_MODIFIER_SHIFT"),
	("CAPS", "WLR_MODIFIER_CAPS"),
	("CTRL", "WLR_MODIFIER_CTRL"),
	("ALT", "WLR_MODIFIER_ALT"),
	("MOD2", "WLR_MODIFIER_MOD2"),
	("MOD3", "WLR_MODIFIER_MOD3"),
	("META", "WLR_MODIFIER_LOGO"),
	("MOD5", "WLR_MODIFIER_MOD5"),
)

# Backends, as (AQUA name, VDEV value) tuples.

BACKENDS = (
	("DRM", "BACKEND_DRM"),
	("WAYLAND", "BACKEND_WAYLAND"),
	("X11", "BACKEND_X11"),
	("HEADLESS", "BACKEND_HEADLESS"),
)

# All constants exported by the VDEV, as (KOS name, VDEV value, KOS type) tuples.

CONSTS = [(f"MOD_{name}", val, "U32") for name, val in MODS]
CONSTS += [(f"BACKEND_{name}", val, "U8") for name, val in BACKENDS]

# VDEV constants.

vdev_consts = ""

for name, val, kos_type in CONSTS:
	vdev_consts += f"""\t{{
		.name = "{name}",
		.type = KOS_TYPE_{kos_type},
		.val = {{.{kos_type.lower()} = {val}}},
	}},
"""

# Library constant fields & reads.

lib_const_fields = ""
lib_const_reads = ""

for name, _, kos_type in CONSTS:
	lib_const_fields += f"\t\tuint32_t {name};\n"
	lib_const_reads += f"""
		if (strcmp(name, "{name}") == 0) {{
			ctx->consts.{name} = c->val.{kos_type.lower()};
		}}
"""

# Library modifier enum & conversion.

lib_mods = ""
lib_conv_mods = ""
go_mods = ""

for i, (name, _) in enumerate(MODS):
	lib_mods += f"\tWM_MOD_{name} = 1 << {i},\n"
	lib_conv_mods += f"""
	if (vdev_mods & ctx->consts.MOD_{name}) {{
		mods |= WM_MOD_{name};
	}}
"""
	go_mods += f"\tWM_MOD_{name} WmMod = C.WM_MOD_{name}\n"

# Library backend enum & conversion.

lib_backends = ""
lib_conv_backend = ""
go_backends = ""

for name, _ in BACKENDS:
	lib_backends += f"\tWM_BACKEND_{name},\n"
	lib_conv_backend += f"""
	if (vdev_backend == ctx->consts.BACKEND_{name}) {{
		return WM_BACKEND_{name};
	}}
"""
	go_backends += f"\tWM_BACKEND_{name} WmBackend = C.WM_BACKEND_{name}\n"


def inject_src(path: str, tag: str, payload: str):
	with open(path) as f:
		src = f.read().split("\n")

	# Markers may be indented (e.g. by gofmt).

	stripped = [line.strip() for line in src]
	begin = stripped.index(f"// {tag}:BEGIN")
	end = stripped.index(f"// {tag}:END")

	assert begin < end

	src = src[: begin + 1] + payload.strip("\n").split("\n") + src[end:]

	with open(path, "w") as f:
		f.write("\n".join(src))


inject_src("main.c", "CONSTS", vdev_consts)

inject_src("../../lib/wm.h", "MODS", lib_mods)
inject_src("../../lib/wm.c", "CONST_FIELDS", lib_const_fields)
inject_src("../../lib/wm.c", "CONST_READS", lib_const_reads)
inject_src("../../lib/wm.c", "CONV_MODS", lib_conv_mods)
inject_src("../../lib/wm.h", "BACKENDS", lib_backends)
inject_src("../../lib/wm.c", "CONV_BACKEND", lib_conv_backend)

inject_src("../../lib/bindings/go/wm.go", "MODS", go_mods)
inject_src("../../lib/bindings/go/wm.go", "BACKENDS", go_backends)
os.system("gofmt -w ../../lib/bindings/go/wm.go")
