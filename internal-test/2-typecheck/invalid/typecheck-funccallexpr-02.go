// Invalid expression: ? <- ((float64) -> int)(int)

package main

func test(a float64) int {
    return 1
}

func main() {
    var x = test(1)
}
