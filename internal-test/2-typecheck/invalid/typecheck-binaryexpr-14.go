// Invalid expression: ? <- struct { a int; } & struct { a int; }

package main

func main() {
    var x struct { a int; }
    x.a = 1
    var y struct { a int; }
    y.a = 1

    var z = x & y // Invalid operation
}
