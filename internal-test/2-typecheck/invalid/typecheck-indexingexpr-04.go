// Invalid expression: ? <- ([]int)[F] [ type A int; type B A; type C B; type D []C; type E D; type F E ]

package main

func main() {
    type (
        A int
        B A
        C B
        D []C
        E D
        F E
    )

    var x []int
    x = append(x, 1)
    x = append(x, 2)
    x = append(x, 3)

    var y F

    var z = x[y] // Invalid indexing expression
}
