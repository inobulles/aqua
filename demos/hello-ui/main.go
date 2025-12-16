// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import "fmt"
import "obiw.ac/aqua"

func main() {
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
