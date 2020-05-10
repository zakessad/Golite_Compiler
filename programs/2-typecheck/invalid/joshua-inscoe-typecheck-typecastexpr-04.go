// Invalid expression: ? <- (A type)(struct { a, b int; }) [ type A struct { a, b int; } ]

package main

func main() {
    type A struct { a, b int; }

    var x struct { a, b int; }
    x.a = 1
    x.b = 2

    var y = A(x) // Invalid typecast expression
}
