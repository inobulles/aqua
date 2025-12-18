// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

type Raster struct {
	x_res, y_res uint32
	data         []byte
}
