// A program to solve Sudoku.

//~=====================================
//~|   |   | 5 |   |   |   | 2 |   |   |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~| 7 |   |   |   |   | 4 |   |   | 3 |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~| 2 | 4 |   | 6 |   | 7 |   |   |   |
//~=====================================
//~| 5 | 8 |   |   |   |   |   | 1 |   |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~|   |   | 7 |   |   |   |   |   |   |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~|   |   |   |   | 3 |   |   |   | 4 |
//~=====================================
//~|   |   | 9 |   | 4 |   |   | 8 |   |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~| 1 |   |   |   | 2 |   |   |   |   |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~|   |   |   |   |   | 1 | 5 | 2 |   |
//~=====================================
//~
//~=====================================
//~| 8 | 6 | 5 | 9 | 1 | 3 | 2 | 4 | 7 |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~| 7 | 9 | 1 | 2 | 5 | 4 | 8 | 6 | 3 |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~| 2 | 4 | 3 | 6 | 8 | 7 | 1 | 9 | 5 |
//~=====================================
//~| 5 | 8 | 6 | 4 | 7 | 9 | 3 | 1 | 2 |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~| 4 | 3 | 7 | 1 | 6 | 2 | 9 | 5 | 8 |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~| 9 | 1 | 2 | 5 | 3 | 8 | 6 | 7 | 4 |
//~=====================================
//~| 6 | 2 | 9 | 3 | 4 | 5 | 7 | 8 | 1 |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~| 1 | 5 | 8 | 7 | 2 | 6 | 4 | 3 | 9 |
//~|–––+–––+–––|–––+–––+–––|–––+–––+–––|
//~| 3 | 7 | 4 | 8 | 9 | 1 | 5 | 2 | 6 |
//~=====================================


package main


var EPSILON float64 = 0.00001


func _sqrt(x float64) float64 {
    var s float64 = 0.0
    if x > 0.0 {
        for s = x; s - x / s > EPSILON; s = (s + x / s) / 2.0 {
            // nothing
        }
    }
    return s
}

func _int_sqrt(x int) int {
    var s int = int(_sqrt(float64(x)))

    if s * s == x {
        return s
    } else if (s - 1) * (s - 1) == x {
        return s - 1
    } else if (s + 1) * (s + 1) == x {
        return s + 1
    } else {
        return 0
    }
}


type SudokuNode struct {
    value int
    sqr_x int
    sqr_y int
}

type Sudoku struct {
    board [][]SudokuNode
    n int
    sqr_n int
}


func build_sudoku(board [][]int, n int) Sudoku {
    var sqr_n int = _int_sqrt(n)

    var sudoku Sudoku
    sudoku.n = n
    sudoku.sqr_n = sqr_n

    var line []SudokuNode
    var temp []SudokuNode
    var node   SudokuNode

    for row := 0; row < sudoku.n; row++ {
        line = temp

        for col := 0; col < sudoku.n; col++ {
            node.sqr_x = sudoku.sqr_n * (row / sudoku.sqr_n)
            node.sqr_y = sudoku.sqr_n * (col / sudoku.sqr_n)
            node.value = board[row][col]

            line = append(line, node)
        }

        sudoku.board = append(sudoku.board, line)
    }

    return sudoku
}

func print_sudoku(sudoku Sudoku) {
    if sudoku.sqr_n < 1 {
        return
    }

    var width int = 1

    var sep_1 string = "==="
    var sep_2 string = "–––"
    var sep_3 string = "   "

    if sudoku.n >=   10 {
        sep_1 += "="
        sep_2 += "–"
        sep_3 += " "
        width += 1
    }
    if sudoku.n >=  100 {
        sep_1 += "="
        sep_2 += "–"
        sep_3 += " "
        width += 1
    }
    if sudoku.n >= 1000 {
        sep_1 += "="
        sep_2 += "–"
        sep_3 += " "
        width += 1
    }

    print("=")
    for y := 0; y < sudoku.n; y++ {
        print(sep_1, "=")
    }

    println()

    for x := 0; x < sudoku.n; x++ {
        print("|")
        for y := 0; y < sudoku.n; y++ {
            if sudoku.board[x][y].value > 0 {
                var padsize int

                switch {
                    case sudoku.board[x][y].value <   10:
                        padsize = width - 1
                    case sudoku.board[x][y].value <  100:
                        padsize = width - 2
                    case sudoku.board[x][y].value < 1000:
                        padsize = width - 3
                    default:
                        padsize = width - 4
                }

                switch padsize {
                    case 1:
                        print(" ")
                    case 2:
                        print(" ")
                        print(" ")
                    case 3:
                        print(" ")
                        print(" ")
                        print(" ")
                    case 4:
                        print(" ")
                        print(" ")
                        print(" ")
                }

                print(" ", sudoku.board[x][y].value, " ", "|")
            } else {
                print(sep_3, "|")
            }
        }

        println()

        if (x + 1) % sudoku.sqr_n == 0 {
            print("=")
            for y := 0; y < sudoku.n; y++ {
                if (y + 1) % sudoku.sqr_n == 0 {
                    print(sep_1, "=")
                } else {
                    print(sep_1, "=")
                }
            }
        } else {
            print("|")
            for y := 0; y < sudoku.n; y++ {
                if (y + 1) % sudoku.sqr_n == 0 {
                    print(sep_2, "|")
                } else {
                    print(sep_2, "+")
                }
            }
        }

        println()
    }

    return
}


