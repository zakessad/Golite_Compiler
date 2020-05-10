// Mixed short decl with invalid assignment
package main

func f() {
	var x, z int
	x, y, z := 2, "text", "more text"
}