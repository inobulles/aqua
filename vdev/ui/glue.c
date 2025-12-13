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
	// WebGPU backend specific stuff.
	{
		.name = "backend_wgpu_init",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 4,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "ui"},
			{KOS_TYPE_U64, "hid"},
			{KOS_TYPE_U64, "cid"},
			{KOS_TYPE_OPAQUE_PTR, "device"},
		},
	},
	{
		.name = "backend_wgpu_render",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 4,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "ui"},
			{KOS_TYPE_OPAQUE_PTR, "frame"},
			{KOS_TYPE_OPAQUE_PTR, "command_encoder"},
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

extern void GoUiBackendWgpuInit(uintptr_t ui, uint64_t hid, uint64_t cid, void* device);
extern void GoUiBackendWgpuRender(uintptr_t ui, void* frame, void* command_encoder);

static void call(kos_cookie_t cookie, vid_t vdev_id, uint64_t conn_id, uint64_t fn_id, kos_val_t const* args) {
	(void) vdev_id;

	assert(VDRIVER.notif_cb != NULL);

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CALL_RET,
		.conn_id = conn_id,
		.cookie = cookie,
	};

	void* ui = NULL;

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
	// WebGPU backend specific stuff.
	case 5:
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

		GoUiBackendWgpuInit((uintptr_t) ui, args[1].u64, args[2].u64, device);
		break;
	case 6:
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

		GoUiBackendWgpuRender((uintptr_t) ui, frame_encoder, command_encoder);
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
