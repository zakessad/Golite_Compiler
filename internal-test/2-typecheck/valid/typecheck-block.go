package main

func f() {
	// Simple block
	{
		var x int = 4 + 7
		var y float64 = float64(x)
		var s string = "hello"
		s += " world"
	}

	var x = "x is a string and y is a bool here"
	var y = true
	z := 6.5
	// Shadowing block
	{
		var x = 8
		y := "y"

		x /= 2
		y += " got shadowed"
		z /= 2.3
	}
}