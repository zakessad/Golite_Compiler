// Invalid expression: ? <- (struct { a, b int; }).(int type)

package main

func main() {
    var x = struct { a, b int; }

    var y = x.int // Invalid field selection expression
}
