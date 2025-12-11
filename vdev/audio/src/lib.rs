// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(improper_ctypes)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

extern crate cpal;

use std::collections::HashMap;
use std::ffi::c_void;
use std::os::raw::c_char;
use std::sync::Mutex;

use cpal::traits::{DeviceTrait, HostTrait};
use cpal::{Device, SampleFormat};
use ctor::ctor;
use once_cell::sync::Lazy;

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
	cb: fn(vdev_id: vid_t, args: *const kos_val_t) -> Option<kos_val_t>,
}

#[repr(C)]
struct StreamConfig {
	sample_format: u8,
	min_sample_rate: u32,
	max_sample_rate: u32,
	min_buf_size: u32,
	max_buf_size: u32,
	channels: u16,
}

static FNS: [Fn; 1] = [
	Fn {
		name: "get_configs",
		ret_type: kos_type_t_KOS_TYPE_BUF,
		params: &[],
		cb: |vdev_id, _args| {
			// TODO Proper error conditions.

			let dev = VDEV_MAP.lock().unwrap().get(&vdev_id).cloned().unwrap();

			let output_configs = match dev.supported_output_configs() {
				Ok(f) => f.collect(),
				Err(e) => {
					println!("Error getting supported output configs: {e:?}.");
					Vec::new()
				}
			};

			let mut out = Vec::new();

			for config in output_configs.into_iter() {
				match config.buffer_size() {
					cpal::SupportedBufferSize::Range { min, max } => out.push(StreamConfig {
						sample_format: config.sample_format() as u8,
						min_sample_rate: config.min_sample_rate().0,
						max_sample_rate: config.max_sample_rate().0,
						min_buf_size: *min,
						max_buf_size: *max,
						channels: config.channels(),
					}),
					cpal::SupportedBufferSize::Unknown => (),
				}
			}

			Some(kos_val_t {
				buf: kos_val_t__bindgen_ty_1 {
					size: out.len() as u32,
					ptr: out.as_ptr() as *const c_void,
				},
			})
		},
	},
	// TODO The API I want is to simply open a stream and be able to shove data to it continuously.
	// To do this, we should have an open stream function, a write function, a play function, and a pause function (so we can buffer a bunch of frames if we're expecting a lot of network latency/jitter).
	// The CPAL API where it calls back to ask for data won't work for us because then we'll have minimum 1.5 RTT latency over the network rather than just 0.5 RTT.
];

static VDEV_MAP: Lazy<Mutex<HashMap<vid_t, Device>>> = Lazy::new(|| Mutex::new(HashMap::new()));

#[allow(static_mut_refs)]
extern "C" fn probe() {
	let notif_cb = unsafe { VDRIVER.notif_cb };
	assert!(unsafe { VDRIVER.notif_cb }.is_some());

	let mut cur_vdev_id = unsafe { VDRIVER.vdev_id_lo };

	for host_id in cpal::available_hosts() {
		let host = cpal::host_from_id(host_id).expect("could not get host from ID");
		let devices = host.output_devices().expect("could not get host's devices");

		for device in devices {
			let human = device.name().expect("could not get device name");

			// Insert device into global device map.

			let mut map = VDEV_MAP.lock().expect("device map poisoned");
			map.insert(cur_vdev_id, device.clone());

			// Emit actual KOS attach notification.

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
extern "C" fn conn(cookie: u64, vdev_id: vid_t, conn_id: u64) {
	let notif_cb = unsafe { VDRIVER.notif_cb };
	assert!(notif_cb.is_some());

	if !VDEV_MAP.lock().unwrap().contains_key(&vdev_id) {
		unsafe {
			notif_cb.unwrap()(
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
		}

		return;
	}

	let CONSTS = [
		Const {
			name: "SAMPLE_FORMAT_I8",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::I8 as u8,
			},
		},
		Const {
			name: "SAMPLE_FORMAT_I16",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::I16 as u8,
			},
		},
		Const {
			name: "SAMPLE_FORMAT_I24",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::I24 as u8,
			},
		},
		Const {
			name: "SAMPLE_FORMAT_I32",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::I32 as u8,
			},
		},
		// TODO I48 when CPAL adds that.
		Const {
			name: "SAMPLE_FORMAT_I64",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::I64 as u8,
			},
		},
		Const {
			name: "SAMPLE_FORMAT_U8",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::U8 as u8,
			},
		},
		Const {
			name: "SAMPLE_FORMAT_U16",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::U16 as u8,
			},
		},
		// TODO U24 when CPAL adds that.
		Const {
			name: "SAMPLE_FORMAT_U32",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::U32 as u8,
			},
		},
		// TODO U48 when CPAL adds that.
		Const {
			name: "SAMPLE_FORMAT_U64",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::U64 as u8,
			},
		},
		Const {
			name: "SAMPLE_FORMAT_F32",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::F32 as u8,
			},
		},
		Const {
			name: "SAMPLE_FORMAT_F64",
			kind: kos_type_t_KOS_TYPE_U8,
			val: kos_val_t {
				u8_: SampleFormat::F64 as u8,
			},
		},
	];

	let consts = CONSTS
		.iter()
		.map(|x| kos_const_t {
			name: str_to_slice::<u8, 64>(x.name),
			type_: x.kind,
			val: x.val,
		})
		.collect::<Vec<_>>();

	let fns = FNS.clone().map(|x| kos_fn_t {
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
	});

	unsafe {
		notif_cb.unwrap()(
			&kos_notif_t {
				kind: kos_notif_kind_t_KOS_NOTIF_CONN,
				conn_id,
				cookie,
				__bindgen_anon_1: kos_notif_t__bindgen_ty_1 {
					conn: kos_notif_t__bindgen_ty_1__bindgen_ty_4 {
						const_count: CONSTS.len() as u32,
						consts: consts.as_ptr(),
						fn_count: FNS.len() as u32,
						fns: fns.as_ptr(),
					},
				},
			},
			VDRIVER.notif_data,
		);
	}
}

#[allow(static_mut_refs)]
unsafe extern "C" fn call(cookie: u64, vdev_id: vid_t, conn_id: u64, fn_id: u64, args: *const kos_val_t) {
	assert!(VDRIVER.notif_cb.is_some());
	let ret = (FNS[fn_id as usize].cb)(vdev_id, args);

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
