// Terminating block stmt doesn't terminate
package main

func f (a int) int {
    return a
    {
        a++;
    }
}
