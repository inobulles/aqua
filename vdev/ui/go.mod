module aquabsd.black.ui

go 1.24.0

require (
	golang.org/x/image v0.34.0
	obiw.ac/aqua/wgpu v0.0.0
)

require (
	golang.org/x/text v0.32.0 // indirect
	obiw.ac/aqua v0.0.0 // indirect
)

replace obiw.ac/aqua/wgpu => ../../external/go-webgpu

replace obiw.ac/aqua => ../../lib/bindings/go
