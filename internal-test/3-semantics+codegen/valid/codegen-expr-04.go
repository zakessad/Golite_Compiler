//~foo
package main

func foo() int {
	println("foo")
	return 0
}

func main() {
	var _ = foo()
}
