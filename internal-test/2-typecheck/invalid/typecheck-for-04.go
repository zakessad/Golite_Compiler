// Badly typed condition
package main

func f() {
	for true > false {
		println("This won't run.")
	}
}