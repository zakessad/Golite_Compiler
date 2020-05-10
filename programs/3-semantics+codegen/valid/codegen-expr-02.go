//~4 -2 3 3 1 +5.000000e-001
//~16 8 3 3 7 16
//~true false true false true true
//~true true
package main

func main() {
	var x, y = 1, 3
	// arithmetic
	println(x+y, x-y, x*y, y/x, 213451%2, 1.0/2.0)
	// binary
	println(1<<4, 32>>2, 0xF&0x23, 1|2, 8^15, 0x12&^0xF)
	// comparison
	println(1 > 0, 1 < 0, 12 == 12, 12 != 12, 1 <= 1, 2 >= 0)
	// logical
	println((1 == 1) && (1 > 0), ((1 == 0) || (2 >= 2)))

}
