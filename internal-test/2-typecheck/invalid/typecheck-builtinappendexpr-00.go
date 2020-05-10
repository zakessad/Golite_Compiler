// Invalid expression: var _ = append([]int, rune)

package main

func main() {
    var x []int

    x = append(x, 'A') // Invalid builtin append expression
}
