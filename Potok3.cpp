#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <iomanip>


const int NUM_THREADS = 4;          
const int BAR_WIDTH = 40;   
const int STEP_MS = 100;  

std::mutex print_mutex;
struct ThreadInfo {
	int idx;                   
	std::thread::id tid;       
	double exec_time = 0.0;   
	bool done = false;
};

std::vector<ThreadInfo> threads(NUM_THREADS);
void update_progress(int thread_idx, int progress) {
	std::lock_guard<std::mutex> lock(print_mutex);
	std::cout << "\033[" << (thread_idx + 2) << "A";
	std::string bar(BAR_WIDTH, ' ');
	for (int i = 0; i < progress; ++i) {
		bar[i] = '█';
	}

	int percent = (progress * 100) / BAR_WIDTH;
	std::cout << std::setw(2) << thread_idx << " | "
		<< threads[thread_idx].tid << " | ["
		<< bar << "] | " << percent << "%";
	if (threads[thread_idx].done) {
		std::cout << " | " << std::fixed << std::setprecision(3)
			<< threads[thread_idx].exec_time << " сек";
	}

	std::cout << std::endl;
	std::cout.flush();
}
void thread_func(int thread_idx) {
	auto start = std::chrono::high_resolution_clock::now();
	threads[thread_idx].idx = thread_idx;
	threads[thread_idx].tid = std::this_thread::get_id();

	update_progress(thread_idx, 0);
	for (int step = 1; step <= BAR_WIDTH; ++step) {
		std::this_thread::sleep_for(std::chrono::milliseconds(STEP_MS));
		update_progress(thread_idx, step);
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> dur = end - start;
	threads[thread_idx].exec_time = dur.count();
	threads[thread_idx].done = true;
	update_progress(thread_idx, BAR_WIDTH);
}

int main() {
	setlocale(LC_ALL, "Russian");
	for (int i = 0; i < NUM_THREADS; ++i) {
		std::cout << std::setw(2) << i << " | "
			<< "             " << " | ["
			<< std::string(BAR_WIDTH, ' ') << "] | 0%\n";
	}
	std::vector<std::thread> thread_pool;
	for (int i = 0; i < NUM_THREADS; ++i) {
		thread_pool.emplace_back(thread_func, i);
	}
	for (auto& t : thread_pool) {
		t.join();
	}
	std::cout << std::string(70, '-') << "\n";
	return 0;
}