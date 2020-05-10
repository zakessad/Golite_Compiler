/* Invalid: Attempt to start a block comment within a rune. */

package pkg

var x rune = '/*' this is not allowed */
