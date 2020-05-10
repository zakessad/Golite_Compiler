/* Invalid: Rune literal with invalid Unicode code point. */

package pkg

var x rune = '\u00110000'
