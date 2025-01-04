// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2024 Aymeric Wibo

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(improper_ctypes)]

include!(concat!(env!("OUT_DIR"), "/vdev_bindings.rs"));

use ctor::ctor;
use std::os::raw::c_char;

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

#[allow(static_mut_refs)]
unsafe extern "C" fn probe() {
	assert!(VDRIVER.vdev_notif_cb.is_some());

	// At the moment, there can only be one window VDEV on this vdriver.
	// I'm not too sure at the moment what would be the use case for multiple window VDEVs.

	let mut notif = kos_notif_t {
		kind: kos_notif_kind_t_KOS_NOTIF_ATTACH,
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
					vdev_id: 0,
				},
			},
		},
	};

	VDRIVER.vdev_notif_cb.unwrap()(&mut notif, VDRIVER.vdev_notif_data);
}

#[ctor]
unsafe fn vdriver_init() {
	VDRIVER.spec = str_to_slice::<c_char, 64>(SPEC);
	VDRIVER.human = str_to_slice::<c_char, 256>(VDRIVER_HUMAN);
	VDRIVER.vers = VERS;
	VDRIVER.probe = Some(probe);
}
