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
	ApiVers     uint64
	BestApiVers uint64
	Name        string
}

type VdevDescr struct {
	internal C.kos_vdev_descr_t

	Spec         string
	Vers         uint32
	Human        string
	VdriverHuman string
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
		ApiVers:     uint64(descr.api_vers),
		BestApiVers: uint64(descr.best_api_vers),
		Name:        C.GoString((*C.char)(unsafe.Pointer(&descr.name[0]))),
	}
}

func (c *Component) NewVdevIter() *VdevIter {
	it := C.aqua_vdev_it(c.comp)
	return &VdevIter{it: it}
}

func (it *VdevIter) Next() *VdevDescr {
	if it.it.vdev == nil {
		return nil
	}

	vdev := (*C.kos_vdev_descr_t)(unsafe.Pointer(it.it.vdev))
	C.aqua_vdev_it_next(&it.it)

	return &VdevDescr{
		internal:     *vdev,
		Spec:         C.GoString((*C.char)(unsafe.Pointer(&vdev.spec[0]))),
		Vers:         uint32(vdev.vers),
		Human:        C.GoString((*C.char)(unsafe.Pointer(&vdev.human[0]))),
		VdriverHuman: C.GoString((*C.char)(unsafe.Pointer(&vdev.vdriver_human[0]))),
	}
}
