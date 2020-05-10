// Invalid expression: ? <- [5]float64 / float64

package main

func main() {
    var x [5]float64
    x[0] = 1.0
    x[1] = 2.0
    x[2] = 3.0
    x[3] = 4.0
    x[4] = 5.0

    var y = x / 3.0 // Invalid operation
}
