// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(improper_ctypes)]

include!(concat!(env!("OUT_DIR"), "/vdev_bindings.rs"));

use std::os::raw::{c_char, c_void};

use ctor::ctor;
use winit::application::ApplicationHandler;
use winit::event::WindowEvent;
use winit::event_loop::{ActiveEventLoop, ControlFlow, EventLoop};
use winit::window::{Window, WindowId};

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

const SPEC: &str = "aquabsd.black.win";
const VERS: u32 = 0;

const VDEV_HUMAN: &str = "Rust winit window VDEV for FreeBSD, Linux, and macOS";
const VDRIVER_HUMAN: &str = "Default window driver for FreeBSD, Linux, and macOS";

static mut only_vdev_id: u64 = 0;

#[allow(static_mut_refs)]
unsafe extern "C" fn probe() {
	assert!(VDRIVER.notif_cb.is_some());

	// At the moment, there can only be one window VDEV on this vdriver.
	// I'm not too sure at the moment what would be the use case for multiple window VDEVs.

	only_vdev_id = VDRIVER.vdev_id_lo;

	VDRIVER.notif_cb.unwrap()(
		&kos_notif_t {
			kind: kos_notif_kind_t_KOS_NOTIF_ATTACH,
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

#[derive(Default)]
struct App {
	window: Option<Window>,
}

impl ApplicationHandler for App {
	fn resumed(&mut self, event_loop: &ActiveEventLoop) {
		let attrs = Window::default_attributes().with_title("Untitled");

		self.window = Some(event_loop.create_window(attrs).unwrap());
	}

	fn window_event(&mut self, event_loop: &ActiveEventLoop, _window_id: WindowId, event: WindowEvent) {
		match event {
			WindowEvent::CloseRequested => {
				event_loop.exit();
			}
			WindowEvent::RedrawRequested => {
				// TODO Callback to client.
				self.window.as_ref().unwrap().request_redraw();
			}
			_ => (),
		}
	}
}

struct Win {
	event_loop: EventLoop<()>,
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
	cb: fn(args: *const c_void) -> Option<kos_val_t>,
}

static FNS: [Fn; 3] = [
	Fn {
		name: "create",
		ret_type: kos_type_t_KOS_TYPE_OPAQUE_PTR,
		params: &[],
		cb: |_args| {
			let win = Box::new(Win {
				event_loop: EventLoop::new().expect("failed to create event loop"),
			});

			win.event_loop.set_control_flow(ControlFlow::Poll);

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
			let opaque_ptr = unsafe { (*(args as *const kos_val_t)).opaque_ptr };
			let win = unsafe { Box::from_raw(opaque_ptr as *mut Win) };

			drop(win);
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
			let opaque_ptr = unsafe { (*(args as *const kos_val_t)).opaque_ptr };
			let win = unsafe { Box::from_raw(opaque_ptr as *mut Win) };

			let mut app = App::default();
			let _ = win.event_loop.run_app(&mut app);

			None
		},
	},
];

#[allow(static_mut_refs)]
unsafe extern "C" fn conn(cookie: u64, vdev_id: u64, conn_id: u64) {
	assert!(VDRIVER.notif_cb.is_some());

	// Since we only support one VDEV, if this is not the ID we gave when probing, we necessarily know something went wrong.

	if vdev_id != only_vdev_id {
		VDRIVER.notif_cb.unwrap()(
			&kos_notif_t {
				kind: kos_notif_kind_t_KOS_NOTIF_CONN_FAIL,
				cookie,
				__bindgen_anon_1: kos_notif_t__bindgen_ty_1 {
					conn_fail: kos_notif_t__bindgen_ty_1__bindgen_ty_3 {},
				},
			},
			VDRIVER.notif_data,
		);

		return;
	}

	VDRIVER.notif_cb.unwrap()(
		&kos_notif_t {
			kind: kos_notif_kind_t_KOS_NOTIF_CONN,
			cookie,
			__bindgen_anon_1: kos_notif_t__bindgen_ty_1 {
				conn: kos_notif_t__bindgen_ty_1__bindgen_ty_4 {
					conn_id,
					fn_count: FNS.len() as u32,
					fns: FNS
						.clone()
						.map(|x| kos_fn_t {
							name: str_to_slice::<u8, 64>(x.name),
							ret_type: x.ret_type,
							param_count: x.params.len() as u32,
							params: Box::into_raw(
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
unsafe extern "C" fn call(cookie: u64, _conn_id: u64, fn_id: u64, args: *const c_void) {
	assert!(VDRIVER.notif_cb.is_some());
	let ret = (FNS[fn_id as usize].cb)(args);

	VDRIVER.notif_cb.unwrap()(
		&kos_notif_t {
			kind: kos_notif_kind_t_KOS_NOTIF_CALL_RET,
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
