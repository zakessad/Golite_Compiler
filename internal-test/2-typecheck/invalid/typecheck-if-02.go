// Badly typed then statement
package main

func f() {
	if x := 5; true {
		x = "Invalid expression."
	}
}