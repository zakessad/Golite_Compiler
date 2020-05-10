// Invalid expression: ? <- ([]int)[rune]

package main

func main() {
    var x []int
    x = append(x, 1)
    x = append(x, 2)
    x = append(x, 3)

    var y = x[rune(0x00)] // Invalid indexing expression
}
