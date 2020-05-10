package main

func f() {
	// Simple increment/decrement
	var x int = 4
	x++
	x--

	// Increment/decrement on defined type
	type real float64
	var y = real(3.7)
	y++
	y--

	// Increment/decrement on short decl var
	z := 2
	z++
	z--
}