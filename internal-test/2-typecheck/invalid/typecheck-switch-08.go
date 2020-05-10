// Mixed valid and invalid case expressions
package main

func f() {
	var x = "What's 12 - 8 / 4?"
	
	switch x {
	case "1": print("Ya dumb")
	case "10", 10: print("There you go")
	default: println("This doesn't do anything.")
	}
}