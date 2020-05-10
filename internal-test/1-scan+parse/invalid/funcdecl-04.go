/* Invalid: Function declaration which returns multiple results (GoLite-specific). */

package pkg;

func foo(x int, y int) (int, int) {
    return x, y
}
