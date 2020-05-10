//~1 +2.345000e+001 97 13 hello hello\n
//~hello
//~ 23
//~true false

package main

func main() {
	println(1, 23.45, 'a', '\r', "hello", `hello\n`)
	println("hello\n", 23)
	println(true, false)
}
