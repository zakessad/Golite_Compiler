//   Prints the prime numbers up to n.

package main

func prime_generator(n int) {
	for i := 0; i < 100; i++ {
		prime := true
		for j := 0; j < i; j++ {
			if i % j == 0 {
				prime = false
				break
			}
		}
		if prime {
			println(i)
		}
	}
}
