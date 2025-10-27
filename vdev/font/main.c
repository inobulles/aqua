// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#include <aqua/vdriver.h>

#include <umber.h>

#include <pango/pango.h>
#include <pango/pangocairo.h>

#include <assert.h>
#include <stddef.h>

#define SPEC "aquabsd.black.font"
#define VERS 0
#define VDRIVER_HUMAN "Font driver"

typedef struct {
	PangoLayout* layout;
	PangoContext* context;
} layout_t;

static umber_class_t const* cls = NULL;
static vid_t only_vid;

static void init(void) {
	cls = umber_class_new(SPEC, UMBER_LVL_WARN, SPEC " Font VDRIVER.");
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
			.human = "Font rendering device",
			.vdriver_human = VDRIVER_HUMAN,

			.pref = 0,
			.host_id = VDRIVER.host_id,
			.vdev_id = only_vid,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static kos_fn_t const FNS[] = {
	{
		.name = "font_from_str",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_BUF, "str"},
		},
	},
	{
		.name = "font_destroy",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "font"},
		},
	},
	{
		.name = "layout_create",
		.ret_type = KOS_TYPE_OPAQUE_PTR,
		.param_count = 2,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "font"},
			{KOS_TYPE_BUF, "text"},
		},
	},
	{
		.name = "layout_destroy",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 1,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "layout"},
		},
	},
	{
		.name = "layout_set_text",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 2,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "layout"},
			{KOS_TYPE_BUF, "text"},
		},
	},
	{
		.name = "layout_set_limits",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "layout"},
			{KOS_TYPE_U32, "x_res_limit"},
			{KOS_TYPE_U32, "y_res_limit"},
		},
	},
	{
		.name = "layout_pos_to_index",
		.ret_type = KOS_TYPE_I32,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "layout"},
			{KOS_TYPE_U32, "x"},
			{KOS_TYPE_U32, "y"},
		},
	},
	{
		.name = "layout_index_to_pos",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 4,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "layout"},
			{KOS_TYPE_I32, "index"},
			{KOS_TYPE_PTR, "x"},
			{KOS_TYPE_PTR, "y"},
		},
	},
	{
		.name = "layout_get_res",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 3,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "layout"},
			{KOS_TYPE_PTR, "x_res"},
			{KOS_TYPE_PTR, "y_res"},
		},
	},
	{
		.name = "layout_render",
		.ret_type = KOS_TYPE_VOID,
		.param_count = 2,
		.params = (kos_param_t[]) {
			{KOS_TYPE_OPAQUE_PTR, "layout"},
			{KOS_TYPE_PTR, "buffer"},
		},
	},
	// TODO Functions for setting colour, markup on/off, alignment (constants!), word/char wrapping.
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
			.fn_count = sizeof FNS / sizeof *FNS,
			.fns = FNS,
		},
	};

	VDRIVER.notif_cb(&notif, VDRIVER.notif_data);
}

static PangoFontDescription* font_from_str(char const* str) {
	assert(cls != NULL);

	LOG_V(cls, "Creating Pango font description from string: \"%s\".", str);

	PangoFontDescription* const descr = pango_font_description_from_string(str);

	if (descr == NULL) {
		LOG_E(cls, "Failed to create Pango font description from string: \"%s\".", str);
		return NULL;
	}

	return descr;
}

static void font_destroy(PangoFontDescription* font) {
	pango_font_description_free(font);
}

static layout_t* layout_create(PangoFontDescription* font, char const* text, size_t length) {
	LOG_V(cls, "Creating layout objects.");

	layout_t* const layout = malloc(sizeof *layout);
	assert(layout != NULL);

	layout->context = pango_context_new();
	assert(layout->context != NULL);
	pango_context_set_font_map(layout->context, pango_cairo_font_map_get_default());

	layout->layout = pango_layout_new(layout->context);
	assert(layout->layout != NULL);

	size_t const truncated = MAX(length, 30);

	LOG_V(
		cls,
		"Setting layout font to '%s' and text to '%.*s%s'.",
		pango_font_description_get_family(font),
		truncated,
		text,
		truncated < length ? "..." : ""
	);

	pango_layout_set_font_description(layout->layout, font);
	pango_layout_set_text(layout->layout, text, length);

	return layout;
}

