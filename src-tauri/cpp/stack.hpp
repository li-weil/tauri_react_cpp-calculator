#pragma once

#include <vector>
#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>

/**
 * @brief Template stack class implementation
 * @tparam T Type of elements stored in the stack
 */
template <typename T>
class Stack {
private:
    T *data_;   //data_ 私有成员命名规范
    int cnt_;
    size_t capacity_;    //size_t非负无符号数

public:
    /**
     * @brief Constructor
     * @param capacity Stack capacity, 0 means unlimited
     */
    explicit Stack(size_t capacity = 10000)  //explicit 防止隐式转换
        : capacity_(capacity), cnt_(0){
        data_ = new T[capacity_];
    }

    /**
     * @brief Destructor
     */
    ~Stack(){
        delete[] data_;
    }

    /**
     * @brief Push operation
     * @param item Item to push onto the stack
     * @throw std::overflow_error If stack is full (only in fixed capacity mode)
     */
    void push(const T& item) {
        if (cnt_ == capacity_) {
            // 对于 POD 类型使用 memcpy，对于非 POD 类型使用循环复制
            T *tmp = new T[capacity_];
            for (int i = 0; i < cnt_; ++i) {
                tmp[i] = data_[i];
            }
            delete[] data_;
            capacity_ *= 2;  // 更新容量
            data_ = new T[capacity_];
            for (int i = 0; i < cnt_; ++i) {
                data_[i] = tmp[i];
            }
            delete[] tmp;
        }
        data_[cnt_++] = item;
    }

    /**
     * @brief Move semantic push operation
     * @param item Item to push onto the stack (rvalue reference)
     */
    void push(T&& item) {
        if (cnt_ == capacity_) {
            // 对于 POD 类型使用 memcpy，对于非 POD 类型使用循环复制
            T *tmp = new T[capacity_];
            for (int i = 0; i < cnt_; ++i) {
                tmp[i] = std::move(data_[i]);
            }
            delete[] data_;
            capacity_ *= 2;  // 更新容量
            data_ = new T[capacity_];
            for (int i = 0; i < cnt_; ++i) {
                data_[i] = std::move(tmp[i]);
            }
            delete[] tmp;
        }
        data_[cnt_++] = std::move(item);
    }

    /**
     * @brief Pop operation
     * @return Top element of the stack
     * @throw std::underflow_error If stack is empty
     */
    T pop() {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        T item = std::move(data_[cnt_ - 1]);
        cnt_--;
        return item;
    }

    /**
     * @brief Peek at top element (without popping)
     * @return Const reference to top element
     * @throw std::underflow_error If stack is empty
     */
    const T& top() const {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        return data_[cnt_-1];
    }

    /**
     * @brief Peek at top element (without popping)
     * @return Reference to top element
     * @throw std::underflow_error If stack is empty
     */
    T& top() {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        return data_[cnt_-1];
    }

    /**
     * @brief Check if stack is empty
     * @return true if stack is empty
     */
    bool empty() const noexcept {
        return cnt_ == 0;
    }

    /**
     * @brief Get number of elements in stack
     * @return Number of elements
     */
    size_t size() const noexcept {
        return cnt_;
    }

    /**
     * @brief Get stack capacity
     * @return Capacity, 0 means unlimited
     */
    size_t capacity() const noexcept {
        return capacity_;
    }


 

    /**
     * @brief Clear the stack
     */
    void clear() noexcept {
        cnt_ = 0;
    }

    /**
     * @brief Get copy of all elements in stack (from bottom to top)
     * @return Vector containing all elements
     */
    std::vector<T> get_all_elements() const {
        std::vector<T> result;
        result.reserve(cnt_);
        for (int i = 0; i < cnt_; ++i) {
            result.push_back(data_[i]);
        }
        return result;
    }



    /**
     * @brief Get string representation of stack
     * @return String representation of stack contents
     */
    std::string to_string() const {
        std::ostringstream oss;
        oss << "Stack[";
        for (size_t i = 0; i < cnt_; ++i) {
            if (i > 0) oss << ", ";
            oss << data_[i];
        }
        oss << "]";
        return oss.str();
    }


};