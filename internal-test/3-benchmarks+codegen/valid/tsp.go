//~____________________________________________________________
//~|  0  |17418|15963|16258|10803|17098|17643|19938|16483|14778|
//~|17418|  0  |17323|11618|10163|10458|15003|11298|11843|14138|
//~|15963|17323|  0  |10683|18978|11523|15818|14363|14658|19203|
//~|16258|11618|10683|  0  |15498|16043|18338|14883|13178|15723|
//~|10803|10163|18978|15498|  0  |10018|18563|18858|13403|19698|
//~|17098|10458|11523|16043|10018|  0  |10243|12538|19083|17378|
//~|17643|15003|15818|18338|18563|10243|  0  |19923|14218|12763|
//~|19938|11298|14363|14883|18858|12538|19923|  0  |13058|17603|
//~|16483|11843|14658|13178|13403|19083|14218|13058|  0  |13898|
//~|14778|14138|19203|15723|19698|17378|12763|17603|13898|  0  |
//~____________________________________________________________
//~118490

package main
/*
Implements TSP by following https://www.geeksforgeeks.org/traveling-salesman-problem-tsp-implementation/
and https://www.geeksforgeeks.org/pseudo-random-number-generator-prng/
****
With V=12, the program runs in about 3.5seconds on my machine and the geerated python code should execute in about 6min.
With V=13, the programs takes 33seconds on my machine and the python code will take infinite time.
*/

var V = 13
var MAX_INT = 2147483647
var graph[][]int

var rand_num, rand_a, rand_c, rand_m = 123, 151, 15845, 10000

/*Get random number between 10000 and 20000 (just to make printing nice)*/
func next_rand() int {
	rand_num = (rand_num*rand_a + rand_c) % rand_m
	return rand_num+10000
}

// Runs random for a while, just to use inits
func init() {
	var upper_bound = 150
	for i := 0; i < upper_bound; i++ {
		_ = next_rand()
	}
}

func min (a,b int) int{
	if a< b {
		return a
	} else {
		return b
	}
}

func next_permutation(vertex []int) bool{
	n := len(vertex)

	i:= n-2
	
	for i>=0 && vertex[i] >= vertex[i+1] {
		i--
	}

	if i== -1{
		return false
	}

	j := i+1 

	for j < n && vertex[j]>vertex[i] {
		j++
	}
	j--

	vertex[i], vertex[j] = vertex[j], vertex[i]

    left, right:= i + 1, n - 1
  
    for left < right{
        vertex[left], vertex[right] = vertex[right], vertex[left] 
        left += 1
        right -= 1
	}
    return true
}

func tsp(graph [][]int, source int) int {
	var vertex []int

	for i := 0; i < V; i++ {
		if i != source {
			vertex = append(vertex, i)
		}
	}

	min_path := MAX_INT

	for {
		current_pathweight := 0
	
		k := source  
		for i:=0; i<len(vertex); i++ {
			current_pathweight += graph[k][vertex[i]] 
			k = vertex[i] 
		}

		current_pathweight += graph[k][source] 
	
		min_path = min(min_path, current_pathweight) 
	
		if !next_permutation(vertex){
			break
		} 
	}
	return min_path
}

// init the graph
func init(){	
	for i:=0; i<V; i++{
		var row []int
		for j:=0; j<V; j++ {
			if(i == j) {
				row = append(row, 0)
			} else if (j < i) { // Copy mirroring element
				row = append(row, graph[j][i])
			} else {
				row = append(row, next_rand())
			}
		}
		graph = append(graph, row)
	}
}

func print_graph() {
	for i:=0; i<V; i++{
		print("______")
	}
	println()
	for i:=0; i<V; i++{
		print("|")
		for j:=0; j<V; j++ {
			if(i == j){
				print("  ",0,"  ", "|")
			} else {
				print(graph[i][j],"|")
			}
			
		}
		println()
	}
	for i:=0; i<V; i++{
		print("______")
	}
	println()
}

func main() {
	print_graph()
	println(tsp(graph, 0))
}
