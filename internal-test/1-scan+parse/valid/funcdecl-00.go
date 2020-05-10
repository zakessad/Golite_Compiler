/* Valid: Valid function declarations. */

package pkg;

func foo0() {
    return
}

func foo1() {
    var x int = 1
}

func foo2(x int) int {
    return x + 2
}

func foo3(x, y int) int {
    if x < y{
        return x
    }else{
        return y
    }
}

func foo4() int {
    for {
    }
}

func foo5(x int) int {
    switch x {
    case 0, 2, 4:
        return 0;
    case 1, 3, 5:
        return 1;
    default:
        return x
    }
}
