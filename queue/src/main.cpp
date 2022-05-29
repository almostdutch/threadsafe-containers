//============================================================================
// Script for testing the performance of four implementations of thread-safe queue
//============================================================================

#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "timer.h"
#include "threadsafe_queue1.h"
#include "threadsafe_queue2.h"
#include "threadsafe_queue3.h"
#include "threadsafe_queue4.h"
using namespace std;

void usageMsg(void) {
	string separator(50, '-');
	ostringstream msg;
	msg << separator << endl;
	msg
			<< "Usage: ./threadsafe_queue_test kNelements kNpushThreads kNpopThreads kTimeHeadStart kNiter"
			<< endl << endl;
	msg << "Where: " << endl;
	msg << "kNelements = number of elements to be PUSHed or POPed" << endl;
	msg << "kNpushThreads = number of data preparation threads (PUSH thread)"
			<< endl;
	msg << "kNpopThreads = number of data processing threads (POP thread)"
			<< endl;
	msg << "kTimeHeadStart = head start in [ms] for data processing threads"
			<< endl;
	msg << "kNiter = number of test runs (iterations)" << endl;
	msg << separator << endl;
	msg << "aborting.." << endl;
	cerr << msg.str() << endl;
	terminate();
}

// Function to PUSH the number of elements (kNelements) onto the queue
template<typename T>
void pushValues(T &queue, const size_t kNelements) {
	for (size_t ind = 0; ind < kNelements; ++ind)
		queue.push(ind);
}
// Function to POOP the number of elements (kNelements) off the queue
template<typename T>
void popValues(T &queue, const size_t kNelements) {
	for (size_t ind = 0; ind < kNelements; ++ind)
		queue.tryPop();
}

// Function to calculate mean and std dev of test run timings
string calcMeanStd(const vector<size_t> &results) {

	// mean
	double sum = std::accumulate(results.begin(), results.end(), 0.0);
	double mean = sum / results.size();

	// std dev
	double accum = 0.0;
	std::for_each(results.begin(), results.end(), [&](const double d) {
		accum += (d - mean) * (d - mean);
	});
	double stdev = sqrt(accum / (results.size() - 1));

	// write to string
	ostringstream os;
	os.precision(3);
	os << mean << " ± " << stdev;
	return os.str();
}

