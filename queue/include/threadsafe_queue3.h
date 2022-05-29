/*
 * threadsafe_queue3.h
 *
 * Lock-free thread-safe unbounded queue implemented using a singly-linked list,
 * and atomic operations with the strict memory models
 *
 */

#ifndef THREADSAFE_QUEUE3_H_
#define THREADSAFE_QUEUE3_H_

#include <memory> // std::shared_ptr, std::unique_ptr
#include <utility> // std::move
#include <atomic> // std::atomic, atomic_exchange, atomic_load
#include <exception> // std::exception

template<typename Element>
class ThreadSafeQueue3 {
	typedef std::shared_ptr<Element> ElementPtr;
	typedef std::unique_ptr<Element> ElementUPtr;
	class Node; // forward declaration
	typedef std::shared_ptr<Node> NodePtr;

	struct EmptyQueue: public std::exception {
		virtual const char* what() const noexcept (true) override {
			return "Empty Queue";
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
	ThreadSafeQueue3();
	~ThreadSafeQueue3();
	ThreadSafeQueue3(const ThreadSafeQueue3&) = delete;
	ThreadSafeQueue3& operator=(const ThreadSafeQueue3&) = delete;
	ThreadSafeQueue3(ThreadSafeQueue3&&) = delete;
	ThreadSafeQueue3& operator=(ThreadSafeQueue3&&) = delete;

	bool empty() const;
	void push(const Element &element);
	void push(Element &&element);
	template<typename ...Ts>
	void emplace(Ts &&... pars);
	std::unique_ptr<Element> tryPop();
private:
	std::atomic<Node*> m_label_back;
	NodePtr m_label_front;
};

template<typename Element>
ThreadSafeQueue3<Element>::ThreadSafeQueue3() :
		m_label_back(new Node()), m_label_front(m_label_back.load()) {
}

template<typename Element>
ThreadSafeQueue3<Element>::~ThreadSafeQueue3() {
}

template<typename Element>
bool ThreadSafeQueue3<Element>::empty() const {
	return !std::atomic_load(&m_label_front)->next;
}

template<typename Element>
void ThreadSafeQueue3<Element>::push(const Element &element) {
	ElementUPtr new_element(std::make_unique<Element>(element));
	NodePtr new_node(std::make_shared<Node>());
	Node *old_back = m_label_back.exchange(new_node.get());
	old_back->m_data = std::move(new_element);
	old_back->next = std::move(new_node);
}

template<typename Element>
void ThreadSafeQueue3<Element>::push(Element &&element) {
	ElementUPtr new_element(std::make_unique<Element>(std::move(element)));
	NodePtr new_node(std::make_shared<Node>());
	Node *old_back = m_label_back.exchange(new_node.get());
	old_back->m_data = std::move(new_element);
	old_back->next = std::move(new_node);
}

template<typename Element>
template<typename ...Ts>
void ThreadSafeQueue3<Element>::emplace(Ts &&... pars) {
	ElementUPtr new_element(
			std::make_unique<Element>(std::forward<Ts>(pars)...));
	NodePtr new_node(std::make_shared<Node>());
	Node *old_back = m_label_back.exchange(new_node.get());
	old_back->m_data = std::move(new_element);
	old_back->next = std::move(new_node);
}

template<typename Element>
std::unique_ptr<Element> ThreadSafeQueue3<Element>::tryPop() {
	NodePtr front_node(std::atomic_load(&m_label_front));
	while (front_node->next
			&& !std::atomic_compare_exchange_weak(&m_label_front, &front_node,
					front_node->next))
		;
	return front_node->next ?
			std::move(front_node->m_data) : std::unique_ptr<Element>(nullptr);
}

#endif /* THREADSAFE_QUEUE3_H_ */
