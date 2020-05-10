// Invalid expression: ? <- (int)[int]

package main

func main() {
    var x int = 1

    var y = x[0] // Invalid indexing expression
}
