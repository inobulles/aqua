// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

// TODO In fine, we should support everything shown off in the winit example:
// https://github.com/rust-windowing/winit/blob/master/examples/window.rs

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(improper_ctypes)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

use std::os::raw::{c_char, c_void};

use ctor::ctor;
use raw_window_handle::{RawDisplayHandle, RawWindowHandle};
use winit::application::ApplicationHandler;
use winit::event::WindowEvent;
use winit::event_loop::{ActiveEventLoop, ControlFlow, EventLoop};
use winit::raw_window_handle::{HasDisplayHandle, HasWindowHandle};
use winit::window::{Window, WindowId};

const SPEC: &str = "aquabsd.black.win";
const VERS: u32 = 0;

const VDEV_HUMAN: &str = "Rust winit window VDEV for FreeBSD, Linux, and macOS";
const VDRIVER_HUMAN: &str = "Default window driver for FreeBSD, Linux, and macOS";

static mut only_vdev_id: vid_t = 0;

trait AllowedForStrToSlice: Copy + Default {
	fn from_u8(x: u8) -> Self;
}

impl AllowedForStrToSlice for c_char {
	fn from_u8(x: u8) -> Self {
		x as Self
	}
}

impl AllowedForStrToSlice for u8 {
	fn from_u8(x: u8) -> Self {
		x
	}
}

fn str_to_slice<T: AllowedForStrToSlice, const N: usize>(input: &str) -> [T; N] {
	assert!(input.len() + 1 <= N);

	let mut array_tmp = [0u8; N];
	array_tmp[..input.len()].copy_from_slice(input.as_bytes());

	array_tmp.map(T::from_u8)
}

#[derive(Clone, Copy)]
enum Intr {
	REDRAW,
	RESIZE,
}

#[repr(C)]
struct Win {
	win: aqua_win_t, // This must be at the beginning of the struct.
	ino: Option<u32>,
	window: Option<Window>,
}

#[repr(packed)]
#[allow(dead_code)]
struct RedrawIntr {
	intr: u8,
}

#[repr(packed)]
#[allow(dead_code)]
struct ResizeIntr {
	intr: u8,
	x_res: u32,
	y_res: u32,
}

impl Win {
	fn interrupt<T>(&self, data: T) {
		if let Some(ino) = self.ino {
			unsafe {
				VDRIVER.notif_cb.unwrap()(
					&kos_notif_t {
						kind: kos_notif_kind_t_KOS_NOTIF_INTERRUPT,
						conn_id: 0,
						cookie: 0,
						__bindgen_anon_1: kos_notif_t__bindgen_ty_1 {
							interrupt: kos_notif_t__bindgen_ty_1__bindgen_ty_7 {
								ino,
								data_size: std::mem::size_of::<T>() as u32,
								data: &data as *const T as *const c_void,
							},
						},
					},
					VDRIVER.notif_data,
				);
			}
		}
	}
}

impl ApplicationHandler for Win {
	fn resumed(&mut self, event_loop: &ActiveEventLoop) {
		let attrs = Window::default_attributes()
			.with_blur(true)
			.with_transparent(true)
			.with_title(format!("Untitled ({VDEV_HUMAN})"));

		self.window = Some(event_loop.create_window(attrs).unwrap());

		let win_ref = self.window.as_ref().unwrap();

		match win_ref.display_handle().unwrap().as_raw() {
			RawDisplayHandle::Wayland(display) => {
				self.win.kind = aqua_win_kind_t_AQUA_WIN_KIND_WAYLAND;
				self.win.detail.wayland.display = display.display.as_ptr();
			}
			RawDisplayHandle::Xcb(display) => {
				self.win.kind = aqua_win_kind_t_AQUA_WIN_KIND_XCB;
				self.win.detail.xcb.connection = display.connection.unwrap().as_ptr();
				self.win.detail.xcb.screen = display.screen;
			}
			RawDisplayHandle::Xlib(display) => {
				self.win.kind = aqua_win_kind_t_AQUA_WIN_KIND_XLIB;

				self.win.detail.xlib.display = display.display.unwrap().as_ptr();
				self.win.detail.xlib.screen = display.screen;
			}
			RawDisplayHandle::AppKit(_display) => {
				self.win.kind = aqua_win_kind_t_AQUA_WIN_KIND_APPKIT;
				// XXX AppKit doesn't have a display.
			}
			_ => {
				// TODO Error message here.
			}
		}

		match win_ref.window_handle().unwrap().as_raw() {
			RawWindowHandle::Wayland(window) => {
				assert!(self.win.kind == aqua_win_kind_t_AQUA_WIN_KIND_WAYLAND);
				self.win.detail.wayland.surface = window.surface.as_ptr();
			}
			RawWindowHandle::Xcb(window) => {
				assert!(self.win.kind == aqua_win_kind_t_AQUA_WIN_KIND_XCB);
				self.win.detail.xcb.window = window.window.into();
				self.win.detail.xcb.visual_id = window.visual_id.unwrap().into();
			}
			RawWindowHandle::Xlib(window) => {
				assert!(self.win.kind == aqua_win_kind_t_AQUA_WIN_KIND_XLIB);
				self.win.detail.xlib.window = window.window;
				self.win.detail.xlib.visual_id = window.visual_id;
			}
			RawWindowHandle::AppKit(window) => {
				assert!(self.win.kind == aqua_win_kind_t_AQUA_WIN_KIND_APPKIT);
				self.win.detail.appkit.ns_view = window.ns_view.as_ptr();
			}
			_ => {
				// TODO Error message here.
			}
		}
	}

