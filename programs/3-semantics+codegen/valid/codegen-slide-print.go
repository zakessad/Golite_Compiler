//~255 255 +1.200000e-001 true 76 hello 
//~hello \n
package main

func main() {

	// Integers
	print(255, " ")  // 255
	print(0377, " ") // 255
	// Floats
	print(0.12, " ") // +1.200000e-001
	// Booleans
	print(true, " ") // true
	// Runes
	print('L', " ") // 76
	// Strings
	print("hello \n") // hello [newline]
	print(`hello \n`) // hello\n
}
