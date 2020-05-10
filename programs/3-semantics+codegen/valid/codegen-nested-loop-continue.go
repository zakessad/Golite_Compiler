//~0
//~1
//~1
//~2
//~3
//~3
//~Bye!


package main


func main() {
    for i := 0; i < 4; i++ {
        var skip bool = true

        for j := 0; j < 4; j++ {
            if i % 2 == 0 {
                skip = false
            }
        }

        println(i)

        if ! skip {
            continue
        }

        println(i)
    }

    println("Bye!")

    return
}
