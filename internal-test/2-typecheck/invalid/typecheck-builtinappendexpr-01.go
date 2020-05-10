// Invalid expression: var _ = append([]int, []int)

package main

func main() {
    var x []int
    var y []int

    x = append(x, y) // Invalid builtin append expression
}
