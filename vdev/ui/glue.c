// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/vdriver.h>
#include <umber.h>

#include <assert.h>
#include <unistd.h>

#define SPEC "aquabsd.black.ui"
#define VERS 0
#define VDRIVER_HUMAN "UI driver"

vdriver_t VDRIVER;

static umber_class_t const* cls = NULL;
static vid_t only_vid;

static void init(void) {
	cls = umber_class_new(SPEC, UMBER_LVL_WARN, "aqua.black.ui UI VDRIVER.");
	assert(cls != NULL);
}

static void probe(void) {
	assert(VDRIVER.notif_cb != NULL);

	only_vid = VDRIVER.vdev_id_lo;

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_ATTACH,
		.attach.vdev = {
			.kind = KOS_VDEV_KIND_LOCAL,
			.spec = SPEC,
			.vers = VERS,
			.human = "UI device",
			.vdriver_human = VDRIVER_HUMAN,

			.pref = 0,
			.host_id = VDRIVER.host_id,
			.vdev_id = only_vid,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static kos_const_t const CONSTS[] = {
	{
		.name = "ELEM_KIND_DIV",
		.type = KOS_TYPE_U32,
		.val.u32 = 0,
	},
	{
		.name = "ELEM_KIND_TEXT",
		.type = KOS_TYPE_U32,
		.val.u32 = 1,
	},
};

static kos_fn_t const FNS[] = {
	{
		.name = "create",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 0,
	},
	{
		.name = "destroy",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "ui"},
		},
	},
	{
		.name = "get_root",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "ui"},
		},
	},
	{
		.name = "add_div",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 2,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "parent"},
			{KOS_TYPE_BUF, "semantics"},
		},
	},
	{
		.name = "add_text",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "parent"},
			{KOS_TYPE_BUF, "semantics"},
			{KOS_TYPE_BUF, "text"},
		},
	},
	{
		.name = "set_attr_str",
		.ret_type = KOS_TYPE_BOOL,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "elem"},
			{KOS_TYPE_BUF, "key"},
			{KOS_TYPE_BUF, "val"},
		},
	},
	{
		.name = "set_attr_u32",
		.ret_type = KOS_TYPE_BOOL,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "elem"},
			{KOS_TYPE_BUF, "key"},
			{KOS_TYPE_U32, "val"},
		},
	},
	{
		.name = "set_attr_f32",
		.ret_type = KOS_TYPE_BOOL,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "elem"},
			{KOS_TYPE_BUF, "key"},
			{KOS_TYPE_F32, "val"},
		},
	},
	{
		.name = "set_attr_opaque_ptr",
		.ret_type = KOS_TYPE_BOOL,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "elem"},
			{KOS_TYPE_BUF, "key"},
			{KOS_TYPE_OPAQUE_PTR, "val"},
		},
	},
	{
		.name = "set_attr_dim",
		.ret_type = KOS_TYPE_BOOL,
		.param_count = 4,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "elem"},
			{KOS_TYPE_BUF, "key"},
			{KOS_TYPE_U32, "units"},
			{KOS_TYPE_F32, "val"},
		},
	},
	{
		.name = "set_attr_raster",
		.ret_type = KOS_TYPE_BOOL,
		.param_count = 5,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "elem"},
			{KOS_TYPE_BUF, "key"},
			{KOS_TYPE_U32, "x_res"},
			{KOS_TYPE_U32, "y_res"},
			{KOS_TYPE_BUF, "data"},
		},
	},
	// WebGPU backend specific stuff.
	{
		.name = "backend_wgpu_init",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 5,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "ui"},
			{KOS_TYPE_U64, "hid"},
			{KOS_TYPE_U64, "cid"},
			{KOS_TYPE_OPAQUE_PTR, "device"},
			{KOS_TYPE_U32, "format"},
		},
	},
	{
		.name = "backend_wgpu_render",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 5,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "ui"},
			{KOS_TYPE_OPAQUE_PTR, "frame"},
			{KOS_TYPE_OPAQUE_PTR, "command_encoder"},
			{KOS_TYPE_U32, "x_res"},
			{KOS_TYPE_U32, "y_res"},
		},
	},
};

