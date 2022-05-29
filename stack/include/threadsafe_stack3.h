/*
 * threadsafe_stack3.h
 *
 * Lock-free thread-safe unbounded stack implemented using a singly-linked list,
 * and atomic operations with the relaxed memory models
 *
 */

#ifndef THREADSAFE_STACK3_H_
#define THREADSAFE_STACK3_H_

#include <memory> // std::shared_ptr, std::unique_ptr
#include <utility> // std::move
#include <atomic> // std::atomic, atomic_compare_exchange_weak, atomic_load
#include <exception> // std::exception

template<typename Element>
class ThreadSafeStack3 {
	typedef std::shared_ptr<Element> ElementPtr;
	typedef std::unique_ptr<Element> ElementUPtr;
	class Node; // forward declaration
	typedef std::shared_ptr<Node> NodePtr;

	struct EmptyStack: public std::exception {
		virtual const char* what() const noexcept (true) override {
			return "Empty stack";
		}
	};

	struct Node {
		Node() :
				m_data(nullptr), next(nullptr) {
		}
		Node(const Element &element) :
				m_data(std::make_unique<Element>(element)), next(nullptr) {
		}
		Node(Element &&element) :
				m_data(std::make_unique<Element>(std::move(element))), next(
						nullptr) {
		}
		~Node() = default;
		ElementUPtr m_data;
		NodePtr next;
	};
public:
	ThreadSafeStack3();
	~ThreadSafeStack3();
	ThreadSafeStack3(const ThreadSafeStack3&) = delete;
	ThreadSafeStack3& operator=(const ThreadSafeStack3&) = delete;
	ThreadSafeStack3(ThreadSafeStack3&&) = delete;
	ThreadSafeStack3& operator=(ThreadSafeStack3&&) = delete;

	bool empty() const;
	void push(const Element &element);
	void push(Element &&element);
	template<typename ...Ts>
	void emplace(Ts &&... pars);
	std::unique_ptr<Element> tryPop();
private:
	NodePtr m_head;
};

template<typename Element>
ThreadSafeStack3<Element>::ThreadSafeStack3() :
		m_head(nullptr) {
}

template<typename Element>
ThreadSafeStack3<Element>::~ThreadSafeStack3() {
}

template<typename Element>
bool ThreadSafeStack3<Element>::empty() const {
	return !std::atomic_load_explicit(&m_head, std::memory_order_relaxed);
}

template<typename Element>
void ThreadSafeStack3<Element>::push(const Element &element) {
	NodePtr new_node = std::make_shared<Node>(element);
	new_node->next = std::atomic_load_explicit(&m_head,
			std::memory_order_relaxed);
	while (!std::atomic_compare_exchange_weak_explicit(&m_head, &new_node->next,
			new_node, std::memory_order_relaxed, std::memory_order_relaxed))
		;
}

template<typename Element>
void ThreadSafeStack3<Element>::push(Element &&element) {
	NodePtr new_node = std::make_shared<Node>(std::move(element));
	new_node->next = std::atomic_load_explicit(&m_head,
			std::memory_order_relaxed);
	while (!std::atomic_compare_exchange_weak_explicit(&m_head, &new_node->next,
			new_node, std::memory_order_relaxed, std::memory_order_relaxed))
		;
}

template<typename Element>
template<typename ...Ts>
void ThreadSafeStack3<Element>::emplace(Ts &&... pars) {
	NodePtr new_node = std::make_shared<Node>(std::forward<Ts>(pars)...);
	new_node->next = std::atomic_load_explicit(&m_head,
			std::memory_order_relaxed);
	while (!std::atomic_compare_exchange_weak_explicit(&m_head, &new_node->next,
			new_node, std::memory_order_relaxed, std::memory_order_relaxed))
		;
}

template<typename Element>
std::unique_ptr<Element> ThreadSafeStack3<Element>::tryPop() {
	NodePtr old_head = std::atomic_load_explicit(&m_head,
			std::memory_order_relaxed);
	while (old_head
			&& !std::atomic_compare_exchange_weak_explicit(&m_head, &old_head,
					old_head->next, std::memory_order_relaxed,
					std::memory_order_relaxed))
		;
	return old_head ?
			std::move(old_head->m_data) : std::unique_ptr<Element>(nullptr);
}

#endif /* THREADSAFE_STACK3_H_ */
