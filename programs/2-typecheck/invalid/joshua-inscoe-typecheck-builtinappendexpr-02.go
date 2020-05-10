// Invalid expression: var _ = append(string, rune)

package main

func main() {
    var x = "Hello, world"

    x = append(x, '!') // Invalid builtin append expression
}
