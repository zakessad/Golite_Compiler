package main

// Void return type
func f0() {
	// return
}

// Simple return type
func f1() int {
	return 5
}

// Array return type
func f2() [10]float64 {
	var ret [10]float64
	return ret
}

// Slice return type
func f3() []string {
	var ret []string
	ret_val := "Some text"
	ret = append(ret, ret_val)
	return ret
}

// Struct return type
func f4() struct {x int;} {
	var ret struct {
		x int
	}
	return ret
}

// Simple defined return type
type i int
func f5() i {
	return i(7)
}

// Complex defined return type
type c [7]struct {v float64;}
func f6() c {
	var ret c
	return ret
}