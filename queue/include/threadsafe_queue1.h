/*
 * threadsafe_queue1.h
 *
 * Lock-based thread-safe unbounded queue implemented using library queue,
 * locks, a single mutex, and a condition variable.
 *
 */

#ifndef THREADSAFE_QUEUE1_H_
#define THREADSAFE_QUEUE1_H_

#include <queue> // std::queue
#include <memory> // std::unique_ptr
#include <list> // std::list
#include <utility> // std::move
#include <mutex> // std::mutex, std::lock_guard, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <exception> // std::exception

template<typename Element>
class ThreadSafeQueue1 {
	typedef std::unique_ptr<Element> ElementPtr;
	typedef std::list<ElementPtr> Container;

	struct EmptyQueue: public std::exception {
		virtual const char* what() const noexcept (true) override {
			return "Empty Queue";
		}
	};
public:
	ThreadSafeQueue1();
	~ThreadSafeQueue1();
	ThreadSafeQueue1(const ThreadSafeQueue1&) = delete;
	ThreadSafeQueue1& operator=(const ThreadSafeQueue1&) = delete;
	ThreadSafeQueue1(ThreadSafeQueue1&&) = delete;
	ThreadSafeQueue1& operator=(ThreadSafeQueue1&&) = delete;

	bool empty() const;
	size_t size() const;
	void push(const Element &element);
	void push(Element &&element);
	template<typename ...Ts>
	void emplace(Ts &&... pars);
	ElementPtr waitPop();
	ElementPtr tryPop();
private:
	mutable std::mutex m_mutex;
	std::condition_variable m_cond;
	std::queue<ElementPtr, Container> m_queue;
};

template<typename Element>
ThreadSafeQueue1<Element>::ThreadSafeQueue1() {
}

template<typename Element>
ThreadSafeQueue1<Element>::~ThreadSafeQueue1() {
}

template<typename Element>
bool ThreadSafeQueue1<Element>::empty() const {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_queue.empty();
}

template<typename Element>
size_t ThreadSafeQueue1<Element>::size() const {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_queue.size();
}

template<typename Element>
void ThreadSafeQueue1<Element>::push(const Element &element) {
	ElementPtr new_element(std::make_unique<Element>(element));
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push(std::move(new_element));
	}
	m_cond.notify_one();
}

template<typename Element>
void ThreadSafeQueue1<Element>::push(Element &&element) {
	ElementPtr new_element(std::make_unique<Element>(std::move(element)));
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push(std::move(new_element));
	}
	m_cond.notify_one();
}

template<typename Element>
template<typename ...Ts>
void ThreadSafeQueue1<Element>::emplace(Ts &&... pars) {
	ElementPtr new_element(
			std::make_unique<Element>(std::forward<Ts>(pars)...));
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push(std::move(new_element));
	}
	m_cond.notify_one();
}

template<typename Element>
typename ThreadSafeQueue1<Element>::ElementPtr ThreadSafeQueue1<Element>::waitPop() {
	std::unique_lock<std::mutex> lock(m_mutex);
	m_cond.wait(lock, [this]() -> bool {
		return !m_queue.empty();
	});
	ElementPtr front_element(std::move(m_queue.front()));
	m_queue.pop();
	return front_element;
}

template<typename Element>
typename ThreadSafeQueue1<Element>::ElementPtr ThreadSafeQueue1<Element>::tryPop() {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_queue.empty())
		return ElementPtr(nullptr);
	ElementPtr front_element(std::move(m_queue.front()));
	m_queue.pop();
	return front_element;
}

#endif /* THREADSAFE_QUEUE1_H_ */
