// Invalid expression: ? <- (A).c [ type A struct { a, b int; } ]

package main

func main() {
    type A struct { a, b int; }

    var x A
    x.a = 1
    x.b = 2

    var y = A.c // Invalid field selection expression
}
