# This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
# Copyright (c) 2025 Aymeric Wibo

import os
from datetime import datetime

year = datetime.now().year

BASE_FN_ID = 1

PACKED = "__attribute__((packed))"

# We must first attempt to build once first to ensure dependencies are downloaded.

os.system("bob build")

# WebGPU commands in the spec which aren't yet implemented by wgpu-native.

WGPU_BLACKLIST = (
	"wgpuInstanceHasWGSLLanguageFeature",
	"wgpuSurfaceSetLabel",
)

with open(".bob/prefix/include/webgpu/webgpu.h") as f:
	(*lines,) = map(str.rstrip, f.readlines())

with open(".bob/prefix/include/wgpu.h") as f:
	lines += [*map(str.rstrip, f.readlines())]


def wgpu_type_to_kos(t: str):
	KNOWN = {
		"void": "KOS_TYPE_VOID",
		"size_t": "KOS_TYPE_U32",
		"int32_t": "KOS_TYPE_I32",
		"uint32_t": "KOS_TYPE_U32",
		"uint64_t": "KOS_TYPE_U64",
		"float": "KOS_TYPE_F32",
		"WGPUBool": "KOS_TYPE_BOOL",
		"WGPUMapMode": "KOS_TYPE_U64",
		"void const *": "KOS_TYPE_OPAQUE_PTR",
		"void *": "KOS_TYPE_OPAQUE_PTR",
	}

	if t in KNOWN:
		return KNOWN[t]

	# TODO Since some of these structs can have pointers to other stuff in them and thus can't be serialized/deserialized (which a buf normally has to be), should this not be a ptr instead?

	if (
		t[-1] == "*"
		or t.endswith("CallbackInfo")
		or t in ("WGPUAdapaterInfo", "WGPUSupportedFeatures", "WGPUSupportedWGSLLanguageFeatures")
	):
		return "KOS_TYPE_BUF"

	if t in enums:
		return "KOS_TYPE_U32"

	return "KOS_TYPE_OPAQUE_PTR"


def kos_type_to_union(t: str):
	if t == "KOS_TYPE_BOOL":
		return "b"

	return t.removeprefix("KOS_TYPE_").lower()


enums = set()
fns = ""
call_handlers = ""
lib_protos = ""
lib_fn_ids = ""
lib_fn_validators = ""
lib_impls = ""
fn_id = BASE_FN_ID
cmds = ""  # REMME
impls = ""

c_types = ""
c_wrappers = ""

