#ifndef TIMER_H_
#define TIMER_H_

#include <chrono>

class Timer {
public:
	Timer(void) :
			dur(0) {
	}
	~Timer() {
	}
	void start(void) {
		t1 = std::chrono::high_resolution_clock::now();
	}
	void stop(void) {
		t2 = std::chrono::high_resolution_clock::now();
	}
	const size_t duration(void) {
		dur =
				std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
		return dur;
	}
private:
	std::chrono::high_resolution_clock::time_point t1;
	std::chrono::high_resolution_clock::time_point t2;
	size_t dur;
};

#endif /* TIMER_H_ */

