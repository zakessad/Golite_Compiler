package main

// Uninitialized declaration
var x int

// Untyped initialized declaration
var y = "text"

// Typed, initialized declaration
var z rune = 'A'

// Array declaration
var a [10]int
var b [10]int = a

// Slide declaration
var c []int
var d []int = c

// Struct declaration
var u struct {
	x, y int
	name string
}
var v struct {
	x, y int
	name string
} = u

var r = v.name
var s string = r