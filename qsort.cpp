#include <windows.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <time.h>
#define TCNT (3)
using namespace std;

struct task{
	int left;
	int right;
};

vector<struct task> queue;

int threads_count;
int N;
int* A;
vector<int>results;
HANDLE *threads;
HANDLE mutex;
HANDLE event;
int duration;

int sorted = 0;

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
}

void time_file() {
	ofstream timefile("time.txt");
	timefile << duration << endl;
}

vector<task>tasksQueue;

/*
DWORD WINAPI thread_entry(void* param) {
	const char* msg = (const char*)param;
	int i;
	WaitForSingleObject(mutex, INFINITE);
	for (i = 0; i < 5; i++) {
		printf("%s ", msg);
	}
	ReleaseMutex(mutex);
	return 0;
}
*/

void synchOff() {
	SetEvent(event);
	ReleaseMutex(mutex);
}

int qsort(int *arr, int left, int right) {
	if (left >= right)
		return 1;

	int mid = arr[(left + right) / 2];
	int i = left, j = right;
	int result;
	result = 0;

	while (i <= j) {
		while (arr[i] < mid) {
			i = i + 1;
		}
		while (arr[j] > mid) {
			j = j - 1;
		}

		if (i <= j) {
			int32_t tmp = arr[i];
			arr[i] = arr[j];
			arr[j] = tmp;
			i = i + 1;
			j = j - 1;
		}
	}

	if (right - left <= 1000) {
		qsort(arr, left, j);
		qsort(arr, i, right);
		result = right - left + 1;
		return result;
	}
	else {
		struct task queueElem;
		WaitForSingleObject(mutex, INFINITE);
		queueElem.left = left;
		queueElem.right = j;
		queue.push_back(queueElem);
		synchOff();
		int dif;
		dif = 0;
		dif = i - j;
		if (dif >= 2) {
			WaitForSingleObject(mutex, INFINITE);
			int leftElem = j + 1;
			int rightElem = i - 1;
			queueElem.left = leftElem;
			queueElem.right = rightElem;
			queue.push_back(queueElem);
			synchOff();
		}
		WaitForSingleObject(mutex, INFINITE);
		queueElem.left = i;
		queueElem.right = right;
		queue.push_back(queueElem);
		synchOff();
		return result;
	}
}

int sumElements() {
	int sum = 0;
	for (int value : results) {
		sum += value;
	}
	return sum;
}


DWORD WINAPI thread_entry(void* param) {
	int result;
	int queueState;
	int left = 0, right = 0;
	result = 0;
	for (; sumElements() < N;) {
		WaitForSingleObject(event, INFINITE);
		WaitForSingleObject(mutex, INFINITE);
		queueState = queue.empty();
		if (queueState) {
			synchOff();
		}
		else {
			struct task queueElem = queue.back();
			queue.pop_back();
			ReleaseMutex(mutex);
			WaitForSingleObject(mutex, INFINITE);
			left = queueElem.left;
			right = queueElem.right;
			results.push_back(qsort(A, left, right));
			synchOff();
		}
	}
	synchOff();
	return NULL;
}

void threads_working() {
	HANDLE* threads = (HANDLE*)calloc(threads_count, sizeof(HANDLE));
	mutex = CreateMutex(NULL, FALSE, NULL);
	event = CreateEvent(NULL, FALSE, TRUE, NULL);

	for (int i = 0; i < threads_count; i++)
		threads[i] = CreateThread(0, 0, thread_entry, 0, 0, 0);

	struct task firstInQueue;
	firstInQueue.left = 0;
	firstInQueue.right = N - 1;
	queue.push_back(firstInQueue);
	clock_t start = clock();
	while (WAIT_TIMEOUT == WaitForMultipleObjects(threads_count, threads, TRUE, INFINITE));
	clock_t finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC * 1000;
	for (int i = 0; i < threads_count; i++)
		CloseHandle(threads[i]);
	CloseHandle(mutex);
	CloseHandle(event);
	free(threads);
}

int main() {
	input_file();
	threads_working();
	output_file();
	time_file();
}
