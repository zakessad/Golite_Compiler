//~0 1 0
package main

var a [5]int
var b []int // b = append (b, 0)
var c struct {
	f int
}

func foo() [5]int {
	return a
}
func bar() []int {
	return b
}
func baz() struct {
	f int
} {
	return c
}

func init() {
	b = append(b, 0)
}
func main() {
	var d, e, f = foo(), bar(), baz()
	d[0], e[0], f.f = 1, 1, 1
	println(a[0], b[0], c.f)
}
