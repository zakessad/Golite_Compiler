// Non-comparable condition
package main

func f() {
	var x []int

	switch x {
	default: println("This doesn't do anything.");
	}
}