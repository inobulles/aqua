// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

/*
#include <stdint.h>
*/
import "C"
import "runtime/cgo"

type ElemKind int

const (
	ElemKindDiv ElemKind = iota
	ElemKindText
)

type IElem interface {
	// Recalculate the size and position of the element.
	// If this element is a div, it will recursively do this for all its children too.
	reflow()

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

	attrs map[string]string

	mt, mb, ml, mr Dimension // Margin.
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
	content_align_x, content_align_y Align
}

func (d Div) defaults() Div {
	// No padding by default.

	d.pt = Dimension{}.zero()
	d.pb = Dimension{}.zero()
	d.pl = Dimension{}.zero()
	d.pr = Dimension{}.zero()

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

	d.flow_direction = AxisY
	d.flow_wrap = false
	d.content_align_x = AlignBegin
	d.content_align_y = AlignBegin

	return d
}

type Text struct {
	Elem
	text string
}

func (e *Elem) set_attr(key string, val string) {
	e.attrs[key] = val
}

func (e *Elem) rem_attr(key string) {
	delete(e.attrs, key)
}

func elem_from_raw(raw C.uintptr_t) any {
	handle := cgo.Handle(raw)
	return handle.Value()
}

func (d *Div) ElemBase() *Elem  { return &d.Elem }
func (t *Text) ElemBase() *Elem { return &t.Elem }
