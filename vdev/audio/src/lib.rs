// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![allow(improper_ctypes)]

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

extern crate cpal;
extern crate ringbuf;

use std::collections::HashMap;
use std::ffi::c_void;
use std::os::raw::c_char;
use std::sync::{Arc, Mutex};
use std::{ptr, slice};

use cpal::traits::{DeviceTrait, HostTrait, StreamTrait};
use cpal::{Device, SampleFormat};
use ctor::ctor;
use once_cell::sync::Lazy;
use ringbuf::traits::{Consumer, Producer, Split};
use ringbuf::{CachingProd, HeapRb, SharedRb};

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
#[derive(Debug)]
struct StreamConfig {
	sample_format: u8,
	min_sample_rate: u32,
	max_sample_rate: u32,
	min_buf_size: u32,
	max_buf_size: u32,
	channels: u16,
}

struct Stream {
	stream: cpal::Stream,
	ringbuf: CachingProd<Arc<SharedRb<ringbuf::storage::Heap<u8>>>>,
}

fn open_stream<T>(dev: &cpal::Device, config: cpal::StreamConfig, ringbuf_size: usize) -> Stream
where
	T: cpal::SizedSample + cpal::FromSample<f32>,
{
	let sample_size = std::mem::size_of::<T>();
	let ringbuf = HeapRb::new(ringbuf_size * sample_size);
	let (prod, mut cons) = ringbuf.split();

	let stream = dev
		.build_output_stream(
			&config,
			move |data: &mut [T], _: &cpal::OutputCallbackInfo| {
				// TODO Figure out multiple channels.

				for frame in data.chunks_mut(config.channels as usize) {
					for sample in frame.iter_mut() {
						let mut buf = [0u8; 8]; // Max size for u64/f64.
						let needed = sample_size.min(buf.len());

						let got = cons.pop_slice(&mut buf[..needed]);

						if got < needed {
							// Underrun, output silence.

							// *sample = T::EQUILIBRIUM;
							*sample = T::from_sample(unsafe { f32::from_bits(i32::cast_unsigned(rand())) });
							continue;
						}

						let val = match sample_size {
							1 => T::from_sample(i8::from_le_bytes([buf[0]]) as f32),
							2 => {
								let v = i16::from_le_bytes(buf[..2].try_into().unwrap());
								T::from_sample(v as f32)
							}
							3 => {
								let v = i32::from_le_bytes([buf[0], buf[1], buf[2], 0]);
								T::from_sample(v as f32)
							}
							4 => {
								let v = f32::from_le_bytes(buf[..4].try_into().unwrap());
								T::from_sample(v)
							}
							8 => {
								let v = f64::from_le_bytes(buf[..8].try_into().unwrap());
								T::from_sample(v as f32)
							}
							_ => {
								*sample = T::EQUILIBRIUM;
								continue;
							}
						};

						*sample = val;
					}
				}
			},
			|err| eprintln!("an error occurred on stream: {err}"),
			None,
		)
		.expect("Failed to build stream.");

	Stream { ringbuf: prod, stream }
}

