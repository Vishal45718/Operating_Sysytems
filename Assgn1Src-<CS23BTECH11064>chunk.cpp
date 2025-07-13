#include <iostream>
#include <pthread.h>
#include <chrono>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;
using namespace std::chrono;

struct thread_data
{
    int start_row, end_row, start_col, end_col, id;
    const vector<vector<int>> &sudoku;
    bool validity;
    vector<string> local_output;

    thread_data(int start_row, int end_row, int start_col, int end_col, int id, const vector<vector<int>> &sudoku)
        : start_row(start_row), end_row(end_row), start_col(start_col), end_col(end_col), id(id), sudoku(sudoku), validity(true) {}
};

void *row_check(void *arg)
{
    thread_data *data = (thread_data *)arg;
    const vector<vector<int>> &sudoku = data->sudoku;
    int n = sudoku.size();

    for (int i = data->start_row; i < data->end_row; i++)
    {
        vector<bool> visited(n, false);
        for (int j = 0; j < n; j++)
        {
            int index = sudoku[i][j] - 1;
            if (visited[index])
            {
                data->validity = false;
                data->local_output.push_back("Thread " + to_string(data->id) + " validated Row " + to_string(i) + " as invalid.");
                pthread_exit(NULL);
            }
            visited[index] = true;
        }
        data->local_output.push_back("Thread " + to_string(data->id) + " validated Row " + to_string(i) + " as valid.");
    }
    pthread_exit(NULL);
}

void *col_check(void *arg)
{
    thread_data *data = (thread_data *)arg;
    const vector<vector<int>> &sudoku = data->sudoku;
    int n = sudoku.size();

    for (int i = data->start_col; i < data->end_col; i++)
    {
        vector<bool> visited(n, false);
        for (int j = 0; j < n; j++)
        {
            int index = sudoku[j][i] - 1;
            if (visited[index])
            {
                data->validity = false;
                data->local_output.push_back("Thread " + to_string(data->id) + " validated Column " + to_string(i) + " as invalid.");
                pthread_exit(NULL);
            }
            visited[index] = true;
        }
        data->local_output.push_back("Thread " + to_string(data->id) + " validated Column " + to_string(i) + " as valid.");
    }
    pthread_exit(NULL);
}

void *subgrid_check(void *arg)
{
    thread_data *data = (thread_data *)arg;
    const vector<vector<int>> &sudoku = data->sudoku;
    int n = sqrt(sudoku.size());
    vector<bool> visited(sudoku.size(), false);

    for (int i = data->start_row; i < data->end_row; i++)
    {
        for (int j = data->start_col; j < data->end_col; j++)
        {
            int index = sudoku[i][j] - 1;
            if (visited[index])
            {
                data->validity = false;
                data->local_output.push_back("Thread " + to_string(data->id) + " validated Subgrid (" +
                                             to_string(data->start_row) + "," + to_string(data->start_col) + ") as invalid.");
                pthread_exit(NULL);
            }
            visited[index] = true;
        }
    }
    data->local_output.push_back("Thread " + to_string(data->id) + " validated Subgrid (" +
                                 to_string(data->start_row) + "," + to_string(data->start_col) + ") as valid.");
    pthread_exit(NULL);
}

int main()
{
    ifstream input_file("input.txt");
    ofstream output_file("output.txt");
    int K, N;
    input_file >> K >> N;
    vector<vector<int>> sudoku(N, vector<int>(N));

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            input_file >> sudoku[i][j];

    int chunk_size = N / K;
    int n = sqrt(N);
    vector<pthread_t> threads;
    vector<thread_data*> thread_data_i;

    auto start_time = high_resolution_clock::now();
    for (int i = 0; i < K; i++)
    {
        thread_data *data = new thread_data(i * chunk_size, (i + 1) * chunk_size, 0, N, i + 1, sudoku);
        thread_data_i.push_back(data);
        pthread_t thread;
        pthread_create(&thread, NULL, row_check, data);
        threads.push_back(thread);
    }

    for (int i = 0; i < K; i++)
    {
        thread_data *data = new thread_data(0, N, i * chunk_size, (i + 1) * chunk_size, i + K + 1, sudoku);
        thread_data_i.push_back(data);
        pthread_t thread;
        pthread_create(&thread, NULL, col_check, data);
        threads.push_back(thread);
    }

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
        {
            thread_data *data = new thread_data(i * n, (i + 1) * n, j * n, (j + 1) * n, i * n + j + 2 * K + 1, sudoku);
            thread_data_i.push_back(data);
            pthread_t thread;
            pthread_create(&thread, NULL, subgrid_check, data);
            threads.push_back(thread);
        }

    for (int i = 0; i < threads.size(); i++)
        pthread_join(threads[i], NULL);

    bool is_sudoku_valid = true;
    for (auto data : thread_data_i)
    {
        for (const auto &msg : data->local_output)
            output_file << msg << endl;
        if (!data->validity)
            is_sudoku_valid = false;
    }

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end_time - start_time);

    if (is_sudoku_valid)
    {
        output_file << "Sudoku is valid." << endl;
    }
    else
    {
        output_file << "Sudoku is invalid." << endl;
    }

    output_file << "Time taken: " << duration.count() << " microseconds." << endl;

    return 0;
}