	fn window_event(&mut self, event_loop: &ActiveEventLoop, _window_id: WindowId, event: WindowEvent) {
		match event {
			WindowEvent::CloseRequested => {
				event_loop.exit();
			}
			WindowEvent::Resized(size) => {
				self.interrupt(ResizeIntr {
					intr: Intr::RESIZE as u8,
					x_res: size.width,
					y_res: size.height,
				});
			}
			WindowEvent::RedrawRequested => {
				self.interrupt(RedrawIntr {
					intr: Intr::REDRAW as u8,
				});

				self.window.as_ref().unwrap().request_redraw();
			}
			_ => (),
		}
	}
}

struct Const {
	name: &'static str,
	kind: kos_type_t,
	val: kos_val_t,
}

struct Param {
	name: &'static str,
	kind: kos_type_t,
}

#[derive(Clone)]
struct Fn {
	name: &'static str,
	ret_type: kos_type_t,
	params: &'static [Param],
	cb: fn(args: *const kos_val_t) -> Option<kos_val_t>,
}

static FNS: [Fn; 4] = [
	Fn {
		name: "create",
		ret_type: kos_type_t_KOS_TYPE_OPAQUE_PTR,
		params: &[],
		cb: |_args| {
			let win = Box::new(Win {
				win: aqua_win_t {
					kind: aqua_win_kind_t_AQUA_WIN_KIND_NONE,
					detail: aqua_win_t__bindgen_ty_1 {
						none: aqua_win_t__bindgen_ty_1__bindgen_ty_1 {},
					},
					priv_: __IncompleteArrayField::new(),
				},
				ino: None,
				window: None,
			});

			Some(kos_val_t {
				opaque_ptr: Box::into_raw(win) as *mut c_void as u64,
			})
		},
	},
	Fn {
		name: "destroy",
		ret_type: kos_type_t_KOS_TYPE_VOID,
		params: &[Param {
			name: "win",
			kind: kos_type_t_KOS_TYPE_OPAQUE_PTR,
		}],
		cb: |args| {
			let _ = unsafe { Box::from_raw((*args).opaque_ptr as *mut Win) };

			// We don't need to explicitly drop the box; from_raw automatically reclaims ownership and its Drop implementation will be called as we go out of scope.
			None
		},
	},
	Fn {
		name: "register_interrupt",
		ret_type: kos_type_t_KOS_TYPE_VOID,
		params: &[
			Param {
				name: "win",
				kind: kos_type_t_KOS_TYPE_OPAQUE_PTR,
			},
			Param {
				name: "ino",
				kind: kos_type_t_KOS_TYPE_U32,
			},
		],
		cb: |args| {
			let mut win = unsafe { Box::from_raw((*args).opaque_ptr as *mut Win) };
			let ino = unsafe { (*args.add(1)).u32_ };

			win.ino = Some(ino);

			let _ = Box::into_raw(win); // Don't drop memory.
			None
		},
	},
	Fn {
		name: "loop",
		ret_type: kos_type_t_KOS_TYPE_VOID,
		params: &[Param {
			name: "win",
			kind: kos_type_t_KOS_TYPE_OPAQUE_PTR,
		}],
		cb: |args| {
			let mut win = unsafe { Box::from_raw((*args).opaque_ptr as *mut Win) };

			let event_loop = EventLoop::new().expect("failed to create event loop");
			event_loop.set_control_flow(ControlFlow::Poll);
			let _ = event_loop.run_app(&mut win);

			let _ = Box::into_raw(win); // Don't drop memory.
			None
		},
	},
];