for line in lines:
	# Parse function declaration.

	if line.startswith('#include "'):
		continue

	if line.endswith(" WGPU_ENUM_ATTRIBUTE;"):
		enums.add(line.split()[-2])
		continue

	if not line.startswith("WGPU_EXPORT "):
		c_types += line + "\n"
		continue

	type_and_name, params = line.split("(")
	_, *ret_type, name = type_and_name.split()
	ret_type = " ".join(ret_type)

	if name in WGPU_BLACKLIST:
		continue

	raw_params = params.split(")")[0]
	params = raw_params.split(", ")
	param_types = []
	param_names = []

	for arg in params:
		*t, arg_name = arg.split()
		param_types.append(" ".join(t).removeprefix("WGPU_NULLABLE "))
		param_names.append(arg_name)

	# Generate function struct.

	fns += f"""\t{{
		.name = "{name}",
		.ret_type = {wgpu_type_to_kos(ret_type)},
		.param_count = {len(param_names)},
		.params = (kos_param_t[]) {{\n"""

	for t, n in zip(param_types, param_names):
		fns += f"""\t\t\t{{
				.type = {wgpu_type_to_kos(t)},
				.name = "{n}",
			}},\n"""

	fns += "\t\t},\n\t},\n"

	# Generate args parser for call handler.

	parser = ""

	for i, p in enumerate(param_names):
		t = param_types[i]
		kos_t = wgpu_type_to_kos(t)

		if t in ("WGPUSurfaceCapabilities", "WGPUAdapterInfo"):
			parser += f"\t\t{t} const {p} = *({t}*) args[{i}].opaque_ptr;\n"

		elif t == "WGPUStringView":
			parser += f"\t\t{t} const {p} = {{\n"
			parser += f"\t\t\t.data = args[{i}].buf.ptr,\n"
			parser += f"\t\t\t.length = args[{i}].buf.size,\n"
			parser += f"\t\t}};\n"

		elif t == "WGPUFuture":
			parser += f"\t\t{t} const {p} = {{ .id = args[{i}].u64 }};\n"

		elif kos_t == "KOS_TYPE_BUF" and t[-1] != "*":
			parser += f"\t\t{t} const {p} = *({t}*) args[{i}].buf.ptr;\n"
			parser += f"\t\tassert(args[{i}].buf.size == sizeof {p});\n"

		elif kos_t == "KOS_TYPE_BUF" and t[-1] == "*":
			parser += f"\t\t{t} const {p} = args[{i}].buf.ptr;\n"
			parser += f"\t\tassert(args[{i}].buf.size == sizeof *{p});\n"

		elif kos_t == "KOS_TYPE_OPAQUE_PTR":
			parser += f"\t\t{t} const {p} = args[{i}].opaque_ptr;\n"

		else:
			union = kos_type_to_union(kos_t)
			parser += f"\t\t{t} const {p} = args[{i}].{union};\n"

	# Generate return code.

	call = f"{name}({", ".join(param_names)})"

	kos_ret_type = wgpu_type_to_kos(ret_type)

	if ret_type == "WGPUFuture":
		ret = f"notif.call_ret.ret.u64 = {call}.id"

	elif ret_type == "WGPUAdapterInfo":
		ret = f"""{ret_type}* const ptr = malloc(sizeof({ret_type}));
		assert(ptr != NULL);
		notif.call_ret.ret.buf.ptr = (void*) ptr;
		notif.call_ret.ret.buf.size = sizeof({ret_type});
		*ptr = {call};"""

	elif kos_ret_type == "KOS_TYPE_VOID":
		ret = call

	elif kos_ret_type == "KOS_TYPE_OPAQUE_PTR":
		ret = f"notif.call_ret.ret.opaque_ptr = (void*) {call}"

	else:
		union = kos_type_to_union(kos_ret_type)
		ret = f"notif.call_ret.ret.{union} = {call}"

	# Generate actual call handler.

	call_handlers += f"""\tcase {fn_id}: {{
{parser}\t\t{ret};
		break;
	}}
"""

	# Generate library prototype and function IDs.

	lib_protos += f"{ret_type} aqua_{name}(wgpu_ctx_t ctx, {raw_params});\n"
	lib_fn_ids += f"\t\tuint32_t {name};\n"

	# Generate function validator.

	param_validators = ""

	for i, p in enumerate(param_names):
		t = param_types[i]
		kos_t = wgpu_type_to_kos(t)

		param_validators += f""" &&
			fn->params[{i}].type == {kos_t} &&
			strcmp((char*) fn->params[{i}].name, "{p}") == 0"""

	lib_fn_validators += f"""
		if (
			strcmp(name, "{name}") == 0 &&
			fn->ret_type == {kos_ret_type} &&
			fn->param_count == {len(param_names)}{param_validators}
		) {{
			ctx->fns.{name} = i;
		}}
"""

	# Generate args generator for library implementation.

	args = []

	for i, p in enumerate(param_names):
		t = param_types[i]
		kos_t = wgpu_type_to_kos(t)

		if t in ("WGPUSurfaceCapabilities", "WGPUAdapterInfo"):
			args.append(f".buf.size = sizeof {p},\n\t\t\t.buf.ptr = &{p},")

		elif kos_t == "KOS_TYPE_BUF":
			args.append(f".buf.size = sizeof *{p},\n\t\t\t.buf.ptr = (void*) {p},")

		elif kos_t == "KOS_TYPE_OPAQUE_PTR":
			args.append(f".opaque_ptr = (void*) {p},")

		else:
			union = kos_type_to_union(kos_t)
			args.append(f".{union} = {p},")

	args = ",\n\t\t".join(map(lambda arg: f"{{\n\t\t\t{arg}\n\t\t}}", args))

	if kos_ret_type == "KOS_TYPE_VOID":
		ret = ""

	elif kos_ret_type == "KOS_TYPE_OPAQUE_PTR":
		ret = f"\n\treturn ctx->last_ret.opaque_ptr;"

	else:
		union = kos_type_to_union(kos_ret_type)
		ret = f"\n\treturn ctx->last_ret.{union};"

	lib_impls += f"""{ret_type} aqua_{name}(wgpu_ctx_t ctx, {raw_params}) {{
	kos_val_t const args[] = {{
		{args},
	}};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.{name}, args);
	kos_flush(true);
	{ret}
}}

"""

	# Done! Move to next function ID.

	fn_id += 1


def inject_src(path: str, tag: str, payload: str):
	with open(path) as f:
		src = f.read().split("\n")

	begin = src.index(f"// {tag}:BEGIN")
	end = src.index(f"// {tag}:END")

	assert begin != -1 and end != -1
	assert begin < end

	src = "\n".join(src[: begin + 1] + payload.strip("\n").split("\n") + src[end:])

	with open(path, "w") as f:
		f.write(src)


# Generate sources for device & library.

inject_src("fns.h", "FNS", fns)
inject_src("main.c", "CALL_HANDLERS", call_handlers)

# Compile device as a sanity check.

os.system("bob build")

# Inject function prototypes into library source.

inject_src("../../lib/wgpu.h", "PROTOS", lib_protos)
inject_src("../../lib/wgpu.c", "FN_IDS", lib_fn_ids)
inject_src("../../lib/wgpu.c", "FN_VALIDATORS", lib_fn_validators)
inject_src("../../lib/wgpu.c", "FNS", lib_impls[:-1])
