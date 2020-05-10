// Invalid expression: _ <- (() -> void)()

package main

func test() {
}

func main() {
    var _ = test()
}
