// Invalid expression: var _ = append([]int, int, int)

package main

func main() {
    var x []int

    x = append(x, 1, 2) // Invalid builtin append expression
}
