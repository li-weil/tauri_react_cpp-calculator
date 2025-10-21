#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>


template <typename T>
class Stack {
private:
    T *data_;   //data_ 私有成员命名规范
    int cnt_;
    size_t capacity_;    //size_t非负无符号数

public:
    explicit Stack(size_t capacity = 10000)  //explicit 防止隐式转换
        : capacity_(capacity), cnt_(0){
        data_ = new T[capacity_];
    }


    ~Stack(){
        delete[] data_;
    }

    void push(const T& item) {
        if (cnt_ == capacity_) {
            capacity_ <<= 1;
            T *tmp = new T[capacity_];
            for (int i = 0; i < cnt_; ++i) {
                tmp[i] = data_[i];
            }
            delete[] data_;
            data_ = tmp;
        }
        data_[cnt_++] = item;
    }

    void push(T&& item) {
        if (cnt_ == capacity_) {
            capacity_ <<= 1;
            T *tmp = new T[capacity_];
            for (int i = 0; i < cnt_; ++i) {
                tmp[i] = std::move(data_[i]);
            }
            delete[] data_;
            data_ = tmp;
        }
        data_[cnt_++] = std::move(item);
    }

    T pop() {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        T item = std::move(data_[cnt_ - 1]);
        cnt_--;
        return item;
    }

    const T& top() const {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        return data_[cnt_-1];
    }

    T& top() {
        if (empty()) {
            throw std::underflow_error("Stack is empty");
        }
        return data_[cnt_-1];
    }

    bool empty() const noexcept {
        return cnt_ == 0;
    }

    size_t size() const noexcept {
        return cnt_;
    }

    size_t capacity() const noexcept {
        return capacity_;
    }

    void clear() noexcept {
        cnt_ = 0;
    }

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