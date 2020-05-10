/* Invalid: String literal with Unicode surrogate pair. */

package pkg

var x string = "\uDFFF"