int main(int argc, char *argv[]) {

	if (argc < 6)
		usageMsg();

	// Timer
	Timer timer;

	// Print format parameters
	string separator(50, '-');
	const size_t kNsetwText = 25;
	const size_t kNsetwNumber = 10;

	// Test parameters
	const size_t kNelements = stoi(string(argv[1])); // number of elements to be PUSHed or POPed
	const size_t kNpushThreads = stoi(string(argv[2])); // number of data preparation threads (PUSH thread)
	const size_t kNpopThreads = stoi(string(argv[3])); // number of data processing threads (POP thread)
	const size_t kTimeHeadStart = stoi(string(argv[4])); // head start in [ms] for data processing threads
	const size_t kNiter = stoi(string(argv[5])); // number of test runs (iterations)
	vector<size_t> results; // container of results (timings of all test runs)
	vector<std::thread> threads; // container of threads

	cout << "Nelements: " << kNelements << endl;
	cout << "NpushThreads: " << kNpushThreads << endl;
	cout << "NpopThreads: " << kNpopThreads << endl;
	cout << "TimeHeadStart [ms]: " << kTimeHeadStart << endl;
	cout << "Niter: " << kNiter << endl;

	{
		// Test for queue #1
		results.clear();
		for (size_t iterNo = 0; iterNo < kNiter; ++iterNo) {
			ThreadSafeQueue1<int> q;

			timer.start();
			// Spawn data preparation threads
			for (size_t ind = 0; ind < kNpushThreads; ++ind)
				threads.push_back(
						std::thread(pushValues<decltype(q)>,
								std::reference_wrapper<decltype(q)>(q),
								kNelements));

			// Head start for data preparation threads
			this_thread::sleep_for(chrono::milliseconds(kTimeHeadStart));

			// Spawn data processing threads
			for (size_t threadNo = 0; threadNo < kNpopThreads; ++threadNo)
				threads.push_back(
						std::thread(popValues<decltype(q)>,
								std::reference_wrapper<decltype(q)>(q),
								kNelements));

			// Wait till we are done
			std::for_each(threads.begin(), threads.end(),
					std::mem_fun_ref(&std::thread::join));

			timer.stop();
			threads.clear();
			results.push_back(timer.duration() - kTimeHeadStart);
		}

		// Report result
		cout << separator << endl;
		cout << "Test for queue #1 (avg of " << kNiter << " runs)" << endl;

		cout << left << setw(kNsetwText) << "Size of empty queue: "
				<< setw(kNsetwNumber) << sizeof(ThreadSafeQueue1<int> )
				<< " [bytes]" << endl;

		cout << setw(kNsetwText) << "Test duration: " << setw(kNsetwNumber)
				<< calcMeanStd(results) << " [ms]" << endl;
		cout << separator << endl;
	}

	{
		// Test for queue #2
		results.clear();
		for (size_t iterNo = 0; iterNo < kNiter; ++iterNo) {
			ThreadSafeQueue2<int> q;

			timer.start();
			// Spawn data preparation threads
			for (size_t ind = 0; ind < kNpushThreads; ++ind)
				threads.push_back(
						std::thread(pushValues<decltype(q)>,
								std::reference_wrapper<decltype(q)>(q),
								kNelements));

			// Head start for data preparation threads
			this_thread::sleep_for(chrono::milliseconds(kTimeHeadStart));

			// Spawn data processing threads
			for (size_t threadNo = 0; threadNo < kNpopThreads; ++threadNo)
				threads.push_back(
						std::thread(popValues<decltype(q)>,
								std::reference_wrapper<decltype(q)>(q),
								kNelements));

			// Wait till we are done
			std::for_each(threads.begin(), threads.end(),
					std::mem_fun_ref(&std::thread::join));

			timer.stop();
			threads.clear();
			results.push_back(timer.duration() - kTimeHeadStart);
		}

		// Report result
		cout << separator << endl;
		cout << "Test for queue #2 (avg of " << kNiter << " runs)" << endl;

		cout << left << setw(kNsetwText) << "Size of empty queue: "
				<< setw(kNsetwNumber) << sizeof(ThreadSafeQueue2<int> )
				<< " [bytes]" << endl;

		cout << setw(kNsetwText) << "Test duration: " << setw(kNsetwNumber)
				<< calcMeanStd(results) << " [ms]" << endl;
		cout << separator << endl;
	}

	{
		// Test for queue #3
		results.clear();
		for (size_t iterNo = 0; iterNo < kNiter; ++iterNo) {
			ThreadSafeQueue3<int> q;

			timer.start();
			// Spawn data preparation threads
			for (size_t ind = 0; ind < kNpushThreads; ++ind)
				threads.push_back(
						std::thread(pushValues<decltype(q)>,
								std::reference_wrapper<decltype(q)>(q),
								kNelements));

			// Head start for data preparation threads
			this_thread::sleep_for(chrono::milliseconds(kTimeHeadStart));

			// Spawn data processing threads
			for (size_t threadNo = 0; threadNo < kNpopThreads; ++threadNo)
				threads.push_back(
						std::thread(popValues<decltype(q)>,
								std::reference_wrapper<decltype(q)>(q),
								kNelements));

			// Wait till we are done
			std::for_each(threads.begin(), threads.end(),
					std::mem_fun_ref(&std::thread::join));

			timer.stop();
			threads.clear();
			results.push_back(timer.duration() - kTimeHeadStart);
		}

		// Report result
		cout << separator << endl;
		cout << "Test for queue #3 (avg of " << kNiter << " runs)" << endl;

		cout << left << setw(kNsetwText) << "Size of empty queue: "
				<< setw(kNsetwNumber) << sizeof(ThreadSafeQueue3<int> )
				<< " [bytes]" << endl;

		cout << setw(kNsetwText) << "Test duration: " << setw(kNsetwNumber)
				<< calcMeanStd(results) << " [ms]" << endl;
		cout << separator << endl;
	}

	{
		// Test for queue #4
		results.clear();
		for (size_t iterNo = 0; iterNo < kNiter; ++iterNo) {
			ThreadSafeQueue4<int> q;

			timer.start();
			// Spawn data preparation threads
			for (size_t ind = 0; ind < kNpushThreads; ++ind)
				threads.push_back(
						std::thread(pushValues<decltype(q)>,
								std::reference_wrapper<decltype(q)>(q),
								kNelements));

			// Head start for data preparation threads
			this_thread::sleep_for(chrono::milliseconds(kTimeHeadStart));

			// Spawn data processing threads
			for (size_t threadNo = 0; threadNo < kNpopThreads; ++threadNo)
				threads.push_back(
						std::thread(popValues<decltype(q)>,
								std::reference_wrapper<decltype(q)>(q),
								kNelements));

			// Wait till we are done
			std::for_each(threads.begin(), threads.end(),
					std::mem_fun_ref(&std::thread::join));

			timer.stop();
			threads.clear();
			results.push_back(timer.duration() - kTimeHeadStart);
		}

		// Report result
		cout << separator << endl;
		cout << "Test for queue #4 (avg of " << kNiter << " runs)" << endl;

		cout << left << setw(kNsetwText) << "Size of empty queue: "
				<< setw(kNsetwNumber) << sizeof(ThreadSafeQueue4<int> )
				<< " [bytes]" << endl;

		cout << setw(kNsetwText) << "Test duration: " << setw(kNsetwNumber)
				<< calcMeanStd(results) << " [ms]" << endl;
		cout << separator << endl;
	}

	return 0;
}

