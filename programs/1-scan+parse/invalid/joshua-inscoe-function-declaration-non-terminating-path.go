/* Invalid: Function declaration with result parameter but a non-terminating path. */
package temp;

func foo(x int, y int) int {
    if x < y
    {
        y = x
    }
    else
    {
        return y
    }
}
