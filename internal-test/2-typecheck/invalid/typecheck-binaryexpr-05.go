// Invalid expression: ? <- [3]struct { a [2]struct { b []int; }; } <= [3]struct { a [2]struct { b []int; }; }

package main

func main() {
    var x [3]struct { a [2]struct { b []int; }; }
    x[0].a[0].b = append(x[0].a[0].b, 1)
    x[0].a[1].b = append(x[0].a[1].b, 2)
    x[1].a[0].b = append(x[1].a[0].b, 3)
    x[1].a[1].b = append(x[1].a[1].b, 4)
    x[2].a[0].b = append(x[2].a[0].b, 5)
    x[2].a[1].b = append(x[2].a[1].b, 6)
    var y [3]struct { a [2]struct { b []int; }; }
    y[0].a[0].b = append(y[0].a[0].b, 1)
    y[0].a[1].b = append(y[0].a[1].b, 2)
    y[1].a[0].b = append(y[1].a[0].b, 3)
    y[1].a[1].b = append(y[1].a[1].b, 4)
    y[2].a[0].b = append(y[2].a[0].b, 5)
    y[2].a[1].b = append(y[2].a[1].b, 6)

    var z = x <= y // Invalid comparison
}
