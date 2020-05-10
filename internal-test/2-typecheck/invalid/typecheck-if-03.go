// Badly typed else statement
package main

func f() {
	var start string = "initial"
	if x := start ; true {
		x = "second"
	} else {
		x = 3.4
	}
}