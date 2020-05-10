package main

func f() {
	// Simple if statement
	if true {
		println("Hello there")
	}

	// Non-literal expression in condition
	var x int = 10
	if x >= 4 {
		println("General Kenobi")
	}

	// Non base boolean type
	type b bool
	i := 4
	condition := b(7 > i && true)
	if condition {
		i++
		f() // Recusive call because why not
	}

	// Init statement
	if y := 5 + 2 < 10; b(y) || b(true) {
		y = false
	}

	// Empty if statement
	if true {}

	// If / else
	if false {
		println("Since this won't run")
	} else {
		print("This will.")
		print("\n")
	}

	// If / else if / else
	type boolean b
	if val := boolean(false); val {
		println("part 1")
	} else if b(val && boolean(5 == 3 + 2)) {
		println("part 2")
	} else {
		println("part 3")
	}
}