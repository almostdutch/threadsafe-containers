/*
 * threadsafe_stack1.h
 *
 * Lock-based thread-safe unbounded stack implemented using library stack,
 * locks, a single mutex, and a condition variable.
 *
 */

#ifndef THREADSAFE_STACK1_H_
#define THREADSAFE_STACK1_H_

#include <stack> // std::stack
#include <memory> // std::unique_ptr
#include <list> // std::list
#include <utility> // std::move
#include <mutex> // std::mutex, std::lock_guard, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <exception> // std::exception

template<typename Element>
class ThreadSafeStack1 {
	typedef std::unique_ptr<Element> ElementPtr;
	typedef std::list<ElementPtr> Container;

	struct EmptyStack: public std::exception {
		virtual const char* what() const noexcept (true) override {
			return "Empty stack";
		}
	};
public:
	ThreadSafeStack1();
	~ThreadSafeStack1();
	ThreadSafeStack1(const ThreadSafeStack1&) = delete;
	ThreadSafeStack1& operator=(const ThreadSafeStack1&) = delete;
	ThreadSafeStack1(ThreadSafeStack1&&) = delete;
	ThreadSafeStack1& operator=(ThreadSafeStack1&&) = delete;

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
	std::stack<ElementPtr, Container> m_stack;
};

template<typename Element>
ThreadSafeStack1<Element>::ThreadSafeStack1() :
		m_stack(std::stack<ElementPtr, Container>()) {
}

template<typename Element>
ThreadSafeStack1<Element>::~ThreadSafeStack1() {
}

template<typename Element>
bool ThreadSafeStack1<Element>::empty() const {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_stack.empty();
}

template<typename Element>
size_t ThreadSafeStack1<Element>::size() const {
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_stack.size();
}

template<typename Element>
void ThreadSafeStack1<Element>::push(const Element &element) {
	ElementPtr new_element(std::make_unique<Element>(element));
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stack.push(std::move(new_element));
	}
	m_cond.notify_one();
}

template<typename Element>
void ThreadSafeStack1<Element>::push(Element &&element) {
	ElementPtr new_element(std::make_unique<Element>(std::move(element)));
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stack.push(std::move(new_element));
	}
	m_cond.notify_one();
}

template<typename Element>
template<typename ...Ts>
void ThreadSafeStack1<Element>::emplace(Ts &&... pars) {
	ElementPtr new_element(
			std::make_unique<Element>(std::forward<Ts>(pars)...));
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_stack.push(std::move(new_element));
	}
	m_cond.notify_one();
}

template<typename Element>
typename ThreadSafeStack1<Element>::ElementPtr ThreadSafeStack1<Element>::waitPop() {
	std::unique_lock<std::mutex> lock(m_mutex);
	m_cond.wait(lock, [this]() -> bool {
		return !this->m_stack.empty();
	});
	ElementPtr back_element(std::move(m_stack.top()));
	m_stack.pop();
	return back_element;
}

template<typename Element>
typename ThreadSafeStack1<Element>::ElementPtr ThreadSafeStack1<Element>::tryPop() {
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_stack.empty())
		return ElementPtr(nullptr);
	ElementPtr back_element(std::move(m_stack.top()));
	m_stack.pop();
	return back_element;
}

#endif /* THREADSAFE_STACK1_H_ */
