// Struct with non-comparable field as condition
package main

func f() {
	var x struct {
		a int
		b []string
	}

	switch x {
	default: println("This doesn't do anything.");
	}
}