// Invalid expression: ? <- (A type).a [ type A struct { a, b int; } ]

package main

func main() {
    type A struct { a, b int; }

    var x = A.a // Invalid field selection expression
}