static void layout_destroy(layout_t* layout) {
	g_object_unref(layout->layout);
	g_object_unref(layout->context);

	free(layout);
}

static void layout_set_text(layout_t* layout, char const* text, size_t length) {
	size_t const truncated = MAX(length, 30);

	LOG_V(
		cls,
		"Setting layout text to '%.*s%s'.",
		truncated,
		text,
		truncated < length ? "..." : ""
	);

	pango_layout_set_text(layout->layout, text, length);
}

static void layout_set_limits(layout_t* layout, uint32_t x_res_limit, uint32_t y_res_limit) {
	LOG_V(cls, "Setting limits to (x=%u, y=%u).\n", x_res_limit, y_res_limit);

	if (x_res_limit == 0) {
		pango_layout_set_width(layout->layout, -1);
	}

	else {
		pango_layout_set_width(layout->layout, x_res_limit * PANGO_SCALE);
	}

	if (y_res_limit == 0) {
		pango_layout_set_height(layout->layout, -1);
	}

	else {
		pango_layout_set_height(layout->layout, y_res_limit * PANGO_SCALE);
	}
}

static int layout_pos_to_index(layout_t* layout, uint32_t x, uint32_t y) {
	LOG_V(cls, "Mapping %ux%u to index.", x, y);

	int index = -1;
	int trailing = 0;

	if (!pango_layout_xy_to_index(layout->layout, x * PANGO_SCALE, y * PANGO_SCALE, &index, &trailing)) {
		LOG_W(cls, "Position (%u, %u) did not map to a valid index.", x, y);
		return -1;
	}

	return index;
}

static void layout_index_to_pos(layout_t* layout, int32_t index, uint32_t* x_ref, uint32_t* y_ref) {
	LOG_V(cls, "Mapping %d to position.", index);

	PangoRectangle rect;
	pango_layout_index_to_pos(layout->layout, index, &rect);

	*x_ref = rect.x / PANGO_SCALE;
	*y_ref = rect.y / PANGO_SCALE;
}

static void layout_get_res(layout_t* layout, uint32_t* x_res_ref, uint32_t* y_res_ref) {
	LOG_V(cls, "Getting resolution of layout.\n");

	int32_t w = 0, h = 0;
	pango_layout_get_pixel_size(layout->layout, &w, &h);

	if (w <= 0 || h <= 0) {
		LOG_W(cls, "Layout has unexpected resolution (%dx%d).", w, h);
	}

	*x_res_ref = w;
	*y_res_ref = h;
}

static int layout_render(layout_t* layout, kos_ptr_t buf) {
	// XXX I have not profiled this (and either way I think this would depend a lot on the type of application), but I don't suspect the Cairo operations are costly enough to bother caching them between invocations of layout_render.

	int rv = -1;

	LOG_V(cls, "Preparing to render layout.");

	int32_t w = 0, h = 0;
	pango_layout_get_pixel_size(layout->layout, &w, &h);

	if (w <= 0 || h <= 0) {
		LOG_W(cls, "Layout has no size (%dx%d), cannot render.", w, h);
		return 0;
	}

	cairo_surface_t* const surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);

	if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
		LOG_E(cls, "Failed to create Cairo surface for rendering layout: %s.", cairo_status_to_string(cairo_surface_status(surface)));
		goto err_surface_create;
	}

	cairo_t* const cr = cairo_create(surface);

	if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
		LOG_E(cls, "Failed to create Cairo context for rendering layout: %s.", cairo_status_to_string(cairo_status(cr)));
		goto err_cairo_create;
	}

	LOG_V(cls, "Rendering layout.\n");

	pango_cairo_update_layout(cr, layout->layout);
	cairo_set_source_rgba(cr, 1., 1., 1., 1.);

	pango_cairo_show_layout(cr, layout->layout);
	cairo_surface_flush(surface);

	LOG_V(cls, "Writing data to client bitmap.\n");

	size_t const bytes = cairo_image_surface_get_stride(surface) * h;

	if (VDRIVER.write_ptr(buf, cairo_image_surface_get_data(surface), bytes) < 0) {
		LOG_E(cls, "Failed to write rendered layout data to client bitmap.");
		goto err_write;
	}

	rv = 0;

