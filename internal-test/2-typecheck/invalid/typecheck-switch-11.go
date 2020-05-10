// No condition switch with non-bool defined type case expression
package main

func f() {
	type b bool

	switch {
	case b(true): print("This is also not bool!")
	default: println("This doesn't do anything.")
	}
}