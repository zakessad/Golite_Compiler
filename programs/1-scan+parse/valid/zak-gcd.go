// Computes the gcd of two number
package temp;


func gcd (int a, int b) int {
    if b == 0{
        return a;
    } else if a >b {
         return gcd(b, a%b);
    } else {
         return gcd (a, b%a);
    }

}


func main() {
    var a,b int = 3,568;

    println(gcd(a,b));
}
