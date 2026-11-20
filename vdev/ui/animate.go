// This Source Form is subject to the terms of the AQUA Software License, v. 1.0.
// Copyright (c) 2026 Aymeric Wibo

package main

import (
	"fmt"
	"math"
	"os"
	"strings"
	"time"
)

type AnimCurve int

const (
	// Currently, linear is the only time-based curve.
	AnimCurveLinear AnimCurve = iota
	AnimCurveElastic
)

const (
	// Stiffness is in s^-2.
	// It is the square of the natural angular frequency, so to get a stiffness value for a certain frequency you must use the following formula:
	// stiffness = (tau*frequency)^2
	ANIM_DEFAULT_STIFFNESS = 750

	// Damping is in s^-1.
	// It's what controls the overshoot, which can be gotten from the damping ratio (zeta):
	// zeta = damping / (2 * sqrt(stiffness))
	// - zeta <1 is "underdamped", so the spring will overshoot and oscillate.
	// - zeta =1 is critically damped, which is fastest way to reach the target without overshooting.
	// - zeta >1 is "overdamped", so the spring still won't overshoot, but will reach the target slower than 1.
	// Overshoot percentage may be computed with the following formula:
	// overshoot = e^(-pi*zeta/sqrt(1-zeta^2))
	ANIM_DEFAULT_DAMPING = 25

	// In seconds.
	ANIM_DEFAULT_TIME = 0.2

	// Thresholds under which an animation is considered settled and snapped to its target.
	// These are relative to the distance of the last retarget, so they work the same for pixels and scale factors.

	ANIM_SETTLE_POS = 1e-4
	ANIM_SETTLE_VEL = 1e-3
)

type Anim struct {
	pos float64
	vel float64

	target float64
	curve  AnimCurve

	// Where we were when the target was last set, and how long ago that was.
	// Only used by time-based curves.

	from    float64
	elapsed float64

	// Scale used for settle thresholds, i.e. the distance travelled on the last retarget.
	scale float64

	// When the animation was last advanced.
	last_update time.Time

	// For some curves, some of these values don't do anything.

	time      float64 // Duration of time-based curves, in seconds.
	stiffness float64 // In s^-2.
	damping   float64 // In s^-1.
}

func new_anim(pos float64, curve AnimCurve) Anim {
	return Anim{
		pos:       pos,
		target:    pos,
		from:      pos,
		curve:     curve,
		scale:     1,
		time:      ANIM_DEFAULT_TIME,
		stiffness: ANIM_DEFAULT_STIFFNESS,
		damping:   ANIM_DEFAULT_DAMPING,
	}
}

// Set a new target.
// This can be called at any point during the animation.
// Time-based curves just restart from the current position, so they will have a kink in velocity.
func (a *Anim) set_target(target float64) {
	if target == a.target {
		return
	}

	if a.settled() {
		a.last_update = time.Now()
	}

	a.target = target
	a.from = a.pos
	a.elapsed = 0

	a.scale = math.Abs(target - a.pos)

	if a.scale == 0 {
		a.scale = 1
	}
}

// Jump straight to a value without animating.
func (a *Anim) set(pos float64) {
	a.pos = pos
	a.vel = 0
	a.target = pos
	a.from = pos
	a.elapsed = a.time
}

func (a *Anim) settled() bool {
	return a.pos == a.target && a.vel == 0
}

