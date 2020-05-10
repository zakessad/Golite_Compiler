// Badly typed init statement
package main

func f() {
	for x := "true" && 1; 5 > 10; {
		println("This won't run.")
	}
}