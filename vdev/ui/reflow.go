// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

func (e Div) reflow() {
	// TODO Find max size of div here and pass that down.
	// Text can then wrap to fit that size.

	for _, child := range e.children {
		child.reflow()
	}
}

func (e Text) reflow() {
	// ui := e.ui
	// ui.backend.calculate_size(e, 0, 0)
}

func (ui *Ui) reflow() {
	// We only reflow if the UI is dirty (i.e. needs a reflow).
	// TODO Is it better design to have the caller check for this or does it not actually matter? Find arguments for and against.

	if !ui.dirty {
		return
	}

	ui.dirty = false
	ui.root.reflow()

	// Get initial size of leaves.
	// We gonna wanna do a DFS and all that for inputs.
	// See and learn from the mistakes I made in the past: https://github.com/obiwac/aquabsd-private/blob/28ccd2260716827facd09f22bf10a5ec912d10ad/components/experimental-devices/aquabsd.alps.ui/functions.h
}
