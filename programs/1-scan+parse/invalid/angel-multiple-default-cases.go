package test

func main() {
	switch(true) {
	default:
		println("First default")
	default:
		println("This will not pass")
	}
}