err_write:

	cairo_destroy(cr);

err_cairo_create:

	cairo_surface_destroy(surface);

err_surface_create:

	return rv;
}

static void call(kos_cookie_t cookie, uint64_t conn_id, uint64_t fn_id, kos_val_t const* args) {
	assert(VDRIVER.notif_cb != NULL);

	kos_notif_t notif = {
		.kind = KOS_NOTIF_CALL_RET,
		.conn_id = conn_id,
		.cookie = cookie,
	};

	switch (fn_id) {
	case 0: { // font_from_str
		char* const str = strndup(args[0].buf.ptr, args[0].buf.size);
		assert(str != NULL);

		PangoFontDescription* const font = font_from_str((char const*) args[0].buf.ptr);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(font);

		free(str);
		break;
	}
	case 1: { // font_destroy
		PangoFontDescription* const font = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (font == NULL) {
			LOG_E(cls, "Tried to destroy non-local or NULL font.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		font_destroy(font);
		break;
	}
	case 2: { // layout_create
		PangoFontDescription* const font = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (font == NULL) {
			LOG_E(cls, "Tried to create layout with a non-local or NULL font.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		layout_t* const layout = layout_create(font, args[1].buf.ptr, args[1].buf.size);
		notif.call_ret.ret.opaque_ptr = vdriver_make_opaque_ptr(layout);

		break;
	}
	case 3: { // layout_destroy
		layout_t* const layout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (layout == NULL) {
			LOG_E(cls, "Tried to destroy non-local or NULL layout.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		layout_destroy(layout);
		break;
	}
	case 4: { // layout_set_text
		layout_t* const layout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (layout == NULL) {
			LOG_E(cls, "Tried to set text of non-local or NULL layout.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		layout_set_text(layout, args[1].buf.ptr, args[1].buf.size);
		break;
	}
	case 5: { // layout_set_limits
		layout_t* const layout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (layout == NULL) {
			LOG_E(cls, "Tried to set limits of non-local or NULL layout.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		layout_set_limits(layout, args[1].u32, args[2].u32);
		break;
	}
	case 6: { // layout_pos_to_index
		layout_t* const layout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (layout == NULL) {
			LOG_E(cls, "Tried to convert position to text index of non-local or NULL layout.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		notif.call_ret.ret.i32 = layout_pos_to_index(layout, args[1].u32, args[2].u32);
		break;
	}
	case 7: { // layout_index_to_pos
		layout_t* const layout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (layout == NULL) {
			LOG_E(cls, "Tried to convert text index to position of non-local or NULL layout.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		uint32_t x = 0, y = 0;
		layout_index_to_pos(layout, args[1].i32, &x, &y);

		if (
			VDRIVER.write_ptr(args[2].ptr, &x, sizeof x) < 0 ||
			VDRIVER.write_ptr(args[3].ptr, &y, sizeof y) < 0
		) {
			LOG_E(cls, "Failed to write layout index position to pointers.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		break;
	}
	case 8: { // layout_get_res
		layout_t* const layout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (layout == NULL) {
			LOG_E(cls, "Tried to get resolution of non-local or NULL layout.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		uint32_t x_res, y_res;
		layout_get_res(layout, &x_res, &y_res);

		if (
			VDRIVER.write_ptr(args[1].ptr, &x_res, sizeof x_res) < 0 ||
			VDRIVER.write_ptr(args[2].ptr, &y_res, sizeof y_res) < 0
		) {
			LOG_E(cls, "Failed to write resolution to pointers.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		break;
	}
	case 9: { // layout_render
		layout_t* const layout = vdriver_unwrap_local_opaque_ptr(args[0].opaque_ptr);

		if (layout == NULL) {
			LOG_E(cls, "Tried to render non-local or NULL layout.");
			notif.kind = KOS_NOTIF_CALL_FAIL;
			break;
		}

		if (layout_render(layout, args[1].ptr) < 0) {
			notif.kind = KOS_NOTIF_CALL_FAIL;
		}

		break;
	}
	default:
		notif.kind = KOS_NOTIF_CALL_FAIL;
		break;
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
