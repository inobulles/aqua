// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
	let vdev_header = "../../kos/vdev.h";
	let kos_header = "../../kos/kos.h";

	// Tell cargo to invalidate the built crate whenever the header changes

	println!("cargo:rerun-if-changed={}", vdev_header);
	println!("cargo:rerun-if-changed={}", kos_header);

	let bindings = bindgen::Builder::default()
		.header(vdev_header)
		.generate_comments(true)
		.generate()
		.expect("Unable to generate bindings");

	let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

	bindings
		.write_to_file(out_path.join("vdev_bindings.rs"))
		.expect("Couldn't write bindings");
}
