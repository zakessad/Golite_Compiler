// Invalid expression: ? <- B > C [ type B int; type C int ]

package main

func main() {
    type (
        B int
        C int
    )
    var (
        x B = B(1)
        y C = C(1)
    )

    var z = x > y // Invalid comparison
}
