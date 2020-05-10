// Incompatible op assignment type
package main

func f() {
	var x, y string
	y = "text"
	x &= y
}