func _get_safe_moves(sudoku Sudoku, x, y int) []int {
    var row_values []int
    var col_values []int
    var sqr_values []int

    var sqr_x int = sudoku.board[x][y].sqr_x
    var sqr_y int = sudoku.board[x][y].sqr_y

    for row := 0; row < sudoku.n; row++ {
        row_values = append(row_values, sudoku.board[row][y].value)
    }
    for col := 0; col < sudoku.n; col++ {
        col_values = append(col_values, sudoku.board[x][col].value)
    }
    for sqr := 0; sqr < sudoku.n; sqr++ {
        sqr_values = append(sqr_values, sudoku.board[sqr_x + (sqr / sudoku.sqr_n)][sqr_y + (sqr % sudoku.sqr_n)].value)
    }

    var moves []int

    for val := 0; val < sudoku.n; val++ {
        var safe bool = true

        for i := 0; i < len(row_values); i++ {
            if val + 1 == row_values[i] {
                safe = false
            }
        }
        if ! safe {
            continue
        }
        for i := 0; i < len(col_values); i++ {
            if val + 1 == col_values[i] {
                safe = false
            }
        }
        if ! safe {
            continue
        }
        for i := 0; i < len(sqr_values); i++ {
            if val + 1 == sqr_values[i] {
                safe = false
            }
        }
        if ! safe {
            continue
        }

        moves = append(moves, val + 1)
    }

    return moves
}

func _solve_recurs(sudoku Sudoku, x, y int) bool {
    if x == sudoku.n {
        return true
    }

    if sudoku.board[x][y].value != 0 {
        return _solve_recurs(sudoku, x + (y + 1) / sudoku.n, (y + 1) % sudoku.n)
    }

    var moves []int

    moves = _get_safe_moves(sudoku, x, y)
    if len(moves) > 0 {
        for i := 0; i < len(moves); i++ {
            sudoku.board[x][y].value = moves[i]
            if _solve_recurs(sudoku, x + (y + 1) / sudoku.n, (y + 1) % sudoku.n) {
                return true
            }
        }
        sudoku.board[x][y].value = 0
    }

    return false
}


func solve(sudoku Sudoku) bool {
    return _solve_recurs(sudoku, 0, 0)
}


func main() {
    var n int = 9

    var board [][]int
    var temp []int
    var line []int

    line = temp
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 5)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 2)
    line = append(line, 0)
    line = append(line, 0)

    board = append(board, line)

    line = temp
    line = append(line, 7)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 4)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 3)

    board = append(board, line)

    line = temp
    line = append(line, 2)
    line = append(line, 4)
    line = append(line, 0)
    line = append(line, 6)
    line = append(line, 0)
    line = append(line, 7)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)

    board = append(board, line)

    line = temp
    line = append(line, 5)
    line = append(line, 8)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 1)
    line = append(line, 0)

    board = append(board, line)

    line = temp
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 7)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)

    board = append(board, line)

    line = temp
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 3)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 4)

    board = append(board, line)

    line = temp
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 9)
    line = append(line, 0)
    line = append(line, 4)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 8)
    line = append(line, 0)

    board = append(board, line)

    line = temp
    line = append(line, 1)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 2)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)

    board = append(board, line)

    line = temp
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 0)
    line = append(line, 1)
    line = append(line, 5)
    line = append(line, 2)
    line = append(line, 0)

    board = append(board, line)

    var sudoku Sudoku = build_sudoku(board, n)
    if sudoku.sqr_n == 0 {
        println("Error: The Sudoku board must be of size (N * N) where N is an integer")

        return
    }

    print_sudoku(sudoku)

    println()

    if ! solve(sudoku) {
        println("Error: There is no solution to the given Sudoku")

        return
    }

    print_sudoku(sudoku)

    return
}
