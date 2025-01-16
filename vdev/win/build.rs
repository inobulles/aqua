// This Source Form is subject to the terms of the AQUA Software License,
// v. 1.0. Copyright (c) 2025 Aymeric Wibo

extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
	let vdev_header = "../../kos/vdev.h";
	let kos_header = "../../kos/kos.h";
	let win_header = "win.h";

	// Tell cargo to invalidate the built crate whenever one of the headers change.

	println!("cargo:rerun-if-changed={}", vdev_header);
	println!("cargo:rerun-if-changed={}", kos_header);
	println!("cargo:rerun-if-changed={}", win_header);

	let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

	let bindings = bindgen::Builder::default()
		.header(vdev_header)
		.header(win_header)
		.generate_comments(true)
		.generate()
		.expect("Unable to generate bindings");

	bindings
		.write_to_file(out_path.join("bindings.rs"))
		.expect("Couldn't write bindings");
}
