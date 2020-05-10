// Invalid expression: ? <- (int)()

package main

func main() {
    var x int = 1

    var y = x() // Invalid function call
}
