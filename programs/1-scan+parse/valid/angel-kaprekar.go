//   Takes a number n and determines if it is a prime.

package main

func find_kaprekar(n int) bool {
	// Find the square root of n.
	var lo = 0
	var hi = n
	var mi = 0
	var i = 0
	var sqrt_n = 0
	for i < 1000 {
		mi = (hi + lo) / 2
		sqrt_n = mid
		if (mid * mid == n) {
			break
		} else {
			i = i + 1 // keep looking
		}
	}

	// Check if any number from 1 to sqrt_n divides n.
	var is_prime = True;
	i = 1;

	for i <= sqrt_n  {
		var remainder = n
		for remainer >= i {
			remainder = remainder - i
		}
		if remainder == 0 {
			is_prime = False // i divides n so it is not prime
			i = sqrt_n + 1
		}
	}

	if is_prime {
		println("The number is prime!")
		return true
	} else {
		println("The number is not prime!")
		return false
	}
}
