// Invalid expression: var _ = append([]int)

package main

func main() {
    var x []int

    x = append(x) // Invalid builtin append expression
}
