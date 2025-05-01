// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package aqua

/*
#cgo LDFLAGS: -laqua_root

#include <aqua/root.h>
*/
import "C"
import "unsafe"

type Context struct {
	ctx C.aqua_ctx_t
}

type Component struct {
	comp C.aqua_component_t
}

type VdevIter struct {
	it C.aqua_vdev_it_t
}

type KosDescr struct {
	Api_vers      uint64
	Best_api_vers uint64
	Name          string
}

func Init() *Context {
	ctx := C.aqua_init()

	if ctx == nil {
		return nil
	}

	return &Context{ctx: ctx}
}

func (c *Context) GetKosDescr() KosDescr {
	descr := C.aqua_get_kos_descr(c.ctx)

	return KosDescr{
		Api_vers:      uint64(descr.api_vers),
		Best_api_vers: uint64(descr.best_api_vers),
		Name:          C.GoString((*C.char)(unsafe.Pointer(&descr.name[0]))),
	}
}

func (c *Component) NewVdevIter() *VdevIter {
	it := C.aqua_vdev_it(c.comp)
	return &VdevIter{it: it}
}

func (it *VdevIter) Next() unsafe.Pointer {
	if it.it.vdev == nil {
		return nil
	}

	vdev := unsafe.Pointer(it.it.vdev)
	C.aqua_vdev_it_next(&it.it)

	return vdev
}