#[allow(static_mut_refs)]
unsafe extern "C" fn probe() {
	assert!(VDRIVER.notif_cb.is_some());

	// At the moment, there can only be one window VDEV on this vdriver.
	// I'm not too sure at the moment what would be the use case for multiple window VDEVs.

	only_vdev_id = VDRIVER.vdev_id_lo;

	VDRIVER.notif_cb.unwrap()(
		&kos_notif_t {
			kind: kos_notif_kind_t_KOS_NOTIF_ATTACH,
			conn_id: 0,
			cookie: 0,
			__bindgen_anon_1: kos_notif_t__bindgen_ty_1 {
				attach: kos_notif_t__bindgen_ty_1__bindgen_ty_1 {
					vdev: kos_vdev_descr_t {
						kind: kos_vdev_kind_t_KOS_VDEV_KIND_LOCAL,
						spec: str_to_slice::<u8, 64>(SPEC),
						vers: VERS,
						human: str_to_slice::<u8, 256>(VDEV_HUMAN),
						vdriver_human: str_to_slice::<u8, 256>(VDRIVER_HUMAN),

						pref: 0,
						host_id: 0,
						vdev_id: only_vdev_id,
					},
				},
			},
		},
		VDRIVER.notif_data,
	);
}

#[allow(static_mut_refs)]
unsafe extern "C" fn conn(cookie: u64, vdev_id: vid_t, conn_id: u64) {
	assert!(VDRIVER.notif_cb.is_some());

	// Since we only support one VDEV, if this is not the ID we gave when probing, we necessarily know something went wrong.

	if vdev_id != only_vdev_id {
		VDRIVER.notif_cb.unwrap()(
			&kos_notif_t {
				kind: kos_notif_kind_t_KOS_NOTIF_CONN_FAIL,
				conn_id: 0,
				cookie,
				__bindgen_anon_1: kos_notif_t__bindgen_ty_1 {
					conn_fail: kos_notif_t__bindgen_ty_1__bindgen_ty_3 {},
				},
			},
			VDRIVER.notif_data,
		);

		return;
	}

	let CONSTS = [
		Const {
			name: "INTR_REDRAW",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: Intr::REDRAW as u8,
			},
		},
		Const {
			name: "INTR_RESIZE",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: Intr::RESIZE as u8,
			},
		},
	];

	VDRIVER.notif_cb.unwrap()(
		&kos_notif_t {
			kind: kos_notif_kind_t_KOS_NOTIF_CONN,
			conn_id,
			cookie,
			__bindgen_anon_1: kos_notif_t__bindgen_ty_1 {
				conn: kos_notif_t__bindgen_ty_1__bindgen_ty_4 {
					const_count: CONSTS.len() as u32,
					consts: CONSTS
						.iter()
						.map(|x| kos_const_t {
							name: str_to_slice::<u8, 64>(x.name),
							type_: x.kind,
							val: x.val,
						})
						.collect::<Vec<_>>()
						.as_ptr(),
					fn_count: FNS.len() as u32,
					fns: FNS
						.clone()
						.map(|x| kos_fn_t {
							name: str_to_slice::<u8, 64>(x.name),
							ret_type: x.ret_type,
							param_count: x.params.len() as u32,
							params: Box::into_raw(
								// TODO Should we ever bother to free this?
								x.params
									.iter()
									.map(|param| kos_param_t {
										type_: param.kind,
										name: str_to_slice::<u8, 64>(param.name),
									})
									.collect::<Vec<_>>()
									.into_boxed_slice(),
							) as *const kos_param_t,
						})
						.as_ptr(),
				},
			},
		},
		VDRIVER.notif_data,
	);
}

#[allow(static_mut_refs)]
unsafe extern "C" fn call(cookie: u64, conn_id: u64, fn_id: u64, args: *const kos_val_t) {
	assert!(VDRIVER.notif_cb.is_some());
	let ret = (FNS[fn_id as usize].cb)(args);

	VDRIVER.notif_cb.unwrap()(
		&kos_notif_t {
			kind: kos_notif_kind_t_KOS_NOTIF_CALL_RET,
			conn_id,
			cookie,
			__bindgen_anon_1: kos_notif_t__bindgen_ty_1 {
				call_ret: kos_notif_t__bindgen_ty_1__bindgen_ty_6 {
					ret: ret.unwrap_or(kos_val_t {
						u8_: 0, // Dummy value; this won't be read anyway as if we're returning this, we've already told the client the function returns void.
					}),
				},
			},
		},
		VDRIVER.notif_data,
	);
}

#[ctor]
unsafe fn vdriver_init() {
	// XXX No way to do this at compile time afaict.
	// Unfortunately this forces VDRIVER to be mutable.

	VDRIVER.spec = str_to_slice::<c_char, 64>(SPEC);
	VDRIVER.human = str_to_slice::<c_char, 256>(VDRIVER_HUMAN);
}

#[no_mangle]
pub static mut VDRIVER: vdriver_t = vdriver_t {
	spec: [0; 64],
	human: [0; 256],
	vers: VERS,
	probe: Some(probe),
	init: None,
	conn: Some(conn),
	call: Some(call),

	// We have to set these explicitly because.
	vdev_id_lo: 0,
	vdev_id_hi: 0,
	notif_cb: None,
	notif_data: std::ptr::null_mut(),
};
