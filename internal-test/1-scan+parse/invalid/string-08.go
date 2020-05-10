/* Invalid: String literal with invalid Unicode code point. */

package pkg

var x string = "\u00110000"
