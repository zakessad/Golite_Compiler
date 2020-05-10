// Invalid expression: ? <- struct { a struct { b int; }; } != struct { a struct { c int; }; }

package main

func main() {
    var x struct { a struct { b int; }; }
    x.a.b = 1
    var y struct { a struct { c int; }; }
    y.a.c = 1

    var z = x != y // Invalid comparison
}
