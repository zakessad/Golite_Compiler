// Invalid expression: ? <- (A type)(C) [ type A int; type B []A; type C B ]

package main

func main() {
    type (
        A int
        B []A
        C B
    )

    var x C
    x = append(x, A(1))
    x = append(x, A(2))
    x = append(x, A(3))

    var y = A(x) // Invalid typecast expression
}
