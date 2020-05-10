//~1 2 3 4
//~1 2 3 4


package main


func reverse(x [4]int) {
    for i, j := 0, len(x) - 1; i < j; i, j = i + 1, j - 1 {
        x[i] ^= x[j]
        x[j] ^= x[i]
        x[i] ^= x[j]
    }

    return
}


func main() {
    var x [4]int

    x[0] = 1
    x[1] = 2
    x[2] = 3
    x[3] = 4

    println(x[0], x[1], x[2], x[3])

    reverse(x)

    println(x[0], x[1], x[2], x[3])

    return
}
