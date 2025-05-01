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

	// Create window.

	win := win_ctx.Create()
	win.Loop()
}
