// Valid expressions

package main


func call0() {
}

func call1(a int) {
}

func call2(a, b int) {
}

func call3(a, b, c int) {
}


func main() {
    type (
        A int
        B A
        C []A
        D B
        E C
        F [2]B
        G [2]C
        H struct { a, b int; }
        I struct { b, a int; }
        J struct { a struct { b int; }; }
    )

    var (
        x0 A = A(1)
        x1 B = B(A(2))
        x2 C
        x3 D = D(B(A(3)))
        x4 E
        x5 F
        x6 G
        x7 H
        x8 I
        x9 J
    )

    // Builtin append, cap, len
    x2 = append(x2, A(cap(x2)))
    x2 = append(x2, A(len(x2)))
    x4 = append(x4, A(cap(x4)))
    x4 = append(x4, A(len(x4)))

    x5[0] = x1
    x5[1] = x1
    x6[0] = x2
    x6[1] = x2

    x7.a = 1
    x7.b = 2
    x8.b = 2
    x8.a = 1

    x9.a.b = 1

    // Unary expressions
    var _, _, _ = +1, +2.0, +'A'
    var _, _, _ = -1, -2.0, -'A'
    var _, _ = !true, !false
    var _, _ = ^0, ^'A'

    // Binary expressions
    var _, _, _ = true || true, false || false, true || false
    var _, _, _ = true && true, false && false, true && false
    var _, _, _, _, _, _ = 1 == 1, 1 != 1, 1 < 1, 1 <= 1, 1 > 1, 1 >= 1
    var _, _, _, _, _, _ = 1.0 == 1.0, 1.0 != 1.0, 1.0 < 1.0, 1.0 <= 1.0, 1.0 > 1.0, 1.0 >= 1.0
    var _, _, _, _, _, _ = "Hello" == "world", "Hello" != "world", "Hello" < "world", "Hello" <= "world", "Hello" > "world", "Hello" >= "world"
    var _, _, _, _, _, _ = 'A' == 'Z', 'A' != 'Z', 'A' < 'Z', 'A' <= 'Z', 'A' > 'Z', 'A' >= 'Z'
    var _, _, _, _ = 1 + 1, 2.0 + 2.0, 'A' + 'Z', "Hello" + "world"
    var _, _ = x0 == x0, x0 != x0
    var _, _ = x1 == x1, x1 != x1
    //var _, _ = x2 == x2, x2 != x2
    var _, _ = x3 == x3, x3 != x3
    //var _, _ = x4 == x4, x4 != x4
    var _, _ = x5 == x5, x5 != x5
    //var _, _ = x6 == x6, x6 != x6
    var _, _ = x7 == x7, x7 != x7
    var _, _ = x8 == x8, x8 != x8
    var _, _ = x9 == x9, x9 != x9
    var _, _, _ = 1 - 1, 2.0 - 2.0, 'A' - 'Z'
    var _, _, _ = 1 * 1, 2.0 * 2.0, 'A' * 'Z'
    var _, _, _ = 1 / 1, 2.0 / 2.0, 'A' / 'Z'
    var _, _ = 1 % 1, 'A' % 'Z'
    var _, _ = 1 | 1, 'A' | 'Z'
    var _, _ = 1 & 1, 'A' & 'Z'
    var _, _ = 1 << 1, 'A' << 'Z'
    var _, _ = 1 >> 1, 'A' >> 'Z'
    var _, _ = 1 &^ 1, 'A' &^ 'Z'
    var _, _ = 1 ^ 1, 'A' ^ 'Z'

    // Function call expressions
    call0()
    call1(1)
    call2(1, 2)
    call3(1, 2, 3)

    // Indexing expressions
    var _, _, _, _, _, _ = x2[0], x5[0], x6[0], x2[x0], x5[x0], x6[x0]

    // Field selection expressions
    var _, _, _, _, _ = x7.a, x7.b, x8.b, x8.a, x9.a.b

    // Typecast expressions
    var _, _, _, _, _, _ = int('A'), int(2.0), int(1), int(x0), int(x1), int(x3)
    var _, _, _, _, _, _ = float64('A'), float64(2.0), float64(1), float64(x0), float64(x1), float64(x3)
    var _, _, _, _, _ = rune('A'), rune(1), rune(x0), rune(x1), rune(x3)
    var _, _, _, _, _ = string('A'), string(1), string(x0), string(x1), string(x3)
    var _, _, _, _, _, _ = A('A'), A(2.0), A(1), A(x0), A(x1), A(x3)
    var _, _, _, _, _, _ = B('A'), B(2.0), B(1), B(x0), B(x1), B(x3)
    var _, _, _, _, _, _ = D('A'), D(2.0), D(1), D(x0), D(x1), D(x3)
    var _ = D(B(A(1)))
}
