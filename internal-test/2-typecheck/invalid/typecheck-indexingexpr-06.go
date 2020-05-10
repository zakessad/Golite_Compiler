// Invalid expression: ? <- ([]int type)[int]

package main

func main() {
    type A []int

    var x = A[0] // Invalid indexing expression
}
