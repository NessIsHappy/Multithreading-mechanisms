#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <semaphore.h>
#define TCNT (3)
using namespace std;
#include <time.h>

pthread_mutex_t mutex;
pthread_mutex_t output;
pthread_cond_t condition[5];
pthread_t philosophers[5];
int action[5];
int total;
int phil;
struct timeval start;

void *thread_entry(void* arg);
int time_from_start(void);
void start_eating(int i);
void start_thinking(int i);
int check_outtime(void);
void threads_working(int argc, char *argv[]);

int threads_count;
int mem_threads_count;
int N;
int S;
int solutions = 0;
int duration = 0;
//std::vector<int>A;
int *A;

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
    A = (int *)calloc(N, sizeof(int));
    for (int i = 0; i < N; i++) {
        input >> A[i];
        cout << A[i] << " ";
    }
    input >> S;
    input.close();
}

void output_file() {
    ofstream output("output.txt");
    output << mem_threads_count << endl;
    output << N << endl;
    output << solutions << endl;
}

void time_file() {
    ofstream timefile("time.txt");
    timefile << duration << endl;
}

void sleep_ms(int ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

int main(int argc, char* argv[]) {

    //input_file();
    threads_working(argc, argv);
    //output_file();
    //time_file();
    return 0;
}

void threads_working(int argc, char *argv[]) {
    int i;
    if (argc == 3) {
        total = atoi(argv[1]);
        phil = atoi(argv[2]);
    }
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&output, NULL);
    for (i = 0; i < 5; i += 1) {
        action[i] = 0;
        pthread_cond_init(&condition[i], NULL);
    }
    gettimeofday(&start, NULL);
    int philosopher_ids[5];
    i = 0;
    while(i < 5) {
        philosopher_ids[i] = i;
        pthread_create(&philosophers[i], NULL, thread_entry, &philosopher_ids[i]);
        i += 1;
    }
    pthread_join(philosophers[0], NULL);
    pthread_join(philosophers[1], NULL);
    pthread_join(philosophers[2], NULL);
    pthread_join(philosophers[3], NULL);
    pthread_join(philosophers[4], NULL);
    pthread_cond_destroy(&condition[0]);
    pthread_cond_destroy(&condition[1]);
    pthread_cond_destroy(&condition[2]);
    pthread_cond_destroy(&condition[3]);
    pthread_cond_destroy(&condition[4]);
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&output);
}

void *thread_entry(void* arg) {
    int i = *((int*)arg);
    char prev = 0;

    while (true) {
        pthread_mutex_lock(&mutex);
        if (check_outtime() == 1) break;
        pthread_mutex_unlock(&mutex);
        sleep_ms(phil);
        pthread_mutex_lock(&mutex);
        start_eating(i);
        pthread_mutex_unlock(&mutex);
        pthread_mutex_lock(&mutex);
        if (check_outtime() == 1) break;
        pthread_mutex_unlock(&mutex);

        if (action[i]) {
            cout << time_from_start() << ":" << (i + 1) << ":" << 'T' << "->" << 'E' << endl;
            prev = 1;
            sleep_ms(phil);
            start_thinking(i);
            prev = 0;
        }
        if (time_from_start() >= total) {
            pthread_mutex_lock(&mutex);
            int k = 0;
            while (k < 5) {
                pthread_cond_broadcast(&condition[k]);
                k++;
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
    }
    if (prev) {
        start_thinking(i);
    }

    return NULL;
}

// thinkng - 0, eating - 1

int time_from_start() {
    pthread_mutex_lock(&output);
    struct timeval now;
    gettimeofday(&now, NULL);
    int result = (now.tv_sec - start.tv_sec) * 1000 + (now.tv_usec - start.tv_usec) / 1000;
    pthread_mutex_unlock(&output);
    return result;
}

void start_eating(int i) {
    if (time_from_start() <= total && action[(i + 5 - 1) % 5] == 0 && action[(i + 1) % 5] == 0) {
        action[i] = 1;
        pthread_cond_signal(&condition[i]);
    }
    while (action[i] == 0 && time_from_start() <= total) {
        pthread_cond_wait(&condition[i], &mutex);
    }
}

void start_thinking(int i) {
    pthread_mutex_lock(&mutex);
    action[i] = 0;
    cout << time_from_start() << ":" << (i + 1) << ":" << 'E' << "->" << 'T' << endl;
    int left = (i + 5 - 1) % 5;
    int right = (i + 1) % 5;
    if (time_from_start() <= total && action[(left + 5 - 1) % 5] == 0 && action[(left + 1) % 5] == 0) {
        action[left] = 1;
        pthread_cond_signal(&condition[left]);
    }
    if (time_from_start() <= total && action[(right + 5 - 1) % 5] == 0 && action[(right + 1) % 5] == 0) {
        action[right] = 1;
        pthread_cond_signal(&condition[right]);
    }
    pthread_mutex_unlock(&mutex);
}

int check_outtime() {
    if (time_from_start() > total) {
        pthread_mutex_unlock(&mutex);
        return 1;
    }
    return 0;
}
