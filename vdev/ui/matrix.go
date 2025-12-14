// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import "math"

func mul_mat(a [4][4]float32, b [4][4]float32) [4][4]float32 {
	var res [4][4]float32

	for i := range 4 {
		for j := range 4 {
			res[i][j] = 0

			for k := range 4 {
				res[i][j] += a[k][j] * b[i][k]
			}
		}
	}

	return res
}

func rot_mat(angle float32) [4][4]float32 {
	c := float32(math.Cos(float64(angle)))
	s := float32(math.Sin(float64(angle)))

	return [4][4]float32{
		{c, s, 0, 0},
		{-s, c, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1},
	}
}
