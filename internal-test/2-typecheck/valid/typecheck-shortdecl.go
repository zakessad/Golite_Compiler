package main

func f() {
	// Simple short declaration
	x := 5
	x += 7

	// Multiple simple declarations
	u, v, w := 5, 2.4, "text"
	u *= 2
	v /= 3.1
	w += ", more text"

	// Mixed assignments and short declarations
	type tf bool
	var a int
	var b tf = tf(false)
	a, b, c := -8, tf(true), "c is a string"
	a %= 2
	b = b || tf(true)
	c += ", c is a longer string"

	// Defined type short declaration
	type real float64
	d := real(3.3)
	d--
}