/* This is an implementation of the Substitution Cipher in GoLite. */

package main

func substitute(ptext string) string {
    var ctext string = ""

    for i := 0; i < len(ptext); i++ {
        switch ptext[i] {
        case 'A', 'a':
            ctext += "H"
        case 'B', 'b':
            ctext += "Z"
        case 'C', 'c':
            ctext += "O"
        case 'D', 'd':
            ctext += "M"
        case 'E', 'e':
            ctext += "F"
        case 'F', 'f':
            ctext += "R"
        case 'G', 'g':
            ctext += "T"
        case 'H', 'h':
            ctext += "Y"
        case 'I', 'i':
            ctext += "U"
        case 'J', 'j':
            ctext += "A"
        case 'K', 'k':
            ctext += "L"
        case 'L', 'l':
            ctext += "B"
        case 'M', 'm':
            ctext += "V"
        case 'N', 'n':
            ctext += "J"
        case 'O', 'o':
            ctext += "X"
        case 'P', 'p':
            ctext += "S"
        case 'Q', 'q':
            ctext += "K"
        case 'R', 'r':
            ctext += "W"
        case 'S', 's':
            ctext += "G"
        case 'T', 't':
            ctext += "C"
        case 'U', 'u':
            ctext += "P"
        case 'V', 'v':
            ctext += "E"
        case 'W', 'w':
            ctext += "Q"
        case 'X', 'x':
            ctext += "I"
        case 'Y', 'y':
            ctext += "N"
        case 'Z', 'z':
            ctext += "D"

        default:
            break
        }
    }

    return ctext
}

func main() {
    var ptext string = "Attack at dawn!"

    println("substitute(", ptext, ") =", substitute(ptext))
}
