// Invalid case expresion with same resolved type
package main

func f() {
	type i int
	var x i = i(1)
	
	switch x {
	case 10: print("Almost good")
	default: println("This doesn't do anything.")
	}
}