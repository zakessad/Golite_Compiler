// Invalid expression: ? <- cap([]int, []int)

package main

func main() {
    var x []int
    var y []int

    var z = cap(x, y) // Invalid builtin cap expression
}
