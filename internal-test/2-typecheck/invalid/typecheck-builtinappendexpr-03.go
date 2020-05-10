// Invalid expression: var _ = append([]C, int) [ type A int; type B []A; type C B ]

package main

func main() {
    type (
        A int
        B []A
        C B
    )

    var x C

    x = append(x, 1) // Invalid builtin append expression
}
