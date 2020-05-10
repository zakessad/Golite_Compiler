// No condition switch with non-bool case expression
package main

func f() {
	switch {
	case 10: print("10 is not bool!")
	default: println("This doesn't do anything.")
	}
}