// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import "fmt"

func align_off(align Align, container_size uint32, elem_size uint32) uint32 {
	switch align {
	case AlignBegin:
		return 0
	case AlignCentre:
		return container_size/2 - elem_size/2
	case AlignEnd:
		return container_size - elem_size
	default:
		panic(fmt.Sprintf("unexpected main.Align: %#v", align))
	}
}

func (e *Div) reflow(max_w, max_h uint32) {
	// TODO Find max size of div here and pass that down.
	// Text can then wrap to fit that size.

	pl := e.dimension_to_px_x(e.pl)
	pr := e.dimension_to_px_x(e.pr)
	pt := e.dimension_to_px_y(e.pt)
	pb := e.dimension_to_px_y(e.pb)

	flow_x := pl
	flow_y := pt

	for _, child := range e.children {
		child.reflow(max_w-pl-pr, max_h-pt-pb)

		if child.ElemBase().is_abs {
			continue // Take element out of flow if absolutely positioned.
		}

		cw, ch := child.ElemBase().flow_w, child.ElemBase().flow_h

		child.ElemBase().flow_x = flow_x
		child.ElemBase().flow_y = flow_y

		switch e.flow_direction {
		case AxisY: // Elements flow from top to bottom.
			flow_y += ch
			flow_y += e.dimension_to_px_y(e.gap_y)

			child.ElemBase().flow_x = flow_x + align_off(
				e.content_align_x,
				max_w,
				child.ElemBase().flow_w,
			)
		case AxisX: // Elements flow from left to right.
			flow_x += cw
			flow_x += e.dimension_to_px_x(e.gap_x)

			child.ElemBase().flow_y = flow_y + align_off(
				e.content_align_y,
				max_h,
				child.ElemBase().flow_h,
			)
		default:
			panic(fmt.Sprintf("unexpected main.Axis: %#v", e.flow_direction))
		}
	}

	// Make sure div is not smaller than minimum size.

	if min_w := e.get_attr("min_w"); min_w != nil {
		e.flow_w = max(e.flow_w, e.dimension_to_px_x(min_w.(Dimension)))
	}
	if min_h := e.get_attr("min_h"); min_h != nil {
		e.flow_h = max(e.flow_h, e.dimension_to_px_y(min_h.(Dimension)))
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