// Advance an elastic curve by dt seconds using the closed-form solution of a damped spring.
// We solve from the current state (pos, vel) each time rather than from when the target was set, so retargeting mid-animation is nice and smooth.
func (a *Anim) step_elastic(dt float64) {
	x0 := a.pos - a.target
	v0 := a.vel

	decay := a.damping / 2
	disc := decay*decay - a.stiffness

	var x, v float64

	switch {
	// Critically damped (zeta = 1):
	// x(t) = e^(-a*t) * (x0 + (v0 + a*x0) * t)
	case math.Abs(disc) <= 1e-9*max(a.stiffness, decay*decay):
		e := math.Exp(-decay * dt)
		c := v0 + decay*x0

		x = e * (x0 + c*dt)
		v = e * (v0 - decay*c*dt)

	// Underdamped (zeta < 1), so oscillates at the damped angular frequency wd = sqrt(w0^2 - a^2):
	// x(t) = e^(-a*t) * (x0 * cos(wd*t) + (v0 + a*x0) / wd * sin(wd*t))
	case disc < 0:
		wd := math.Sqrt(-disc)
		e := math.Exp(-decay * dt)
		cos, sin := math.Cos(wd*dt), math.Sin(wd*dt)
		c := (v0 + decay*x0) / wd

		x = e * (x0*cos + c*sin)
		v = e * (v0*cos - (wd*x0+decay*c)*sin)

	// Overdamped (zeta > 1), so the sum of two decaying exponentials with rates r1 and r2:
	// x(t) = c1 * e^(r1*t) + c2 * e^(r2*t)
	default:
		root := math.Sqrt(disc)
		r1, r2 := -decay+root, -decay-root

		c1 := (v0 - r2*x0) / (r1 - r2)
		c2 := x0 - c1

		e1, e2 := math.Exp(r1*dt), math.Exp(r2*dt)

		x = c1*e1 + c2*e2
		v = r1*c1*e1 + r2*c2*e2
	}

	a.pos = a.target + x
	a.vel = v
}

// Advance the animation to the given time.
// Returns true if the animation is still running (i.e. something needs to be redrawn next frame).
func (a *Anim) update(now time.Time) bool {
	if a.settled() {
		return false
	}

	dt := max(now.Sub(a.last_update).Seconds(), 0)
	a.last_update = now

	switch a.curve {
	case AnimCurveLinear:
		prev := a.pos
		a.elapsed = min(a.elapsed+dt, a.time)

		if a.time <= 0 || a.elapsed >= a.time {
			a.pos = a.target
			a.vel = 0
			return false
		}

		a.pos = a.from + (a.target-a.from)*(a.elapsed/a.time)

		if dt > 0 {
			a.vel = (a.pos - prev) / dt
		}

	case AnimCurveElastic:
		a.step_elastic(dt)

		if math.Abs(a.target-a.pos) < ANIM_SETTLE_POS*a.scale && math.Abs(a.vel) < ANIM_SETTLE_VEL*a.scale {
			a.pos = a.target
			a.vel = 0
			return false
		}
	}

	return true
}

// Advance all animations in the UI to the current time.
// This should be called once per frame, before rendering.
// Returns true if any animation is still running.
func (ui *Ui) animate() bool {
	return animate_elem(&ui.root, time.Now())
}

func animate_elem(elem IElem, now time.Time) bool {
	base := elem.ElemBase()

	running := base.scale.update(now)
	running = base.rot.update(now) || running

	if div, ok := elem.(*Div); ok {
		for _, child := range div.children {
			running = animate_elem(child, now) || running
		}
	}

	return running
}

func (a *Anim) set_curve(curve AnimCurve) {
	a.curve = curve
	a.from = a.pos
	a.elapsed = 0
}

func (a *Anim) process_attrs(prop string, key string, val any) bool {
	rest, ok := strings.CutPrefix(key, prop)
	if !ok {
		return false
	}

	switch rest {
	case "":
		a.set(float64(val.(float32)))
	case ".anim":
		a.set_target(float64(val.(float32)))
	case ".anim.curve":
		switch val.(string) {
		case "linear":
			a.set_curve(AnimCurveLinear)
		case "elastic":
			a.set_curve(AnimCurveElastic)
		default:
			fmt.Fprintf(os.Stderr, "Unknown animation curve '%s' for '%s' (must be 'linear' or 'elastic').\n", val, key)
		}
	case ".anim.duration":
		a.time = float64(val.(float32))
	case ".anim.stiffness", ".anim.damping":
		v := float64(val.(float32))

		if v < 0 {
			fmt.Fprintf(os.Stderr, "'%s' can't be negative (got %g).\n", key, v)
			break
		}

		if rest == ".anim.stiffness" {
			a.stiffness = v
		} else {
			a.damping = v
		}
	default:
		return false
	}

	return true
}
