// Invalid expression: ? <- ((int, int) -> int)(int)

package main

func test(a, b int) int {
    return 1
}

func main() {
    var y = test(1) // Invalid function call
}
