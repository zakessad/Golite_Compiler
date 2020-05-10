// Invalid expression: ? <- A ^ int [ type A int ]

package main

func main() {
    type A int
    var x A = A(int('A'))

    var y = x ^ 0x20 // Invalid operation
}
