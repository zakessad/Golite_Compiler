//~init1
//~init2
//~init3
//~main
package pkg

func init() {
	println("init1")
}

func f() {
	var x = 1
}

func init() {
	println("init2")
}

func init() {
	println("init3")
}

func main() {
	println("main")
}
