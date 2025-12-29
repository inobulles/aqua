// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import (
	"fmt"
	"runtime"

	"obiw.ac/aqua"
)

func main() {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()

	ctx := aqua.Init()

	if ctx == nil {
		panic("Failed to initialize AQUA context.")
	}

	descr := ctx.GetKosDescr()
	fmt.Printf("AQUA context initialized successfully: KOS v%d, %s.\n", descr.ApiVers, descr.Name)

	// Get window VDEV.

	win_comp := ctx.WinInit()
	iter := win_comp.NewVdevIter()

	var found *aqua.VdevDescr

	for vdev := iter.Next(); vdev != nil; vdev = iter.Next() {
		fmt.Printf("Found window VDEV: %s (\"%s\", from \"%s\").\n", vdev.Spec, vdev.Human, vdev.VdriverHuman)
		found = vdev
	}

	if found == nil {
		panic("No window VDEV found.")
	}

	win_ctx := win_comp.Conn(found)

	// Get WebGPU VDEV.

	wgpu_comp := ctx.WgpuInit()
	iter = wgpu_comp.NewVdevIter()

	for vdev := iter.Next(); vdev != nil; vdev = iter.Next() {
		fmt.Printf("Found WebGPU VDEV: %s (\"%s\", from \"%s\").\n", vdev.Spec, vdev.Human, vdev.VdriverHuman)
		found = vdev
	}

	if found == nil {
		panic("No WebGPU VDEV found.")
	}

	wgpu_ctx := wgpu_comp.Conn(found)
	defer wgpu_ctx.Disconn()

	// Get UI VDEV.

	ui_comp := ctx.UiInit()
	iter = ui_comp.NewVdevIter()

	found = nil

	for vdev := iter.Next(); vdev != nil; vdev = iter.Next() {
		fmt.Printf("Found UI VDEV: %s (\"%s\", from \"%s\").\n", vdev.Spec, vdev.Human, vdev.VdriverHuman)
		found = vdev
	}

	if found == nil {
		panic("No UI VDEV found.")
	}

	ui_ctx := ui_comp.Conn(found)

	if ui_ctx.GetSupportedBackends()&aqua.UI_BACKEND_WGPU == 0 {
		panic("WebGPU UI backend is not supported.")
	}

	// Create window.

	win := win_ctx.Create()
	defer win.Destroy()

	// Create a UI.

	ui := ui_ctx.Create()
	defer ui.Destroy()

	root := ui.GetRoot()

	root.AddText("text.title", "Hello world!")
	root.AddText("text.paragraph", "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.")

	div := root.AddDiv("")

	div.SetAttr("min_w", aqua.UiDim{}.Pixels(200))
	div.SetAttr("min_h", aqua.UiDim{}.Pixels(200))

	div.SetAttr("bg.r", float32(1.0))
	div.SetAttr("bg.g", float32(0.0))
	div.SetAttr("bg.b", float32(1.0))
	div.SetAttr("bg.a", float32(1.0))

	// Set up UI backend.

	state, err := ui.WgpuEzSetup(win, wgpu_ctx)

	if err != nil {
		panic("UI WebGPU backend setup failed.")
	}

	// Start window loop.

	win.RegisterRedrawCb(func() {
		state.Render()
	})

	win.RegisterResizeCb(func(x_res, y_res uint32) {
		state.Resize(x_res, y_res)
	})

	win.Loop()
}
