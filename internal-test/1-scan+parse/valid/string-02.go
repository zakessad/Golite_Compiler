/* Valid: Raw string literals with uninterpreted invalid escape sequences (GoLite-specific). */

package pkg

var x0 string = `\x6F`
var x1 string = `\377`
var x2 string = `\uD0BF\uD180\uD0B8\uD0B2\uD0B5\uD182`
