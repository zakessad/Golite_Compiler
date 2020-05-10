//~true
//~false
//~false
package main

func main() {
	var a, b [5]int
	println(a == b)
	a[0] = 3
	println(a == b)
	c := a
	c[0] = 23
	println(a == c)
}
