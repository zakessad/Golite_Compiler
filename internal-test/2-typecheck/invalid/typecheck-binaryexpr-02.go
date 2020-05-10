// Invalid expression: ? <- struct { a, b int; } == struct { b, a int; }

package main

func main() {
    var x struct { a, b int; }
    x.a = 1
    x.b = 2
    var y struct { b, a int; }
    y.b = 2
    y.a = 1

    var z = x == y // Invalid comparison
}
