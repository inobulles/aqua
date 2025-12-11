// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(improper_ctypes)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

extern crate cpal;

use std::os::raw::c_char;

use cpal::traits::{DeviceTrait, HostTrait};
use ctor::ctor;

const SPEC: &str = "aquabsd.black.audio";
const VERS: u32 = 0;

const VDRIVER_HUMAN: &str = "Rust CPAL audio driver for FreeBSD, Linux, and macOS";

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

static FNS: [Fn; 0] = [
	// TODO
];

#[allow(static_mut_refs)]
extern "C" fn probe() {
	let notif_cb = unsafe { VDRIVER.notif_cb };
	assert!(unsafe { VDRIVER.notif_cb }.is_some());

	let mut cur_vdev_id = unsafe { VDRIVER.vdev_id_lo };

	for host_id in cpal::available_hosts() {
		let host = cpal::host_from_id(host_id).expect("could not get host from ID");
		let devices = host.devices().expect("could not get host's devices");

		for device in devices {
			let human = device.name().expect("could not get device name");

			unsafe {
				notif_cb.unwrap()(
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
									human: str_to_slice::<u8, 256>(human.as_str()),
									vdriver_human: str_to_slice::<u8, 256>(VDRIVER_HUMAN),

									pref: 0,
									host_id: VDRIVER.host_id,
									vdev_id: cur_vdev_id,
								},
							},
						},
					},
					VDRIVER.notif_data,
				);
			}

			cur_vdev_id += 1;
		}
	}
}

#[allow(static_mut_refs)]
unsafe extern "C" fn conn(cookie: u64, vdev_id: vid_t, conn_id: u64) {
	assert!(VDRIVER.notif_cb.is_some());

	// TODO We need to check if we have this vdev, yo.

	if false {
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

	let CONSTS: [Const; 0] = [
		// TODO (explicit typing can be removed once I add consts).
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
	write_ptr: None,

	// We have to set these explicitly because.
	host_id: 0,
	vdev_id_lo: 0,
	vdev_id_hi: 0,
	notif_cb: None,
	notif_data: std::ptr::null_mut(),
	lib: std::ptr::null_mut(),
};
