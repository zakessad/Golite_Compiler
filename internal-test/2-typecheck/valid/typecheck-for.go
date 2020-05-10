package main

func f() {
	// Empty for loop
	for {}

	// Infinite for loop
	print("This loop goes on ")
	for {
		print("and on ")
	}

	// Single condition for loop (while loop)
	x := 10
	for 5 < x {
		x--
	}

	// Three part for loop
	type integer int
	type count_tracker struct { 
		count integer
		max int
	}
	var ct count_tracker
	for ct.count = integer(0); ct.count <= integer(ct.max); ct.count++ {
		print("Count: ")
		println(ct.count)
		print("Cycles until end: ")
		println(ct.max - int(ct.count))
	}

	// Defined type condition
	type b bool
	for c := b(false); c; {
		print("Generic test text.")
	}
}