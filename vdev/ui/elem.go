// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#include <stdint.h>
*/
import "C"
import (
	"fmt"
	"runtime/cgo"
)

type ElemKind int

const (
	ElemKindDiv ElemKind = iota
	ElemKindText
)

type IElem interface {
	// Recalculate the size and position of the element.
	// If this element is a div, it will recursively do this for all its children too.
	// The element will not exceed the dimensions given by the max_w and max_h arguments.
	reflow(max_w, max_h uint32)

	// Get base element struct.
	ElemBase() *Elem
}

type Elem struct {
	IElem
	kind ElemKind // TODO Necessary?

	ui     *Ui
	parent IElem

	// Attributes are for all optional attributes.
	// Obligatory attributes are all fields of their relevant structs instead.
	// TODO Should we even do this? Or just have everything as fields?

	attrs map[string]any

	mt, mb, ml, mr Dimension // Margin.

	// Position & size of the element after reflowing, in screen pixels.
	// This is relative to the parent.

	flow_x, flow_y uint32
	flow_w, flow_h uint32

	backend_data any
}

type OverflowKind int

const (
	// Allow the children elements to overflow and clip the excess.
	OverflowKindClip OverflowKind = iota
	// Allow the children elements to overflow and scroll.
	OverflowKindScroll
)

type Align int

const (
	// Align the element to the "beginning" of the parent on the axis it applies to.
	// On the X axis, we consider the "beginning" to be the left.
	// On the Y axis, we consider the "beginning" to be the top.
	AlignBegin Align = iota

	// Align the element to the centre of the parent on the axis it applies to.
	AlignCentre

	// Align the element to the "end" of the parent on the axis it applies to.
	// On the X axis, we consider the "end" to be the right.
	// On the Y axis, we consider the "end" to be the bottom.
	AlignEnd
)

type Axis int

const (
	// The horizontal axis.
	AxisX Axis = iota
	// The vertical axis.
	AxisY
)

type Div struct {
	Elem
	children []IElem

	pt, pb, pl, pr         Dimension // Padding.
	max_w, max_h           Dimension // Maximum width and height.
	overflow_x, overflow_y OverflowKind

	// Layout fields.

	flow_direction                   Axis
	flow_wrap                        bool
	gap_x, gap_y                     Dimension // Spacing between elements.
	content_align_x, content_align_y Align
}

func (d Div) defaults() Div {
	d.pt = Dimension{}.pixels(10)
	d.pb = Dimension{}.pixels(10)
	d.pl = Dimension{}.pixels(20)
	d.pr = Dimension{}.pixels(20)

	// Take up full width and height of the parent by default.
	// TODO We need to see how the shrinking works, but we'll probably want to do something like the min width always being full, but the height being as small as the children (with a max width of the height so overflow is scrolled, on the root div at least).

	d.max_w = Dimension{}.full()
	d.max_h = Dimension{}.full()

	// Clip X axis overflow by default, but allow scrolling on the Y axis.

	d.overflow_x = OverflowKindClip
	d.overflow_y = OverflowKindScroll

	// Page-style flow by default.
	// Content starts at the top left and flows downwards.
	// We don't want to wrap by default because that would mean starting a new column up top once we reach the bottom.

	d.gap_x = Dimension{}.pixels(10)
	d.gap_y = Dimension{}.pixels(10)

	d.flow_direction = AxisY
	d.flow_wrap = false
	d.content_align_x = AlignBegin
	d.content_align_y = AlignBegin

	return d
}

type TextSemantic int

const (
	TextSemanticTitle TextSemantic = iota
	TextSemanticParagraph
)

type Text struct {
	Elem
	text     string
	semantic TextSemantic
}

func (t Text) construct(ui *Ui, parent IElem, text string, semantic_str string) *Text {
	var semantic TextSemantic

	switch semantic_str {
	case "text.title":
		semantic = TextSemanticTitle
	default:
		fmt.Errorf("Unknown text semantic '%s'. Defaulting to 'text.paragraph'.", semantic_str)
	case "text.paragraph":
		semantic = TextSemanticParagraph
	}

	return &Text{
		Elem: Elem{
			kind:   ElemKindText,
			ui:     ui,
			parent: parent,
			attrs:  make(map[string]any),
		},
		text:     text,
		semantic: semantic,
	}
}

func (e *Elem) set_attr(key string, val any) {
	e.attrs[key] = val
}

func (e *Elem) rem_attr(key string) {
	delete(e.attrs, key)
}

func (e *Elem) get_attr(key string) any {
	if v, ok := e.attrs[key]; ok {
		return v
	}

	return nil
}

func elem_from_raw(raw C.uintptr_t) any {
	handle := cgo.Handle(raw)
	return handle.Value()
}

func (e *Elem) dimension_to_px(d Dimension) uint32 {
	switch d.kind {
	case DimensionUnitsZero:
		return 0
	case DimensionUnitsPixels:
		return uint32(d.val)
	case DimensionUnitsParentFraction:
		panic("TODO")
	}

	panic("Unknown dimension kind.")
}

func (d *Div) ElemBase() *Elem  { return &d.Elem }
func (t *Text) ElemBase() *Elem { return &t.Elem }
