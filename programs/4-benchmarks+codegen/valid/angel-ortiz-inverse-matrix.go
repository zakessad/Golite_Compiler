//~Input Matrix:
//~+8.000000e+000, +3.000000e+000, +4.000000e+000, +1.000000e+000, +2.000000e+000, +4.000000e+000, +0.000000e+000, +1.000000e+000, +6.000000e+000
//~+8.000000e+000, +7.000000e+000, +5.000000e+000, +0.000000e+000, +1.000000e+000, +2.000000e+000, +2.000000e+000, +8.000000e+000, +0.000000e+000
//~+4.000000e+000, +4.000000e+000, +5.000000e+000, +3.000000e+000, +6.000000e+000, +1.000000e+000, +8.000000e+000, +4.000000e+000, +0.000000e+000
//~+1.000000e+000, +9.000000e+000, +5.000000e+000, +8.000000e+000, +3.000000e+000, +1.000000e+000, +4.000000e+000, +4.000000e+000, +4.000000e+000
//~+6.000000e+000, +9.000000e+000, +4.000000e+000, +6.000000e+000, +9.000000e+000, +8.000000e+000, +1.000000e+000, +6.000000e+000, +8.000000e+000
//~+9.000000e+000, +5.000000e+000, +2.000000e+000, +8.000000e+000, +5.000000e+000, +4.000000e+000, +4.000000e+000, +2.000000e+000, +2.000000e+000
//~+8.000000e+000, +0.000000e+000, +5.000000e+000, +2.000000e+000, +5.000000e+000, +9.000000e+000, +7.000000e+000, +5.000000e+000, +7.000000e+000
//~+0.000000e+000, +7.000000e+000, +0.000000e+000, +4.000000e+000, +9.000000e+000, +5.000000e+000, +6.000000e+000, +7.000000e+000, +8.000000e+000
//~+2.000000e+000, +1.000000e+000, +8.000000e+000, +8.000000e+000, +5.000000e+000, +3.000000e+000, +9.000000e+000, +8.000000e+000, +7.000000e+000
//~
//~Inverse Matrix:
//~+9.458985e-002, +3.584019e-002, -4.837677e-003, -7.461612e-002, -5.500185e-002, +8.503662e-002, -5.291890e-002, +2.954270e-002, +1.927967e-002
//~+6.876629e-003, +1.406920e-002, +1.751078e-002, +1.274557e-001, -2.804038e-002, -3.184496e-002, +3.725775e-002, +2.563851e-002, -1.041403e-001
//~+1.036638e-002, -3.097918e-002, +9.399511e-002, +4.268274e-002, +1.019045e-001, -1.141131e-001, +1.469144e-002, -1.347278e-001, +2.214898e-002
//~-6.446101e-002, -1.655608e-002, -7.176892e-002, +1.150742e-002, +2.299714e-002, +9.518805e-002, -2.383142e-002, -3.571008e-002, +5.984055e-002
//~+2.876538e-002, -5.141157e-002, +1.302329e-001, -1.454500e-001, +1.195074e-001, -8.314353e-003, -1.149182e-001, -1.957277e-003, +4.140893e-002
//~-1.847904e-001, -1.521213e-002, -4.637572e-002, +1.099688e-001, +8.529033e-002, -3.803757e-002, +2.145345e-001, -8.796033e-002, -1.050627e-001
//~+8.313092e-004, -1.387926e-002, +3.646892e-002, +8.500254e-002, -1.381462e-001, +8.767131e-003, +9.259810e-002, +6.312793e-002, -5.865329e-002
//~-6.469180e-002, +1.099105e-001, -8.303362e-002, -9.414593e-002, +1.412239e-002, +2.250529e-002, -5.128215e-002, +3.868124e-002, +9.375285e-002
//~+1.653282e-001, -2.244920e-002, -5.166197e-002, -3.750957e-003, -8.346297e-002, -1.287003e-002, -5.006262e-002, +9.640646e-002, +4.223785e-002

package main

