// Invalid expression: ? <- (() -> int)(int)

package main

func test() int {
    return 1
}

func main() {
    var x = test(1)
}
