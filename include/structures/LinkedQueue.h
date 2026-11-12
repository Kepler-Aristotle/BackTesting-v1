#ifndef QUANTBENCH_LINKED_QUEUE_H
#define QUANTBENCH_LINKED_QUEUE_H

#include <cstddef>
#include <utility>
#include "exceptions/CustomExceptions.h"

namespace quantbench {

/**
 * @brief Generic singly-linked-list FIFO Queue template.
 * Demonstrates Module 4: Templates, custom linked-list data structure implementation,
 * dynamic memory management (RAII), and Rule of Five safety.
 */
template <typename T>
class LinkedQueue {
private:
    struct Node {
        T data;
        Node* next;

        explicit Node(const T& val) : data(val), next(nullptr) {}
        explicit Node(T&& val) : data(std::move(val)), next(nullptr) {}
    };

    Node* front_;
    Node* rear_;
    size_t size_;

public:
    LinkedQueue() noexcept : front_(nullptr), rear_(nullptr), size_(0) {}

    // Destructor (RAII cleanup - prevents memory leaks)
    ~LinkedQueue() noexcept {
        clear();
    }

    // Deep Copy Constructor
    LinkedQueue(const LinkedQueue& other) : front_(nullptr), rear_(nullptr), size_(0) {
        Node* curr = other.front_;
        while (curr != nullptr) {
            enqueue(curr->data);
            curr = curr->next;
        }
    }

    // Copy Assignment Operator
    LinkedQueue& operator=(const LinkedQueue& other) {
        if (this != &other) {
            LinkedQueue temp(other);
            swap(temp);
        }
        return *this;
    }

    // Move Constructor
    LinkedQueue(LinkedQueue&& other) noexcept
        : front_(other.front_), rear_(other.rear_), size_(other.size_) {
        other.front_ = nullptr;
        other.rear_ = nullptr;
        other.size_ = 0;
    }

    // Move Assignment Operator
    LinkedQueue& operator=(LinkedQueue&& other) noexcept {
        if (this != &other) {
            clear();
            front_ = other.front_;
            rear_ = other.rear_;
            size_ = other.size_;
            other.front_ = nullptr;
            other.rear_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void swap(LinkedQueue& other) noexcept {
        std::swap(front_, other.front_);
        std::swap(rear_, other.rear_);
        std::swap(size_, other.size_);
    }

    /**
     * @brief Enqueues an item at the back of the FIFO queue.
     */
    void enqueue(const T& value) {
        Node* newNode = new Node(value);
        if (empty()) {
            front_ = rear_ = newNode;
        } else {
            rear_->next = newNode;
            rear_ = newNode;
        }
        ++size_;
    }

    /**
     * @brief Removes the front item from the FIFO queue.
     * Throws EmptyQueueException if the queue is empty.
     */
    void dequeue() {
        if (empty()) {
            throw EmptyQueueException();
        }
        Node* temp = front_;
        front_ = front_->next;
        if (front_ == nullptr) {
            rear_ = nullptr;
        }
        delete temp;
        --size_;
    }

    /**
     * @brief Returns a reference to the front item without removing it.
     */
    T& peek() {
        if (empty()) {
            throw EmptyQueueException();
        }
        return front_->data;
    }

    const T& peek() const {
        if (empty()) {
            throw EmptyQueueException();
        }
        return front_->data;
    }

    bool empty() const noexcept {
        return front_ == nullptr;
    }

    size_t size() const noexcept {
        return size_;
    }

    void clear() noexcept {
        while (front_ != nullptr) {
            Node* temp = front_;
            front_ = front_->next;
            delete temp;
        }
        rear_ = nullptr;
        size_ = 0;
    }
};

} // namespace quantbench

#endif // QUANTBENCH_LINKED_QUEUE_H
