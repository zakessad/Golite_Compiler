// Invalid expression: ? <- (([]int) -> int)([3]int)

package main

func test(a []int) int {
    return 1
}

func main() {
    var x [3]int
    x[0] = 1
    x[1] = 2
    x[2] = 3

    var y = test(x) // Invalid function call
}
