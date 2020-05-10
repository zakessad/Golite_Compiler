// Invalid expression: ? <- (float64)(int type)

package main

func main() {
    var x float64 = 1.0

    var y = x(int) // Invalid typecast expression
}
