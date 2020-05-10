/* Invalid: Rune literal with Unicode surrogate pair. */

package pkg

var x rune = '\uDFFF'
