// Else block does not terminate 
package main

func f (a int) int{
    if (true) {
        return a
    } else {
        a++
    }
}
