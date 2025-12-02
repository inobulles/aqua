// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

extern crate bindgen;

use std::env;
use std::path::PathBuf;

fn main() {
	let vdriver_header = "../../kos/lib/vdriver.h";
	let vdriver_src = "../../kos/lib/vdriver.c";
	let kos_header = "../../kos/header/kos.h";
	let win_header = "win.h";

	// Tell cargo to invalidate the built crate whenever one of the headers change.

	println!("cargo:rerun-if-changed={}", vdriver_header);
	println!("cargo:rerun-if-changed={}", kos_header);
	println!("cargo:rerun-if-changed={}", win_header);

	let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

	// Build VDRIVER helper library.

	let bob_prefix = env::var("BOB_PREFIX").unwrap();
	let inc_path = format!("-I{bob_prefix}/include");

	cc::Build::new()
		.file(vdriver_src)
		.flag(&inc_path)
		.compile("vdriver");

	// Generate bindings.

	let bindings = bindgen::Builder::default()
		.header(vdriver_header)
		.header(win_header)
		.clang_arg(inc_path)
		.blocklist_item("VDRIVER")
		.generate_comments(true)
		.generate()
		.expect("Unable to generate bindings");

	bindings
		.write_to_file(out_path.join("bindings.rs"))
		.expect("Couldn't write bindings");
}
