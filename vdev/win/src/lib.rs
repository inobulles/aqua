// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(improper_ctypes)]

include!(concat!(env!("OUT_DIR"), "/vdev_bindings.rs"));

use std::os::raw::c_char;

use ctor::ctor;
use winit::event_loop::EventLoop;

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

#[derive(Clone)]
struct Fn {
	name: &'static str,
	cb: fn(),
}

static FNS: [Fn; 1] = [Fn {
	name: "create",
	cb: || {
		let _event_loop = EventLoop::new();
		println!("Event loop created: TODO the rest of this");
	},
}];

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
					fn_count: 1,
					fns: FNS
						.clone()
						.map(|x| kos_vdev_fn_t {
							name: str_to_slice::<u8, 64>(x.name),
							ret_type: kos_type_t_KOS_TYPE_VOID,
							arg_count: 0,
							args: std::ptr::null(),
						})
						.as_ptr(),
				},
			},
		},
		VDRIVER.notif_data,
	);
}

#[ctor]
unsafe fn vdriver_init() {
	// XXX No way to do this at compile time afaict.

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

	// We have to set these explicitly because.
	vdev_id_lo: 0,
	vdev_id_hi: 0,
	notif_cb: None,
	notif_data: std::ptr::null_mut(),
};
