// Badly typed statement inside block
package main

func f() {
	x := 7
	{
		x += "text"
	}
}