// Badly typed post statement
package main

func f() {
	for x := "10"; 5 > 10; x++ {
		println("This won't run.")
	}
}