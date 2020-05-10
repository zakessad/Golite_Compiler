// One of the cases in the switch is not terminating
package main

func f(a int) int{
    switch a {
        case 1: return 1;
        case 2: a++;
        default: return 0;
    }
}
