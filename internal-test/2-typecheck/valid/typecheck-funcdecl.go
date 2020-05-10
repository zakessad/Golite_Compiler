package main

// No arguments, no return type, no body
func f0() {}

func f1(a int) float64 {
	a *= int(5.4)
	return float64(a)
}

func f2(f2, a string, b, c, d int, e struct { x rune; }) [10]bool {
	var ret [10]bool
	return ret
}

func f3() struct {
	x, y int
	z struct {
		name string
	}
} {
	var ret struct {
		x, y int
		z struct {
			name string
		}
	}
	return ret
}