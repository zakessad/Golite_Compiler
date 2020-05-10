// Invalid expression: ? <- ([]int)[[]int]

package main

func main() {
    var x []int
    x = append(x, 1)
    x = append(x, 2)
    x = append(x, 3)
    var y []int
    y = append(y, 1)
    y = append(y, 2)
    y = append(y, 3)

    var z = x[y] // Invalid indexing expression
}
