//~1 2 3 4
//~4 3 2 1


package main


func reverse(x []int) {
    for i, j := 0, len(x) - 1; i < j; i, j = i + 1, j - 1 {
        x[i] ^= x[j]
        x[j] ^= x[i]
        x[i] ^= x[j]
    }

    return
}


func main() {
    var x []int

    x = append(x, 1)
    x = append(x, 2)
    x = append(x, 3)
    x = append(x, 4)

    println(x[0], x[1], x[2], x[3])

    reverse(x)

    println(x[0], x[1], x[2], x[3])

    return
}