func display(m [][]float64) {
	rows, cols := len(m), len(m[0])

	for row := 0; row < rows; row++ {
		for col := 0; col < cols; col++ {
			print(m[row][col])
			if col != cols - 1 {
				print(", ")
			}
		}
		println()
	}
}

func transpose(m [][]float64) [][]float64 {
	rows, cols := len(m), len(m[0])
	var tmp float64

	for row := 0; row < rows; row++ {
		for col := row; col < cols; col++ {
			tmp = m[row][col]
			m[row][col] = m[col][row]
			m[col][row] = tmp
		}
	}

	return m
}

func submatrix(m [][]float64, exrow, excol int) [][]float64 {
	rows, cols := len(m), len(m[0])
	var sub [][]float64

	for row := 0; row < rows; row++ {
		if row == exrow {
			continue
		}

		var subrow []float64
		for col := 0; col < cols; col++ {
			if col == excol {
				continue
			}

			subrow = append(subrow, m[row][col])
		}
		sub = append(sub, subrow)
	}

	return sub
}

func det(m [][]float64) float64 {
	rows, cols := len(m), len(m[0])

	var determinant float64
	sign := 1.0

	if rows == 2 {
		determinant = (m[0][0] * m[1][1]) - (m[0][1] * m[1][0])
	} else {
		for row, col := 0, 0; col < cols; col++ {
			determinant += det(submatrix(m, row, col)) * m[row][col] * sign
			sign *= -1.0
		}
	}

	return determinant
}

func cofactors(m [][]float64) [][]float64 {
	rows, cols := len(m), len(m[0])
	var cfmatrix [][]float64
	sign := 1.0

	for row := 0; row < rows; row++ {
		var cfrow []float64
		for col := 0; col < cols; col++ {
			cfrow = append(cfrow, sign * det(submatrix(m, row, col)))
			sign *= -1.0
		}
		cfmatrix = append(cfmatrix, cfrow)
	}

	return cfmatrix
}

func div(m [][]float64, d float64) [][]float64{
	rows, cols := len(m), len(m[0])

	for row := 0; row < rows; row++ {
		for col := 0; col < cols; col++ {
			m[row][col] = m[row][col] / d
		}
	}

	return m
}

func inverse(m [][]float64) [][]float64 {
	var determinant = det(m)

	if (determinant == 0.0) {
		println("The input matrix is non-invertible.")
		return m
	}

	var cofactor_matrix = cofactors(m)
	var adjoint_matrix = transpose(cofactor_matrix)
	var inverse_matrix = div(adjoint_matrix, determinant)

	return inverse_matrix
}

func get_example_row(v1, v2, v3, v4, v5, v6, v7, v8, v9 int) []float64 {
	var row []float64

	row = append(row, float64(v1))
	row = append(row, float64(v2))
	row = append(row, float64(v3))
	row = append(row, float64(v4))
	row = append(row, float64(v5))
	row = append(row, float64(v6))
	row = append(row, float64(v7))
	row = append(row, float64(v8))
	row = append(row, float64(v9))

	return row
}

func get_example_matrix() [][]float64 {
	var m [][]float64

	m = append(m, get_example_row(8, 3, 4, 1, 2, 4, 0, 1, 6))
	m = append(m, get_example_row(8, 7, 5, 0, 1, 2, 2, 8, 0))
	m = append(m, get_example_row(4, 4, 5, 3, 6, 1, 8, 4, 0))
	m = append(m, get_example_row(1, 9, 5, 8, 3, 1, 4, 4, 4))
	m = append(m, get_example_row(6, 9, 4, 6, 9, 8, 1, 6, 8))
	m = append(m, get_example_row(9, 5, 2, 8, 5, 4, 4, 2, 2))
	m = append(m, get_example_row(8, 0, 5, 2, 5, 9, 7, 5, 7))
	m = append(m, get_example_row(0, 7, 0, 4, 9, 5, 6, 7, 8))
	m = append(m, get_example_row(2, 1, 8, 8, 5, 3, 9, 8, 7))

	return m
}

func main() {
	var m = get_example_matrix()

	println("Input Matrix:")
	display(m)

	println()

	println("Inverse Matrix:")
	display(inverse(m))
}