//~23 23
//~2 60
//~4 60
//~23 54 4
package pkg

var a []int
var b [60]int

func init() {
	a = append(append(a, 23), 54)
	b[0] = 23
}

func main() {
	println(a[0], b[0])
	println(len(a), len(b))
	a = append(a, 4)
	println(cap(a), cap(b))
	println(a[0], a[1], a[2])
}
