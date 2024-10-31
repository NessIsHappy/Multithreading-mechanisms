#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include <ctime>
#include <time.h>
#include <stdio.h>
using namespace std;

int threads_count;
int N = 0, L = 0, K = 0;
int **stolKings;
int solutions = 0;
int duration = 0;
pthread_mutex_t mutex;
pthread_t *threads;

void desk_init(int **board) {
    board = (int**)calloc(N, sizeof(int *));
    for (int i = 0; i < N; i++) {
        board[i] = (int *)calloc(N, sizeof(int));
        for (int j = 0; j < N; j++) {
            board[i][j] = 0;
        }
    }
}

void input_file() {
    ifstream input("input2.txt");

    if (!input.is_open()) {
        cout << "Error: didn't open file!" << endl;
        return;
    }
    input >> threads_count;
    input >> N;
    input >> L;
    input >> K;
    cout << N << L << K << endl;

    stolKings = (int**)calloc(N, sizeof(int *));
    for (int i = 0; i < N; i++) {
        stolKings[i] = (int *)calloc(N, sizeof(int));
        for (int j = 0; j < N; j++) {
            stolKings[i][j] = 0;
        }
    }
    if (N < 2) {
        cout << "Error: N must be >= 2" << endl;
        return;
    }
    int i = 0, j = 0;
    for (int k = 0; k < K; k++) {
        input >> i;
        input >> j;
        cout << i << j << endl;
        stolKings[i][j] = 1;
        cout << i << j << endl;
    }
    input.close();
}

void output_file() {
    ofstream output("output.txt");
    output << solutions << endl;
    output.close();
}

void time_file() {
    ofstream timefile("time.txt");
    timefile << duration << endl;
    timefile.close();
}

bool is_safe(int x, int y, int **desk) {
    if (desk[x][y] || (x+1 < N && desk[x+1][y]) || (x+1 < N && y+1 < N && desk[x+1][y+1]) || 
        (x+1 < N && y-1 >= 0 && desk[x+1][y-1]) || (y+1<N && desk[x][y+1]) || (y-1>=0 && desk[x][y-1]) || 
        (x-1 >= 0 && desk[x-1][y]) || (x-1>=0 && y-1>=0 && desk[x-1][y-1]) || (x-1>=0 && y+1<N && desk[x-1][y+1])) {
        return 0;
    }
    return 1;
}

struct info {
    int thread;
    int start;
    int end;
};

void place_kings(int cur_row, int placed_kings, int **cur_desk) {
    if (placed_kings == L) {
        pthread_mutex_lock(&mutex);
        solutions = solutions + 1;
        pthread_mutex_unlock(&mutex);
        return;
    } else if (cur_row >= N) {
        return;
    }
    for (int i = 0; i < N; i++) {
        if (is_safe(cur_row, i, cur_desk)) {
            cur_desk[cur_row][i] = 1;
            int new_row = cur_row + 1;
            int new_kings = placed_kings + 1;
            place_kings(new_row, new_kings, cur_desk);
            cur_desk[cur_row][i] = 0;
        }
    }
    place_kings(cur_row + 1, placed_kings, cur_desk);
}
/*
void free_board(int **board) {
    for (int i = 0; i < N; i++) {
        free(board[i]);
    }
    free(board);
}
*/
void *thread_entry(void *param) {
    struct info *info = (struct info *)param;
    int start = info->start;
    int end = info->end;
    int **cur_desk;
    cur_desk = (int**)calloc(N, sizeof(int *));
    for (int i = 0; i < N; i++) {
        cur_desk[i] = (int *)calloc(N, sizeof(int));
        for (int j = 0; j < N; j++) {
            cur_desk[i][j] = 0;
        }
    }
    int i = start, j = 0;
    while(i < end) {
        while(j < N) {
            if (is_safe(i, j, cur_desk)) {
                cur_desk[i][j] = 1;
                int new_i = i + 1;
                place_kings(new_i, 1, cur_desk);
                cur_desk[i][j] = 0;
            }
            j = j + 1;
        }
        i = i + 1;
    }
    //free_board(cur_desk);
    return NULL;
}

void threads_working() {
    int i;
    threads = (new pthread_t[threads_count]());
    pthread_mutex_init(&mutex, NULL);
    struct info *info = (struct info *)calloc(N, sizeof(struct info));
    for (i = 0; i < threads_count; i++) {
        info[i].thread = i;
        info[i].start = i * (N / threads_count);
        if (i == threads_count - 1) {
            info[i].end = N;
        } else {
            info[i].end = (i + 1) * (N / threads_count);
        }
        pthread_create(&threads[i], NULL, thread_entry, (void *)&info[i]);
    }
    clock_t start = clock();
    for (i = 0; i < threads_count; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_t finish = clock();
    duration = (finish - start) / CLOCKS_PER_SEC * 1000;
    pthread_mutex_destroy(&mutex);
    delete[] threads;
    delete[] info;
    //free_board(stolKings);

}

int main() {
    input_file();
    threads_working();
    output_file();
    time_file();
}
