// Invalid expression: ? <- ((int) -> int)([]int)

package main

func test(a int) int {
    return 1
}

func main() {
    var x []int
    x = append(x, 1)
    x = append(x, 2)
    x = append(x, 3)

    var y = test(x) // Invalid function call
}
