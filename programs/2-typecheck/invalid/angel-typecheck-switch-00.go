// Badly typed condition
package main

func f() {
	switch 5.5 &^ 8 {
	default: println("This doesn't do anything.");
	}
}