// Incompatible assignment type
package main

// Simple valid assignment
func f() {
	var x int
	x = 7
}

// Multiple valid assignments
func f2() {
	var x, y, z int
	x, y, z = 1, 2, 3
}

// Indexing and slice type assignments
func f3() {
	var x int
	var y, y_value []int
	var z [10]string
	x, y, z[7] = 1, y_value, "text"
}

// Struct and field selection assignments
func f4() {
	type real float64
	type test_struct struct {
		x, y int
		name string
		nested struct {
			z real
		}
	}

	var c test_struct
	c.x = 5
	c.y, c.name = 7, "some struct"
	c.nested.z = real(2.7)
}