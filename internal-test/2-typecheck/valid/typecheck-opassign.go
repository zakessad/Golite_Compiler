package main

func f() {
	// Valid op assign for += (strings only)
	var s string = "hello"
	s += " world"

	// Valid op assing for +=, -=, *=, /=
	var x float64
	x += 5.5

	// Valid op assign for %=, |=, &=, >>=, <<=, &^=, ^=
	var a, b int
	a &= b
}