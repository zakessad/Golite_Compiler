// Incompatible case expression
package main

func f() {
	x := 10
	switch x {
	case 5.8: print("This case is no bueno")
	default: println("This doesn't do anything.")
	}
}