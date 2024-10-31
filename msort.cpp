#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#define TCNT (3)
using namespace std;

int threads_count;
int N;
int* A;
int duration = 0;

HANDLE sem;
CRITICAL_SECTION cs;

void input_file() {
    char buff[50];

    ifstream input("input.txt");

    if (!input.is_open()) {
        cout << "Error: didn't open file!" << endl;
        return;
    }
    input >> threads_count;
    input >> N;
    if (N < 2) {
        cout << "Error: N must be >= 2" << endl;
        return;
    }
    A = (int*)calloc(N, sizeof(int));
    for (int i = 0; i < N; i++) {
        input >> A[i];
        //cout << A[i] << endl;
    }
    //cout << A[N - 1];
    input.close();
}

void output_file() {
    ofstream output("output.txt");
    output << threads_count << endl;
    output << N << endl;
    for (int i = 0; i < N; i++) {
        //cout << A[i] << " ";
        output << A[i] << " ";
    }
    //cout << A[N - 1];
}

void time_file() {
    ofstream timefile("time.txt");
    timefile << duration << endl;
}

struct task {
    int left_index;
    int right_index;
};

struct info {
    int left;
    int mid;
    int right;
    int left_size;
    int right_size;
};

void startSynch() {
    WaitForSingleObject(sem, INFINITE);
    EnterCriticalSection(&cs);
}

void finishSynch() {
    LeaveCriticalSection(&cs);
    ReleaseSemaphore(sem, 1, NULL);
}

void merge(int* Anums, struct info info) {
    int* temp_left = (int*)calloc(info.mid - info.left + 1, sizeof(int));
    int* temp_right = (int*)calloc(info.right - info.mid, sizeof(int));
    int left_size = info.left_size;
    int right_size = info.right_size;
    int left = info.left;
    int right = info.right;
    int mid = info.mid;
    int addr;
    int i;
    i = 0;
    while(i < left_size) {
        addr = i + left;
        temp_left[i] = Anums[addr];
        i = i + 1;
    }
    i = 0;
    while(i < right_size) {
        addr = i + mid + 1;
        temp_right[i] = Anums[addr];
        i = i + 1;
    }
    int j;
    i = 0;
    j = 0;
    int left_save = left;
    while (i < left_size && j < right_size) {
        if (temp_left[i] <= temp_right[j]) {
            Anums[left_save] = temp_left[i];
            left_save = left_save + 1;
            i = i + 1;
        }
        else {
            Anums[left_save] = temp_right[j];
            left_save = left_save + 1;
            j = j + 1;
        }
    }
    for (; left_size > i; i = i + 1, left_save = left_save + 1) {
        startSynch();
        Anums[left_save] = temp_left[i];
        finishSynch();
    }
    for (; right_size > j; j = j + 1, left_save = left_save + 1) {
        startSynch();
        Anums[left_save] = temp_right[j];
        finishSynch();
    }
}

int work_threads = 0;
vector<task>tasksQueue;

void merge_sort(int* Anums, int left, int right) {
    if (left >= right) {
        return;
    }
    else {
        int newRight = left + (right - left) / 2;
        int newLeft = newRight + 1;
        merge_sort(A, left, newRight);
        merge_sort(A, newLeft, right);
        struct info info;
        info.left = left;
        info.right = right;
        info.mid = newRight;
        info.left_size = (right - left) / 2 + 1;
        info.right_size = right - (left + (right - left) / 2);
        merge(A, info);
    }
}

DWORD WINAPI thread_entry(void* param) {
    merge_sort(A, tasksQueue[(int)param].left_index, tasksQueue[(int)param].right_index);
    return NULL;
}

void threads_working() {
    HANDLE* handles = new HANDLE[threads_count];
    const char* msgs[TCNT] = { "Hello!", "Bonjour!", "Terve!" };
    int i, k;
    sem = CreateSemaphore(NULL, threads_count, threads_count, NULL);
    InitializeCriticalSection(&cs);

    for (i = 0, k = 0; i < threads_count; i++) {
        struct task task;
        task.left_index = k;
        i == threads_count - 1 ? task.right_index = N - 1 : task.right_index = (k + N / threads_count) % N;
        tasksQueue.push_back(task);
        handles[i] = CreateThread(0, 0, thread_entry, (void*)i, 0, 0);
        k = k + N / threads_count + 1;
    }

    clock_t start = clock();
    while (WAIT_TIMEOUT == WaitForMultipleObjects(threads_count, handles, TRUE, INFINITE));
    for (int i = 0; i < N; i++) {
        //cout << A[i] << endl;
    }
    int left = tasksQueue[0].left_index;
    int right = tasksQueue[0].right_index;
    for (int i = 1; i < threads_count; i++) {
        struct info info;
        info.left = left;
        info.right = tasksQueue[i].right_index;
        info.mid = right;
        info.left_size = right - left + 1;
        info.right_size = tasksQueue[i].right_index - right;
        merge(A, info);
        right = tasksQueue[i].right_index;
    }
    clock_t finish = clock();
    duration = (double)(finish - start) / CLOCKS_PER_SEC * 1000;
    for (i = 0; i < threads_count; i++)
        CloseHandle(handles[i]);

    CloseHandle(sem);
    DeleteCriticalSection(&cs);
    delete[] handles;
}

int main() {
    input_file();
    threads_working();
    output_file();
    time_file();
}