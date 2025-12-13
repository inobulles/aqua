// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2025 Aymeric Wibo

package main

import (
	"image"
	"image/color"
	"image/draw"
	"os"
	"strings"

	"golang.org/x/image/font"
	"golang.org/x/image/font/opentype"
	"golang.org/x/image/math/fixed"
)

type Font struct {
	face font.Face
}

// Load a TTF or OTF file at given size (px).
func NewFontFromFile(path string, size float64) (*Font, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		println("Couldn't read file.", path)
		return nil, err
	}

	fnt, err := opentype.Parse(data)
	if err != nil {
		println("Couldn't parse file as TTF.")
		return nil, err
	}

	face, err := opentype.NewFace(fnt, &opentype.FaceOptions{
		Size:    size,
		DPI:     72,
		Hinting: font.HintingFull,
	})
	if err != nil {
		println("Couldn't create new font face.")
		return nil, err
	}

	return &Font{face}, nil
}

// Measure wrapped text. maxWidth = max line width before wrapping.
// If maxWidth <= 0, no wrapping is applied.
func (f *Font) Measure(text string, maxWidth int) (uint32, uint32) {
	dr := &font.Drawer{Face: f.face}

	lineHeight := f.face.Metrics().Height.Ceil()
	spaceWidth := dr.MeasureString(" ").Ceil()

	var width, height int
	height = lineHeight

	words := strings.Fields(text)
	curWidth := 0

	for _, w := range words {
		wWidth := dr.MeasureString(w).Ceil()

		// Line wrap
		if maxWidth > 0 && curWidth+wWidth > maxWidth {
			if curWidth > width {
				width = curWidth
			}
			height += lineHeight
			curWidth = 0
		}

		// Add word width + space
		if curWidth == 0 {
			curWidth = wWidth
		} else {
			curWidth += spaceWidth + wWidth
		}
	}

	if curWidth > width {
		width = curWidth
	}

	return uint32(width), uint32(height)
}

// Render text into an RGBA image. Size is automatically determined.
func (f *Font) Render(text string, maxWidth int) *image.RGBA {
	w, h := f.Measure(text, maxWidth)
	if w == 0 || h == 0 {
		w, h = 1, 1
	}

	img := image.NewRGBA(image.Rect(0, 0, int(w), int(h)))
	draw.Draw(img, img.Bounds(), &image.Uniform{color.Transparent}, image.Point{}, draw.Src)

	dr := &font.Drawer{
		Dst:  img,
		Src:  image.White, // text color
		Face: f.face,
		Dot:  fixed.P(0, f.face.Metrics().Ascent.Ceil()),
	}

	space := dr.MeasureString(" ").Ceil()
	lineHeight := f.face.Metrics().Height.Ceil()

	words := strings.Fields(text)
	x := 0
	y := f.face.Metrics().Ascent.Ceil()

	for _, word := range words {
		wWidth := dr.MeasureString(word).Ceil()

		// Wrap
		if maxWidth > 0 && x+wWidth > maxWidth {
			x = 0
			y += lineHeight
			dr.Dot = fixed.P(x, y)
		}

		// Draw
		dr.Dot = fixed.P(x, y)
		dr.DrawString(word)

		x += wWidth + space
	}

	return img
}
