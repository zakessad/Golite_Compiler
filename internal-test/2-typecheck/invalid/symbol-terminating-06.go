// Terminating switch doesn't have a default case
package main

func f (a int) int{
    switch a {
        case 1: return 2;
        case 2: return 3;
    }
}
