#pragma once

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <sstream>
#include <mutex>
#include <unordered_map>
#include <stdexcept>

using namespace std;

// Term类: 表示多项式中的单项式
class Term {
private:
    int coefficient_;  // 系数
    int exponent_;     // 指数

public:

    Term(int coefficient = 0, int exponent = 0);

    int get_coefficient() const { return coefficient_; }
    int get_exponent() const { return exponent_; }

    void set_coefficient(int coefficient) { coefficient_ = coefficient; }
    void set_exponent(int exponent) { exponent_ = exponent; }

    bool operator<(const Term& other) const {
        return exponent_ > other.exponent_; // Sort by exponent descending
    }

    bool operator==(const Term& other) const {
        return exponent_ == other.exponent_;
    }

    string to_string() const;
};

// Polynomial类: 表示多项式及其操作
class Polynomial {
private:
    Term *terms_;     // 项列表
    int cnt_;         // 项数量
    size_t capacity_; // 数组容量

    // 扩容
    void resize_if_needed();

    // 排序项
    void sort_terms();

    // 合并同类项
    void combine_like_terms();

    // 移除系数为零的项
    void remove_zero_terms();

public:

    explicit Polynomial(size_t capacity = 10);


    explicit Polynomial(const string& input, size_t capacity = 10);


    Polynomial(const Term* terms, int count, size_t capacity = 10);

    Polynomial(const Polynomial& other);


    Polynomial(Polynomial&& other) noexcept;

    Polynomial& operator=(const Polynomial& other);

 
    Polynomial& operator=(Polynomial&& other) noexcept;

    ~Polynomial();


    void add_term(const Term& term);

    int get_term_count() const { return cnt_; }

    const Term& get_term(int index) const;

    bool is_zero() const { return cnt_ == 0; }

    size_t capacity() const { return capacity_; }

    // 重载多项式运算符
    Polynomial operator+(const Polynomial& other) const;

    Polynomial operator-(const Polynomial& other) const;

    Polynomial operator*(const Polynomial& other) const;

    Polynomial& operator+=(const Polynomial& other);

    Polynomial& operator-=(const Polynomial& other);

    Polynomial& operator*=(const Polynomial& other);

    // 计算多项式在x处的值
    int evaluate(int x) const;

    // 计算多项式的导数
    Polynomial derivative() const;

    // 转换为标准输出格式字符串
    string to_standard_string() const;
    // 转换为LaTeX格式字符串
    string to_latex_string() const;

    void clear() { cnt_ = 0; }

    // 从字符串重建多项式
    void parse_from_string(const string& input);
};

// 多项式管理器类: 管理多个多项式及其操作
class PolynomialManager {
private:
    static mutex manager_mutex_;  // 线程安全互斥锁
    static unordered_map<char, Polynomial> polynomials_;  // 多项式存储
    static const int MAX_POLYNOMIALS;  // 最大多项式数量
    static const char POLYNOMIAL_NAMES[];  // 可用多项式名称 'a', 'b', 'c', 'd', 'e'

public:

    static int create_polynomial(char name, const string& input);

    static int get_polynomial_string(char name, string& result);

    static int get_polynomial_string_with_latex(char name, string& result);

    static int calculate_polynomials(const string& expr, string& result);

    static int calculate_polynomials_with_latex(const string& expr, string& result);

    static int evaluate_polynomial(char name, int x, int& result);

    static int derivative_polynomial(char name, string& result);

    static int derivative_polynomial_with_latex(char name, string& result);

    static void clear_all();

    static int get_polynomial_names(vector<char>& names);

    static int parse_expression(const string& expr, Polynomial& result);
};