// Badly typed init statement
package main

func f() {
	x := 10
	switch x += "5"; "some condition" {
	default: println("This doesn't do anything.")
	}
}