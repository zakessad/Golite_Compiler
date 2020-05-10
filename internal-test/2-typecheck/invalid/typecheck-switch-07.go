// Incompatible case struct expression
package main

func f() {
	var x struct { a int; }
	var y struct { b int; }
	
	switch x {
	case y: print("This case too is no bueno")
	default: println("This doesn't do anything.")
	}
}