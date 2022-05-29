/*
 * threadsafe_stack2.h
 *
 * Lock-free thread-safe unbounded stack implemented using a singly-linked list,
 * and atomic operations with the strict memory models
 *
 */

#ifndef THREADSAFE_STACK2_H_
#define THREADSAFE_STACK2_H_

#include <memory> // std::shared_ptr, std::unique_ptr
#include <utility> // std::move
#include <atomic> // std::atomic, atomic_compare_exchange_weak, atomic_load
#include <exception> // std::exception

template<typename Element>
class ThreadSafeStack2 {
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
	ThreadSafeStack2();
	~ThreadSafeStack2();
	ThreadSafeStack2(const ThreadSafeStack2&) = delete;
	ThreadSafeStack2& operator=(const ThreadSafeStack2&) = delete;
	ThreadSafeStack2(ThreadSafeStack2&&) = delete;
	ThreadSafeStack2& operator=(ThreadSafeStack2&&) = delete;

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
ThreadSafeStack2<Element>::ThreadSafeStack2() :
		m_head(nullptr) {
}

template<typename Element>
ThreadSafeStack2<Element>::~ThreadSafeStack2() {
}

template<typename Element>
bool ThreadSafeStack2<Element>::empty() const {
	return !std::atomic_load(&m_head);
}

template<typename Element>
void ThreadSafeStack2<Element>::push(const Element &element) {
	NodePtr new_node = std::make_shared<Node>(element);
	new_node->next = std::atomic_load(&m_head);
	while (!std::atomic_compare_exchange_weak(&m_head, &new_node->next,
			new_node))
		;
}

template<typename Element>
void ThreadSafeStack2<Element>::push(Element &&element) {
	NodePtr new_node = std::make_shared<Node>(std::move(element));
	new_node->next = std::atomic_load(&m_head);
	while (!std::atomic_compare_exchange_weak(&m_head, &new_node->next,
			new_node))
		;
}

template<typename Element>
template<typename ...Ts>
void ThreadSafeStack2<Element>::emplace(Ts &&... pars) {
	NodePtr new_node = std::make_shared<Node>(std::forward<Ts>(pars)...);
	new_node->next = std::atomic_load(&m_head);
	while (!std::atomic_compare_exchange_weak(&m_head, &new_node->next,
			new_node))
		;
}

template<typename Element>
std::unique_ptr<Element> ThreadSafeStack2<Element>::tryPop() {
	NodePtr old_head = std::atomic_load(&m_head);
	while (old_head
			&& !std::atomic_compare_exchange_weak(&m_head, &old_head,
					old_head->next))
		;
	return old_head ?
			std::move(old_head->m_data) : std::unique_ptr<Element>(nullptr);
}

#endif /* THREADSAFE_STACK2_H_ */