static void conn(kos_cookie_t cookie, vid_t vid, uint64_t conn_id) {
	assert(VDRIVER.notif_cb != NULL);

	if (vid != only_vid) {
		kos_notif_t const notif = {
			.kind = KOS_NOTIF_CONN_FAIL,
			.cookie = cookie,
		};

		VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
		return;
	}

	kos_notif_t const notif = {
		.kind = KOS_NOTIF_CONN,
		.conn_id = conn_id,
		.cookie = cookie,
		.conn = {
			.const_count = sizeof(CONSTS) / sizeof(*CONSTS),
			.consts = CONSTS,
			.fn_count = sizeof(FNS) / sizeof(*FNS),
			.fns = FNS,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

extern uintptr_t GoUiCreate(void);
extern void GoUiDestroy(uintptr_t ui);
extern uintptr_t GoUiGetRoot(uintptr_t ui);
extern uintptr_t GoUiAddDiv(uintptr_t parent, char const* semantics, size_t semantics_len);
extern uintptr_t GoUiAddText(uintptr_t parent, char const* semantics, size_t semantics_len, char const* text, size_t text_len);
extern bool GoUiSetAttrStr(uintptr_t elem, char const* key, size_t key_len, char const* val, size_t val_len);
extern bool GoUiSetAttrU32(uintptr_t elem, char const* key, size_t key_len, uint32_t val);
extern bool GoUiSetAttrF32(uintptr_t elem, char const* key, size_t key_len, float val);
extern bool GoUiSetAttrOpaquePtr(uintptr_t elem, char const* key, size_t key_len, void* val);
extern bool GoUiSetAttrDim(uintptr_t elem, char const* key, size_t key_len, uint32_t units, float val);
extern bool GoUiSetAttrRaster(uintptr_t elem, char const* key, size_t key_len, uint32_t x_res, uint32_t y_res, void const* data);

extern void GoUiBackendWgpuInit(uintptr_t ui, uint64_t hid, uint64_t cid, void* device, uint32_t format);
extern void GoUiBackendWgpuRender(uintptr_t ui, void* frame, void* command_encoder, uint32_t x_res, uint32_t y_res);

static void call(kos_cookie_t cookie, vid_t vdev_id, uint64_t conn_id, uint64_t fn_id, kos_val_t const* args) {
	(void) vdev_id;

	assert(VDRIVER.notif_cb != NULL);

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CALL_RET,
		.conn_id = conn_id,
		.cookie = cookie,
	};

	void* ui = NULL;
	void* elem = NULL;

	// TODO All these breaks should be returning CALL_FAIL.

	switch (fn_id) {
	case 0:;
		ui = (void*) GoUiCreate();
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(ui);
		break;
	case 1:;
		ui = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (ui == NULL) {
			LOG_E(cls, "'destroy' called with non-local or NULL UI.");
			break;
		}

		GoUiDestroy((uintptr_t) ui);
		break;
	case 2:
		ui = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (ui == NULL) {
			LOG_E(cls, "'get_root' called with non-local or NULL UI.");
			break;
		}

		void* const root = (void*) GoUiGetRoot((uintptr_t) ui);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(root);
		break;
	case 3:
		ui = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (ui == NULL) {
			LOG_E(cls, "'add_div' called with non-local or NULL UI.");
			break;
		}

		void* const div = (void*) GoUiAddDiv(
			(uintptr_t) ui,
			(char const*) args[1].buf.ptr,
			args[1].buf.size
		);

		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(div);
		break;
	case 4:
		ui = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (ui == NULL) {
			LOG_E(cls, "'add_text' called with non-local or NULL UI.");
			break;
		}

		void* const text = (void*) GoUiAddText(
			(uintptr_t) ui,
			(char const*) args[1].buf.ptr,
			args[1].buf.size,
			(char const*) args[2].buf.ptr,
			args[2].buf.size
		);

		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(text);
		break;
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
		elem = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (elem == NULL) {
			LOG_E(cls, "'set_attr' called with non-local or NULL element.");
			break;
		}

		switch (fn_id) {
		case 5:
			notif.call_ret.ret.b = GoUiSetAttrStr(
				(uintptr_t) elem,
				(char const*) args[1].buf.ptr,
				args[1].buf.size,
				(char const*) args[2].buf.ptr,
				args[2].buf.size
			);
			break;
		case 6:
			notif.call_ret.ret.b = GoUiSetAttrU32(
				(uintptr_t) elem,
				(char const*) args[1].buf.ptr,
				args[1].buf.size,
				args[2].u32
			);
			break;
		case 7:
			notif.call_ret.ret.b = GoUiSetAttrF32(
				(uintptr_t) elem,
				(char const*) args[1].buf.ptr,
				args[1].buf.size,
				args[2].f32
			);
			break;
		case 8:;
			void* const ptr = vdriver_unwrap_local_opaque_ptr(args[2].opaque_ptr);

			if (ptr == NULL) {
				LOG_E(cls, "'set_attr_opaque_ptr' called with non-local or NULL opaque pointer.");
				break;
			}

			notif.call_ret.ret.b = GoUiSetAttrOpaquePtr(
				(uintptr_t) elem,
				(char const*) args[1].buf.ptr,
				args[1].buf.size,
				ptr
			);
			break;
		case 9:
			notif.call_ret.ret.b = GoUiSetAttrDim(
				(uintptr_t) elem,
				(char const*) args[1].buf.ptr,
				args[1].buf.size,
				args[2].u32,
				args[3].f32
			);
			break;
		case 10:;
			uint32_t const x_res = args[2].u32;
			uint32_t const y_res = args[3].u32;
			kos_val_t const data = args[4];

			assert(x_res * y_res * 4 == data.buf.size);

			notif.call_ret.ret.b = GoUiSetAttrRaster(
				(uintptr_t) elem,
				(char const*) args[1].buf.ptr,
				args[1].buf.size,
				x_res,
				y_res,
				data.buf.ptr
			);
			break;
		default:
			assert(false);
		}

		break;
	// WebGPU backend specific stuff.
	case 11:
		ui = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (ui == NULL) {
			LOG_E(cls, "'destroy' called with non-local or NULL UI.");
			break;
		}

		void* const device = vdriver_unwrap_local_opaque_ptr(args[3].opaque_ptr);

		if (device == NULL) {
			LOG_E(cls, "'backend_wgpu_init' called with non-local or NULL WebGPU device. Rendering to remote WebGPU devices will come in the future.");
			break;
		}

		uint32_t const format = args[4].u32;

		GoUiBackendWgpuInit((uintptr_t) ui, args[1].u64, args[2].u64, device, format);
		break;
	case 12:
		ui = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (ui == NULL) {
			LOG_E(cls, "'backend_wgpu_render' called with non-local or NULL UI.");
			break;
		}

		void* const frame_encoder = vdriver_unwrap_local_opaque_ptr(args[1].opaque_ptr);
		void* const command_encoder = vdriver_unwrap_local_opaque_ptr(args[2].opaque_ptr);

		if (frame_encoder == NULL || command_encoder == NULL) {
			LOG_E(cls, "'backend_wgpu_render' called with non-local or NULL frame or command encoder. This will be supported in the future.");
			break;
		}

		uint32_t const x_res = args[3].u32;
		uint32_t const y_res = args[4].u32;

		GoUiBackendWgpuRender((uintptr_t) ui, frame_encoder, command_encoder, x_res, y_res);
		break;
	default:
		assert(false); // TODO This should probably return CALL_FAIL or something.
	}

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

vdriver_t VDRIVER = {
	.spec = SPEC,
	.human = VDRIVER_HUMAN,
	.vers = VERS,
	.init = init,
	.probe = probe,
	.conn = conn,
	.call = call,
};
