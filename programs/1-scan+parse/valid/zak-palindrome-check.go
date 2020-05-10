// Small program to check if a numbre is a palindrome
package tmp;

func pal (num int) int {

    reversed :=0;
    remainder :=0;
    original := num;
    for {
        if(num ==0) {
            break;
        }
        remainder = num %10;
        reversed = reversed * 10 + remainder;
        num /=10;
    }
    if original == reversed {
        return 1;
    } else {
        return 0;
    } 


}

func main() {
    var num int = 24899842;
    
    if pal(num) == 1{
        println("Palindrome");
    }else {
        println("Not Palindrome");
    }
}



