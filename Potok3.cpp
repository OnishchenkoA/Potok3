#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <random>
#include <iomanip>

const int NUM_THREADS = 4;
const int BAR_WIDTH = 40;
const int MIN_STEP_MS = 50;
const int MAX_STEP_MS = 200;

std::mutex print_mutex;

struct ThreadInfo {
	int idx;
	double exec_time = 0.0;
	bool done = false;
};

std::vector<ThreadInfo> threads(NUM_THREADS);


int get_random_delay() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(MIN_STEP_MS, MAX_STEP_MS);
	return dis(gen);
}


void update_progress(int thread_idx, int progress) {
	std::lock_guard<std::mutex> lock(print_mutex);
	std::cout << "\033[" << (thread_idx + 2) << ";1H";
	std::cout << std::string(80, ' ') << "\r";
	std::string bar(BAR_WIDTH, ' ');
	for (int i = 0; i < progress; ++i) {
		bar[i] = '#';
	}

	int percent = (progress * 100) / BAR_WIDTH;
	std::cout << std::setw(2) << thread_idx << " | ["
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
	update_progress(thread_idx, 0);
	for (int step = 1; step <= BAR_WIDTH; ++step) {
		int delay_ms = get_random_delay();
		std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
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
	std::cout << "Поток | Прогресс                           | %   | Время\n";
	std::cout << std::string(60, '-') << "\n";
	std::vector<std::thread> thread_pool;
	for (int i = 0; i < NUM_THREADS; ++i) {
		thread_pool.emplace_back(thread_func, i);
	}
	for (auto& t : thread_pool) {
		t.join();
	}
	std::cout << std::string(60, '-') << "\n";

	return 0;
}