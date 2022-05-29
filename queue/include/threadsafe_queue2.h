/*
 * threadsafe_queue2.h
 *
 * Lock-based thread-safe unbounded queue implemented using a singly-linked list,
 * locks, fined-tuned mutexes (front and back mutex), and a condition variable.
 *
 */

#ifndef THREADSAFE_QUEUE2_H_
#define THREADSAFE_QUEUE2_H_

#include <memory> // std::unique_ptr
#include <utility> // std::move
#include <mutex> // std::mutex, std::lock_guard, std::unique_lock
#include <condition_variable> // std::condition_variable
#include <exception> // std::exception

template<typename Element>
class ThreadSafeQueue2 {
	typedef std::unique_ptr<Element> ElementPtr;
	class Node; // forward declaration
	typedef std::unique_ptr<Node> NodePtr;

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
		ElementPtr m_data;
		NodePtr next;
	};
public:
	ThreadSafeQueue2();
	~ThreadSafeQueue2();
	ThreadSafeQueue2(const ThreadSafeQueue2&) = delete;
	ThreadSafeQueue2& operator=(const ThreadSafeQueue2&) = delete;
	ThreadSafeQueue2(ThreadSafeQueue2&&) = delete;
	ThreadSafeQueue2& operator=(ThreadSafeQueue2&&) = delete;

	const Node* getBackLabel() const;
	bool empty() const;
	void push(const Element &element);
	void push(Element &&element);
	template<typename ...Ts>
	void emplace(Ts &&... pars);
	ElementPtr waitPop();
	ElementPtr tryPop();
private:
	mutable std::mutex m_mutex_front;
	mutable std::mutex m_mutex_back;
	std::condition_variable m_cond;
	NodePtr m_node_front;
	Node *m_node_back;
};

template<typename Element>
ThreadSafeQueue2<Element>::ThreadSafeQueue2() :
		m_node_front(std::make_unique<Node>()), m_node_back(m_node_front.get()) {
}

template<typename Element>
ThreadSafeQueue2<Element>::~ThreadSafeQueue2() {
}

template<typename Element>
const typename ThreadSafeQueue2<Element>::Node* ThreadSafeQueue2<Element>::getBackLabel() const {
	std::lock_guard<std::mutex> lock_back(m_mutex_back);
	return m_node_back;
}

template<typename Element>
bool ThreadSafeQueue2<Element>::empty() const {
	std::lock_guard<std::mutex> lock_front(m_mutex_front);
	return !m_node_front->next;
}

template<typename Element>
void ThreadSafeQueue2<Element>::push(const Element &element) {
	NodePtr new_node(std::make_unique<Node>(element));
	Node *new_back = new_node.get();
	{
		std::lock_guard<std::mutex> lock_back(m_mutex_back);
		m_node_back->m_data = std::move(new_node->m_data);
		m_node_back->next = std::move(new_node);
		m_node_back = new_back;
	}
	m_cond.notify_one();
}

template<typename Element>
void ThreadSafeQueue2<Element>::push(Element &&element) {
	NodePtr new_node(std::make_unique<Node>(std::move(element)));
	Node *new_back = new_node.get();
	{
		std::lock_guard<std::mutex> lock_back(m_mutex_back);
		m_node_back->m_data = std::move(new_node->m_data);
		m_node_back->next = std::move(new_node);
		m_node_back = new_back;
	}
	m_cond.notify_one();
}

template<typename Element>
template<typename ...Ts>
void ThreadSafeQueue2<Element>::emplace(Ts &&... pars) {
	NodePtr new_node(std::make_unique<Node>(std::forward<Ts>(pars)...));
	Node *new_back = new_node.get();
	{
		std::lock_guard<std::mutex> lock_back(m_mutex_back);
		m_node_back->m_data = std::move(new_node->m_data);
		m_node_back->next = std::move(new_node);
		m_node_back = new_back;
	}
	m_cond.notify_one();
}

template<typename Element>
typename ThreadSafeQueue2<Element>::ElementPtr ThreadSafeQueue2<Element>::waitPop() {
	std::unique_lock<std::mutex> lock_front(m_mutex_front);
	m_cond.wait(lock_front, [this]() -> bool {
		return m_node_front->next;
	});
	NodePtr front_node(std::move(m_node_front));
	m_node_front = std::move(front_node->next);
	return std::move(front_node->m_data);
}

template<typename Element>
typename ThreadSafeQueue2<Element>::ElementPtr ThreadSafeQueue2<Element>::tryPop() {
	std::unique_lock<std::mutex> lock_front(m_mutex_front);
	if (!m_node_front->next)
		return ElementPtr(nullptr);
	NodePtr front_node(std::move(m_node_front));
	m_node_front = std::move(front_node->next);
	return std::move(front_node->m_data);
}
#endif /* THREADSAFE_QUEUE2_H_ */
