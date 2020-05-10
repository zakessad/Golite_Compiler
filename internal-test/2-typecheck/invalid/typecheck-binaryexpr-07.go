// Invalid expression: ? <- B >= C [ type A int; type B A; type C A ]

package main

func main() {
    type (
        A int
        B A
        C A
    )
    var (
        x B = B(A(1))
        y C = C(A(1))
    )

    var z = x >= y // Invalid comparison
}
