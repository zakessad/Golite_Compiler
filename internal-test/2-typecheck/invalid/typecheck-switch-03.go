// Deeply defined non-comparable condition
package main

func f() {
	type a float64
	type b [2][10]a
	type c [3][][8]b
	type d struct {
		x, y a
		u, v b
		w string
		z c
	}

	var condition d

	switch condition {
	default: println("This doesn't do anything.");
	}
}