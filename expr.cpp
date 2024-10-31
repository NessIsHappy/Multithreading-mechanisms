#include <pthread.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <semaphore.h>
#define TCNT (3)
using namespace std;
#include <time.h>

pthread_t *tids;
sem_t semaphore;

int threads_count;
int mem_threads_count;
int N;
int S;
int solutions = 0;
int duration = 0;
//std::vector<int>A;
int *A;

struct thread_struct {
	int thread_starts;
	int thread_ends;
};

int permutes = 0;
int thread_job = 0;
int thread_job_left = 0;

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

void *thread_entry(void *param) {
	struct thread_struct *thread_struct = (struct thread_struct *)param;
	int cnt = 0;
	int i;
	int almostS;
	i = thread_struct->thread_starts;
	while (thread_struct->thread_ends > i && permutes > i) {
		i = i + 1;
		almostS = A[0];
		int j = 0;
		for (j = 0; j < N - 1; ++j) {
			if (!(i & (1 << j))) {
				almostS = almostS - A[j + 1];
			} else {
				almostS = almostS + A[j + 1];
			}
		}
		if (almostS == S) {
			cnt = cnt + 1;
		}
	}
	sem_wait(&semaphore);
	solutions = solutions + cnt;
	sem_post(&semaphore);
	return NULL;
}

/*
void *thread_entry(void *param) {
	const char *msg = (char *)param;
	int i;
	sem_wait(&semaphore);
	for (i = 0; i < 5; i++)
		cout << msg << endl;
	sem_post(&semaphore);
	return NULL;
}
*/

void threads_working() {
	int i;
	sem_init(&semaphore, 0, 1);
	tids = new pthread_t[threads_count]();
	struct thread_struct *thread_struct = new struct thread_struct[threads_count]();
	permutes = 1 << (N - 1);
	mem_threads_count = threads_count;
	permutes >= threads_count ? : threads_count = permutes;
	thread_job = permutes / threads_count;
	thread_job_left = permutes - threads_count * thread_job;
	int start = 0;
	int end = thread_job;
	for (i = 0; i < threads_count; i++) {
		thread_struct[i].thread_starts = start;
		if (thread_job_left > 0 && thread_job_left > i) {
			end = end + 1;
		}
		thread_struct[i].thread_ends = end;
		start = end;
		end = start + thread_job;
	}
	clock_t start_timer = clock();
	for (int i = 0; i < threads_count; i++) {
		pthread_create(&tids[i], NULL, thread_entry, (void *)&thread_struct[i]);
	}
	for (i = 0; i < threads_count; i++) {
		pthread_join(tids[i], NULL);
	}
	clock_t finish_timer = clock();
	duration = (finish_timer - start_timer) / CLOCKS_PER_SEC * 1000;
	cout << duration << endl;
	sem_destroy(&semaphore);
	delete[] tids;
	delete[] thread_struct;
}

// compiling: g++ expr.cpp -o expr.exe -pthread --std=c++11
// starting: ./expr.exe

int main(int argc, char *argv[]) {
	input_file();
	threads_working();
	output_file();
	time_file();
	return 0;
}