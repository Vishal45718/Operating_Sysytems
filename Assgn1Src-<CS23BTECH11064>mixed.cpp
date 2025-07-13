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
    int id;
    const vector<vector<int>> &sudoku;
    vector<string> &row_output;
    vector<string> &col_output;
    vector<string> &subgrid_output;

    thread_data(int id, const vector<vector<int>> &sudoku,
                vector<string> &row_output, vector<string> &col_output, vector<string> &subgrid_output)
        : id(id), sudoku(sudoku), row_output(row_output), col_output(col_output), subgrid_output(subgrid_output) {}
};

void *row_check(void *arg)
{
    thread_data *data = (thread_data *)arg;
    const vector<vector<int>> &sudoku = data->sudoku;
    int N = sudoku.size();
    int n = sqrt(N);
    int id_subgrid = data->id;
    for (int i = id_subgrid; i < N; i += n)
    {
        vector<bool> visited(N, false);
        bool valid = true;
        for (int j = 0; j < N; j++)
        {
            int index = sudoku[i][j] - 1;
            if (visited[index])
            {
                valid = false;
                break;
            }
            visited[index] = true;
        }
        data->row_output[i] = "Thread " + to_string(data->id) + " checks row " + to_string(i + 1) + " and is " + (valid ? "valid." : "invalid.");
    }
    pthread_exit(NULL);
}

void *col_check(void *arg)
{
    thread_data *data = (thread_data *)arg;
    const vector<vector<int>> &sudoku = data->sudoku;
    int N = sudoku.size();
    int n = sqrt(N);
    int id_subgrid = data->id;
    for (int i = id_subgrid; i < N; i += n)
    {
        vector<bool> visited(N, false);
        bool valid = true;
        for (int j = 0; j < N; j++)
        {
            int index = sudoku[j][i] - 1;
            if (visited[index])
            {
                valid = false;
                break;
            }
            visited[index] = true;
        }
        data->col_output[i] = "Thread " + to_string(data->id) + " checks column " + to_string(i + 1) + " and is " + (valid ? "valid." : "invalid.");
    }
    pthread_exit(NULL);
}

void *subgrid_check(void *arg)
{
    thread_data *data = (thread_data *)arg;
    const vector<vector<int>> &sudoku = data->sudoku;
    int N = sudoku.size();
    int n = sqrt(N);
    int id_subgrid = data->id;
    int subgrid_start_row = (id_subgrid / n) * n;
    int subgrid_start_col = (id_subgrid % n) * n;

    for (int subgrid_row = subgrid_start_row; subgrid_row < N; subgrid_row += n)
    {
        for (int subgrid_col = subgrid_start_col; subgrid_col < N; subgrid_col += n)
        {
            vector<bool> visited(N, false);
            bool valid = true;

            for (int row = subgrid_row; row < subgrid_row + n; row++)
            {
                for (int col = subgrid_col; col < subgrid_col + n; col++)
                {
                    int index = sudoku[row][col] - 1;

                    if (visited[index] == true)
                    {
                        valid = false;
                        break;
                    }

                    visited[index] = true;
                }

                if (valid == false)
                    break;
            }

            int subgrid_index = (subgrid_row / n) * n + (subgrid_col / n);

            data->subgrid_output[subgrid_index] = "Thread " + to_string(data->id) + " checks grid (" + to_string(subgrid_row + 1) + "," + to_string(subgrid_col + 1) + ") and is " + (valid ? "valid." : "invalid.");
        }
    }
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
    {
        for (int j = 0; j < N; j++)
        {
            input_file >> sudoku[i][j];
        }
    }

    int n = sqrt(N);
    vector<pthread_t> threads;
    vector<string> row_output(N, "");
    vector<string> col_output(N, "");
    vector<string> subgrid_output(N, "");

    vector<thread_data *> thread_data_i;
    auto start_time = high_resolution_clock::now();

    for (int i = 0; i < K; i++)
    {
        thread_data *data = new thread_data(i, sudoku, row_output, col_output, subgrid_output);
        thread_data_i.push_back(data);
        pthread_t thread;
        pthread_create(&thread, NULL, row_check, data);
        threads.push_back(thread);
    }

    for (int i = 0; i < K; i++)
    {
        thread_data *data = new thread_data(i, sudoku, row_output, col_output, subgrid_output);
        thread_data_i.push_back(data);
        pthread_t thread;
        pthread_create(&thread, NULL, col_check, data);
        threads.push_back(thread);
    }

    for (int i = 0; i < K; i++)
    {
        thread_data *data = new thread_data(i, sudoku, row_output, col_output, subgrid_output);
        thread_data_i.push_back(data);
        pthread_t thread;
        pthread_create(&thread, NULL, subgrid_check, data);
        threads.push_back(thread);
    }

    for (int i = 0; i < threads.size(); i++)
    {
        pthread_join(threads[i], NULL);
    }

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end_time - start_time);

    bool is_sudoku_valid = true;

    for (int i = 0; i < row_output.size(); ++i)
    {
        const string &result = row_output[i];
        if (!result.empty())
        {
            output_file << result << endl;
            if (result.find("invalid") != string::npos)
            {
                is_sudoku_valid = false;
            }
        }
    }

    for (int i = 0; i < col_output.size(); ++i)
    {
        const string &result = col_output[i];
        if (!result.empty())
        {
            output_file << result << endl;
            if (result.find("invalid") != string::npos)
            {
                is_sudoku_valid = false;
            }
        }
    }

    for (int i = 0; i < subgrid_output.size(); i++)
    {
        const string &result = subgrid_output[i];
        if (!result.empty())
        {
            output_file << result << endl;
            if (result.find("invalid") != string::npos)
            {
                is_sudoku_valid = false;
            }
        }
    }

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
