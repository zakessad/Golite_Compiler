// Terminating switch stmt includes a break
package main

func f(a int) int {
    switch a {
        case 1: return 1;
        case 2: return 2; break;
        default: return a;
    }
}
