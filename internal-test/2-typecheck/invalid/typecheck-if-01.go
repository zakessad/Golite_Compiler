// Badly typed init statement
package main

func f() {
	var x int
	if x = "false"; true {
		println("This won't work.")
	}
}