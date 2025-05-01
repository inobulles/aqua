// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import "fmt"
import "obiw.ac/aqua"

func main() {
	ctx := aqua.Init()

	if ctx == nil {
		fmt.Println("Failed to initialize AQUA context")
		return
	}

	descr := ctx.GetKosDescr()
	fmt.Printf("AQUA context initialized successfully: KOS v%d, %s\n", descr.ApiVers, descr.Name)

	comp := ctx.UiInit()
	iter := comp.NewVdevIter()

	for {
		vdev := iter.Next()

		if vdev == nil {
			break
		}

		fmt.Printf("Found vdev: %s (\"%s\", from \"%s\")\n", vdev.Spec, vdev.Human, vdev.VdriverHuman)
	}
}
