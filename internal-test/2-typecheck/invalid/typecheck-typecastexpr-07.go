// Invalid expression: ? <- (float64)(int)

package main

func main() {
    var x float64 = 1.0

    var y = x(1) // Invalid typecast expression
}
