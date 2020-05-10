// Incompatible case array expression
package main

func f() {
	var x [7]int
	var y [4]int
	switch x {
	case y: print("This case is also no bueno")
	default: println("This doesn't do anything.")
	}
}