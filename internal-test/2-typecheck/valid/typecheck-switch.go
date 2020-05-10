package main

func f() {
	// No condition switch
	switch {
	case true: println("This is weird")
	default: ;
	}

	// Simple switch
	switch 10 {
	default: ;
	}

	// Variable condition
	x := 5.8
	switch x {
	case 0.3: 
		print("Some case")
		break
	default: ;
	}

	// Defined type condition
	type letters string
	var y letters
	switch y {
	case letters("some case"):
		x++
	default: ;
	}

	// Struct type condition
	var z struct {
		a, b int
		name string
	}
	type c_struct struct {
		a, b int
		name string
	}
	var c, d struct {
		a, b int
		name string
	}
	switch z {
	case c, d:
		println("This is a complex switch.")
	default: 
		println("This might execute.")
	}

	// Empty struct type condition
	var w struct {}
	var v struct {}
	switch w {
	case v: print("This is useless but it shohuld work")
	default: ;
	}
}