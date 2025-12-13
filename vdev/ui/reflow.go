// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

func (e Div) reflow(max_w, max_h uint32) {
	// TODO Find max size of div here and pass that down.
	// Text can then wrap to fit that size.

	flow_x := e.dimension_to_px(e.pl)
	flow_y := e.dimension_to_px(e.pt)

	for _, child := range e.children {
		child.reflow(max_w, max_h)

		child.ElemBase().flow_x = flow_x
		child.ElemBase().flow_y = flow_y

		flow_y += child.ElemBase().flow_h
	}
}

func (e *Text) reflow(max_w, max_h uint32) {
	ui := e.ui
	e.flow_w, e.flow_h = ui.backend.calculate_size(e, max_w, max_h)
}

func (ui *Ui) reflow(x_res, y_res uint32) {
	// We only reflow if the UI is dirty (i.e. needs a reflow).

	if !ui.dirty {
		panic("UI should only be reflowed when dirty!")
	}

	ui.root.flow_w = x_res
	ui.root.flow_h = y_res

	ui.root.flow_x = 0
	ui.root.flow_y = 0

	ui.root.reflow(x_res, y_res)

	// Get initial size of leaves.
	// We gonna wanna do a DFS and all that for inputs.
	// See and learn from the mistakes I made in the past: https://github.com/obiwac/aquabsd-private/blob/28ccd2260716827facd09f22bf10a5ec912d10ad/components/experimental-devices/aquabsd.alps.ui/functions.h
}
