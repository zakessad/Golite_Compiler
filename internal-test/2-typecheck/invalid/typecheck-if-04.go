// Non-boolean else if condition
package main

func f() {
	if x := 5; true {
		x *= 2
	} else if x + 7 {
		println("This won't work either.")
	}
}