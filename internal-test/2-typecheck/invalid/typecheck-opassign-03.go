// Incompatible op assignment type
package main

func f() {
	var x, y bool
	y = false
	x &= y
}