static FNS: [Fn; 4] = [
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

			// We need to allocate this memory in C because the KOS is responsible for freeing this memory.

			let bytes = out.len() * std::mem::size_of::<StreamConfig>();
			let ptr = unsafe { malloc(bytes as u64) } as *mut u8;
			assert!(!ptr.is_null(), "Failed to allocate.");

			unsafe {
				ptr::copy_nonoverlapping(out.as_ptr() as *mut u8, ptr, bytes);
			}

			Some(kos_val_t {
				buf: kos_val_t__bindgen_ty_1 {
					size: bytes as u32,
					ptr: ptr as *const c_void,
				},
			})
		},
	},
	Fn {
		name: "open_stream",
		ret_type: kos_type_t_KOS_TYPE_OPAQUE_PTR,
		params: &[
			Param {
				name: "config_sample_format",
				kind: kos_type_t_KOS_TYPE_U8,
			},
			Param {
				name: "config_channels",
				kind: kos_type_t_KOS_TYPE_U16,
			},
			Param {
				name: "config_sample_rate",
				kind: kos_type_t_KOS_TYPE_U32,
			},
			Param {
				name: "config_buf_size",
				kind: kos_type_t_KOS_TYPE_U32,
			},
			Param {
				name: "ringbuf_size",
				kind: kos_type_t_KOS_TYPE_U32,
			},
		],
		cb: |vdev_id, args| {
			let dev = VDEV_MAP.lock().unwrap().get(&vdev_id).cloned().unwrap();

			let sample_format = unsafe { (*args).u8_ };
			let channels = unsafe { (*args.add(1)).u16_ };
			let sample_rate = unsafe { (*args.add(2)).u32_ };
			let buf_size = unsafe { (*args.add(3)).u32_ };
			let ringbuf_size = unsafe { (*args.add(4)).u32_ } as usize;

			let config = cpal::StreamConfig {
				channels: channels,
				sample_rate: cpal::SampleRate(sample_rate),
				buffer_size: cpal::BufferSize::Fixed(buf_size),
			};

			let stream = Box::new(match sample_format {
				x if x == SampleFormat::I8 as u8 => open_stream::<i8>(&dev, config, ringbuf_size),
				x if x == SampleFormat::I16 as u8 => open_stream::<i16>(&dev, config, ringbuf_size),
				x if x == SampleFormat::I24 as u8 => open_stream::<cpal::I24>(&dev, config, ringbuf_size),
				x if x == SampleFormat::I32 as u8 => open_stream::<i32>(&dev, config, ringbuf_size),
				x if x == SampleFormat::I64 as u8 => open_stream::<i64>(&dev, config, ringbuf_size),
				x if x == SampleFormat::U8 as u8 => open_stream::<u8>(&dev, config, ringbuf_size),
				x if x == SampleFormat::U16 as u8 => open_stream::<u16>(&dev, config, ringbuf_size),
				x if x == SampleFormat::U32 as u8 => open_stream::<u32>(&dev, config, ringbuf_size),
				x if x == SampleFormat::U64 as u8 => open_stream::<u64>(&dev, config, ringbuf_size),
				x if x == SampleFormat::F32 as u8 => open_stream::<f32>(&dev, config, ringbuf_size),
				x if x == SampleFormat::F64 as u8 => open_stream::<f64>(&dev, config, ringbuf_size),
				sample_format => panic!("Unsupported sample format '{sample_format}'"),
			});

			Some(kos_val_t {
				opaque_ptr: unsafe { vdriver_make_opaque_ptr(Box::into_raw(stream) as *mut c_void) },
			})
		},
	},
	Fn {
		name: "write",
		ret_type: kos_type_t_KOS_TYPE_VOID,
		params: &[
			Param {
				name: "stream",
				kind: kos_type_t_KOS_TYPE_OPAQUE_PTR,
			},
			Param {
				name: "buf",
				kind: kos_type_t_KOS_TYPE_BUF,
			},
		],
		cb: |_vdev_id, args| {
			let stream_ptr = unsafe { vdriver_unwrap_local_opaque_ptr((*args).opaque_ptr) };
			assert!(
				!stream_ptr.is_null(),
				"Non-local opaque pointer passed to play for stream."
			);

			let mut stream = unsafe { Box::from_raw(stream_ptr as *mut Stream) };
			let buf = unsafe { (*args.add(1)).buf };
			let slice = unsafe { slice::from_raw_parts(buf.ptr as *const u8, buf.size as usize) };

			stream.ringbuf.push_slice(slice);

			let _ = Box::into_raw(stream); // Don't drop memory.
			None
		},
	},
	Fn {
		name: "play",
		ret_type: kos_type_t_KOS_TYPE_VOID,
		params: &[Param {
			name: "stream",
			kind: kos_type_t_KOS_TYPE_OPAQUE_PTR,
		}],
		cb: |_vdev_id, args| {
			let stream_ptr = unsafe { vdriver_unwrap_local_opaque_ptr((*args).opaque_ptr) };
			assert!(
				!stream_ptr.is_null(),
				"Non-local opaque pointer passed to play for stream."
			);

			let stream = unsafe { Box::from_raw(stream_ptr as *mut Stream) };
			stream.stream.play().expect("failed to play stream");

			let _ = Box::into_raw(stream); // Don't drop memory.
			None
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
