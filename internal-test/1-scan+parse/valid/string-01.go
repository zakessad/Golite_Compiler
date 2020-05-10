/* Valid: Raw string literals with uninterpreted invalid escape sequences. */

package pkg

var x0 string = `\'`
var x1 string = `\x6`
var x2 string = `\0`
var x3 string = `\uDFFF`
var x4 string = `\u00110000`
