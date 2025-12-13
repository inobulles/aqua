// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

type DimensionUnits int

const (
	// DimensionKindZero is a dimension of 0.
	// The units don't matter.
	DimensionUnitsZero DimensionUnits = iota
	// A fraction of the parent's size.
	DimensionUnitsParentFraction
	// Size in pixels.
	DimensionUnitsPixels
)

type Dimension struct {
	kind DimensionUnits
	val  float32
}

func (Dimension) zero() Dimension {
	return Dimension{
		kind: DimensionUnitsZero,
		val:  0,
	}
}

func (Dimension) full() Dimension {
	return Dimension{
		kind: DimensionUnitsParentFraction,
		val:  1,
	}
}
