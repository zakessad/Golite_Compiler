// Invalid expression: var _ = append([3]int, int)

package main

func main() {
    var x [3]int
    x[0] = 1
    x[1] = 2
    x[2] = 3

    x = append(x, 4) // Invalid builtin append expression
}
