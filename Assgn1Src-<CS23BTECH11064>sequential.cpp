#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
using namespace std;
using namespace std::chrono;

bool is_sudoku_valid = true;

bool check_row(const vector<vector<int>> &sudoku, int row) {
    int N = sudoku.size();
    vector<bool> visited(N, false);
    for (int j = 0; j < N; j++) {
        int index = sudoku[row][j] - 1;
        if (visited[index]) {
            return false;
        }
        visited[index] = true;
    }
    return true;
}

bool check_col(const vector<vector<int>> &sudoku, int col) {
    int N = sudoku.size();
    vector<bool> visited(N, false);
    for (int j = 0; j < N; j++) {
        int index = sudoku[j][col] - 1;
        if (visited[index]) {
            return false;
        }
        visited[index] = true;
    }
    return true;
}

bool check_subgrid(const vector<vector<int>> &sudoku, int start_row, int start_col) {
    int N = sudoku.size();
    int n = sqrt(N);
    vector<bool> visited(N, false);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int index = sudoku[start_row + i][start_col + j] - 1;
            if (visited[index]) {
                return false;
            }
            visited[index] = true;
        }
    }
    return true;
}

int main() {
    ifstream input_file("input.txt");
    ofstream output_file("output.txt");
    int K, N;
    input_file >> K >> N;
    vector<vector<int>> sudoku(N, vector<int>(N));

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            input_file >> sudoku[i][j];
        }
    }

    int n = sqrt(N);
    auto start_time = high_resolution_clock::now();

    for (int i = 0; i < N; i++) {
        if (!check_row(sudoku, i)) {
            is_sudoku_valid = false;
            output_file << "Row " << i << " is invalid." << endl;
            break;
        }
        output_file << "Row " << i << " is valid." << endl;
    }

    for (int i = 0; i < N; i++) {
        if (!check_col(sudoku, i)) {
            is_sudoku_valid = false;
            output_file << "Column " << i << " is invalid." << endl;
            break;
        }
        output_file << "Column " << i << " is valid." << endl;
    }

    for (int i = 0; i < N; i += n) {
        for (int j = 0; j < N; j += n) {
            if (!check_subgrid(sudoku, i, j)) {
                is_sudoku_valid = false;
                output_file << "Subgrid (" << i << "," << j << ") is invalid." << endl;
                break;
            }
            output_file << "Subgrid (" << i << "," << j << ") is valid." << endl;
        }
    }

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end_time - start_time);

    if (is_sudoku_valid) {
        output_file << "Sudoku is valid." << endl;
    } else {
        output_file << "Sudoku is invalid." << endl;
    }

    output_file << "Time taken: " << duration.count() << " microseconds." << endl;

    return 0;
}
