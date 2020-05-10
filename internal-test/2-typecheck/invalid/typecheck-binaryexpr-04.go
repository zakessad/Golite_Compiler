// Invalid expression: ? <- [3]struct { a [2]struct { b int; }; } < [3]struct { a [2]struct { c int; }; }

package main

func main() {
    var x [3]struct { a [2]struct { b int; }; }
    x[0].a[0].b = 1
    x[0].a[1].b = 2
    x[1].a[0].b = 3
    x[1].a[1].b = 4
    x[2].a[0].b = 5
    x[2].a[1].b = 6
    var y [3]struct { a [2]struct { c int; }; }
    y[0].a[0].c = 1
    y[0].a[1].c = 2
    y[1].a[0].c = 3
    y[1].a[1].c = 4
    y[2].a[0].c = 5
    y[2].a[1].c = 6

    var z = x < y // Invalid comparison
}
