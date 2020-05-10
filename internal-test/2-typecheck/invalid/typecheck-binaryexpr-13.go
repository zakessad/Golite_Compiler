// Invalid expression: ? <- [5]rune | [5]rune

package main

func main() {
    var x [5]rune
    x = append(x, 'H')
    x = append(x, 'e')
    x = append(x, 'l')
    x = append(x, 'l')
    x = append(x, 'o')
    var y [5]rune
    y = append(y, 'W')
    y = append(y, 'o')
    y = append(y, 'r')
    y = append(y, 'l')
    y = append(y, 'd')

    var z = x | y // Invalid operation
}
