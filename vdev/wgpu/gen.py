# This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
# Copyright (c) 2025 Aymeric Wibo

import os
from datetime import datetime

year = datetime.now().year

BASE_FN_ID = 2

PACKED = "__attribute__((packed))"

# WebGPU functions in the spec which aren't yet implemented by wgpu-native.

WGPU_BLACKLIST = (
	"wgpuInstanceHasWGSLLanguageFeature",
	"wgpuSurfaceSetLabel",
)

# These functions aren't supported on Apple OS'.

WGPU_APPLE_BLACKLIST = (
	"wgpuDeviceFromVk",
	"wgpuTextureFromVkImage",
	"wgpuCommandEncoderFromVk",
)

with open("../../external/webgpu-headers/webgpu.h") as f:
	(*lines,) = map(str.rstrip, f.readlines())

with open("../../external/webgpu-headers/wgpu.h") as f:
	lines += [*map(str.rstrip, f.readlines())]


def is_buf_struct(t):
	if t.endswith("CallbackInfo"):
		return True

	return t in (
		"WGPUAdapterInfo",
		"WGPUSupportedFeatures",
		"WGPUSupportedWGSLLanguageFeatures",
		"WGPUSurfaceCapabilities",
	)


def wgpu_type_to_kos(t: str):
	KNOWN = {
		"void": "KOS_TYPE_VOID",
		"size_t": "KOS_TYPE_U32",
		"int32_t": "KOS_TYPE_I32",
		"uint32_t": "KOS_TYPE_U32",
		"uint64_t": "KOS_TYPE_U64",
		"float": "KOS_TYPE_F32",
		"WGPUBool": "KOS_TYPE_BOOL",
		# TODO Just make some list of flag types.
		"WGPUMapMode": "KOS_TYPE_U64",
		"WGPUBufferUsage": "KOS_TYPE_U64",
		"WGPUTextureUsage": "KOS_TYPE_U64",
		"WGPUShaderStage": "KOS_TYPE_U64",
		"WGPUSubmissionIndex": "KOS_TYPE_U64",
		"void const *": "KOS_TYPE_OPAQUE_PTR",
		"void *": "KOS_TYPE_OPAQUE_PTR",
	}

	if t in KNOWN:
		return KNOWN[t]

	# TODO Since some of these structs can have pointers to other stuff in them and thus can't be serialized/deserialized (which a buf normally has to be), should this not be a ptr instead?

	if t[-1] == "*" or is_buf_struct(t):
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

	type_and_name, raw_params = line.split("(")
	_, *ret_type, name = type_and_name.split()
	ret_type = " ".join(ret_type)

	if name in WGPU_BLACKLIST:
		continue

	raw_params = raw_params.split(")")[0]
	params = []
	param_types = []
	param_names = []

	if raw_params != "void":
		params = raw_params.split(", ")

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
		fns += f"""\t\t\t{{{wgpu_type_to_kos(t)}, "{n}"}},\n"""

	fns += "\t\t},\n\t},\n"

	# Generate args parser for call handler.

	parser = ""

	for i, p in enumerate(param_names):
		t = param_types[i]
		kos_t = wgpu_type_to_kos(t)

		if t in ("WGPUSurfaceCapabilities", "WGPUAdapterInfo"):
			parser += f"\t\t{t} const {p} = *({t}*) vdriver_unwrap_local_opaque_ptr(args[{i}].opaque_ptr);\n"

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
			parser += f"\t\t{t} const {p} = (void*) args[{i}].buf.ptr;\n"
			parser += f"\t\tassert(args[{i}].buf.size == sizeof *{p});\n"

		elif kos_t == "KOS_TYPE_OPAQUE_PTR":
			parser += f"\t\t{t} const {p} = vdriver_unwrap_local_opaque_ptr(args[{i}].opaque_ptr);\n"

		else:
			union = kos_type_to_union(kos_t)
			parser += f"\t\t{t} const {p} = args[{i}].{union};\n"

	# Generate return code.

	call = f"{name}({', '.join(param_names)})"
	kos_ret_type = wgpu_type_to_kos(ret_type)

	if ret_type == "WGPUFuture":
		ret = f"notif.call_ret.ret.u64 = {call}.id"

	elif ret_type == "WGPUAdapterInfo":
		ret = f"""{ret_type}* const ptr = malloc(sizeof({ret_type}));
		assert(ptr != NULL);
		notif.call_ret.ret.buf.ptr = ptr;
		notif.call_ret.ret.buf.size = sizeof({ret_type});
		*ptr = {call};"""

	elif kos_ret_type == "KOS_TYPE_VOID":
		ret = call

	elif kos_ret_type == "KOS_TYPE_OPAQUE_PTR":
		ret = f"notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr({call})"

	else:
		union = kos_type_to_union(kos_ret_type)
		ret = f"notif.call_ret.ret.{union} = {call}"

	# Generate actual call handler.
	# TODO The way we're doing the Apple blacklist is not great.
	# Ideally, we should never be advertising this function in the first place.
	# But that makes things difficult with the function ID case statements so let's just ignore this for now.

	if name in WGPU_APPLE_BLACKLIST:
		call_handlers += "#if !defined(__APPLE__)\n"

	call_handlers += f"""\tcase {fn_id}: {{
{parser}\t\t{ret};
		break;
	}}
"""

	if name in WGPU_APPLE_BLACKLIST:
		call_handlers += "#endif\n"

	# Generate library prototype and function IDs.

	lib_fn_sig = f"{ret_type} aqua_{name}({', '.join(['wgpu_ctx_t ctx'] + params)})"
	lib_protos += f"{lib_fn_sig};\n"
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

		if is_buf_struct(t):
			args.append(f".buf.size = sizeof {p},\n\t\t\t.buf.ptr = &{p},")

		elif t == "WGPUStringView":
			args.append(f".buf.size = {p}.length,\n\t\t\t.buf.ptr = (void*) {p}.data,")

		elif kos_t == "KOS_TYPE_BUF":
			args.append(f".buf.size = sizeof *{p},\n\t\t\t.buf.ptr = (void*) {p},")

		elif kos_t == "KOS_TYPE_OPAQUE_PTR":
			args.append(f".opaque_ptr = {{ctx->hid, (uintptr_t) {p}}},")

		else:
			union = kos_type_to_union(kos_t)
			args.append(f".{union} = {p},")

	args = ",\n\t\t".join(map(lambda arg: f"{{\n\t\t\t{arg}\n\t\t}}", args))

	if kos_ret_type == "KOS_TYPE_VOID":
		ret = ""

	elif ret_type == "WGPUFuture":
		ret = f"\n\treturn (WGPUFuture) {{.id = ctx->last_ret.u64}};\n"

	elif ret_type == "WGPUAdapterInfo":
		ret = f"\n\treturn *(WGPUAdapterInfo*) ctx->last_ret.buf.ptr;\n"

	elif kos_ret_type == "KOS_TYPE_OPAQUE_PTR":
		ret = f"""
	assert(ctx->last_ret.opaque_ptr.host_id == ctx->hid);
	return (void*) (uintptr_t) ctx->last_ret.opaque_ptr.ptr;
"""

	else:
		union = kos_type_to_union(kos_ret_type)
		ret = f"\n\treturn ctx->last_ret.{union};\n"

	lib_impls += f"""{lib_fn_sig} {{
	kos_val_t const args[] = {{
		{args}
	}};

	ctx->last_cookie = kos_vdev_call(ctx->conn_id, ctx->fns.{name}, args);
	kos_flush(true);
{ret}}}

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
