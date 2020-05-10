/* This is an implementation of the Miller-Rabin Probabilistic Primality Test in GoLite. */

package main

func gcd(a, b int) int {
    if b == 0 {
        return a
    } else {
        return gcd(b, a % b)
    }
}

func powmod(b, e, n int) int {
    if n == 1 {
        return 0
    }

    var value int = 1

    b %= n
    for e > 0 {
        if e % 2 == 1 {
            value = (value * b) % n
        }

        e /= 2
        b *= b
        b %= n
    }

    return value
}

func is_pseudo_prime(a, n int) bool {
    if gcd(a, n) == 1 {
        var t, s int = (n - 1) / 2, 1
        for t % 2 == 0 {
            t /= 2
            s += 1
        }

        var x, y int = powmod(a, t, n), n - 1
        for i := 0; i <= s; i++ {
            if x == 1 && y == n - 1 {
                return true
            }

            y  = x
            x *= x
            x %= n
        }
    }

    return false
}

func is_prime(n, k int) bool {
    var a int = 2
    for i := 0; i < k && a < n; i++ {
        if ! is_pseudo_prime(a, n) {
            return false
        }
        a++
    }

    return true
}

func main() {
    var n, k int = 3301, 32
    if is_prime(n, k) {
        println(n, "is prime with probability 1 - (1 / 4) ^", k)
    } else {
        println(n, "is composite")
    }
}
