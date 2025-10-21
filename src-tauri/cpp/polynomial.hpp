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

/**
 * @brief Term class: Represents a single term (coefficient, exponent) in a polynomial
 */
class Term {
private:
    int coefficient_;  // coefficient
    int exponent_;     // exponent

public:
    /**
     * @brief Constructor
     * @param coefficient coefficient
     * @param exponent exponent
     */
    Term(int coefficient = 0, int exponent = 0);

    // Getters
    int get_coefficient() const { return coefficient_; }
    int get_exponent() const { return exponent_; }

    // Setters
    void set_coefficient(int coefficient) { coefficient_ = coefficient; }
    void set_exponent(int exponent) { exponent_ = exponent; }

    // Comparison operators (for sorting)
    bool operator<(const Term& other) const {
        return exponent_ > other.exponent_; // Sort by exponent descending
    }

    bool operator==(const Term& other) const {
        return exponent_ == other.exponent_;
    }

    // Convert to string
    string to_string() const;
};

/**
 * @brief Polynomial class: Represents a sparse polynomial
 */
class Polynomial {
private:
    Term *terms_;     // Array of terms
    int cnt_;         // Number of terms
    size_t capacity_; // Capacity of the array

    /**
     * @brief Resize array if needed
     */
    void resize_if_needed();

    /**
     * @brief Sort terms by exponent descending
     */
    void sort_terms();

    /**
     * @brief Combine like terms
     */
    void combine_like_terms();

    /**
     * @brief Remove terms with coefficient 0
     */
    void remove_zero_terms();

public:
    /**
     * @brief Default constructor
     */
    explicit Polynomial(size_t capacity = 10);

    /**
     * @brief Constructor from string
     * @param input format: "c1,e1,c2,e2,..." coefficient-exponent pairs
     */
    explicit Polynomial(const string& input, size_t capacity = 10);

    /**
     * @brief Constructor from C-style array
     * @param terms term array
     * @param count number of terms
     */
    Polynomial(const Term* terms, int count, size_t capacity = 10);

    /**
     * @brief Copy constructor
     */
    Polynomial(const Polynomial& other);

    /**
     * @brief Move constructor
     */
    Polynomial(Polynomial&& other) noexcept;

    /**
     * @brief Copy assignment operator
     */
    Polynomial& operator=(const Polynomial& other);

    /**
     * @brief Move assignment operator
     */
    Polynomial& operator=(Polynomial&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~Polynomial();

    // Term operations
    /**
     * @brief Add term
     * @param term term to add
     */
    void add_term(const Term& term);

    /**
     * @brief Get term count
     */
    int get_term_count() const { return cnt_; }

    /**
     * @brief Get term at specified index
     */
    const Term& get_term(int index) const;

    /**
     * @brief Check if polynomial is zero polynomial
     */
    bool is_zero() const { return cnt_ == 0; }

    /**
     * @brief Get array capacity
     */
    size_t capacity() const { return capacity_; }

    // Arithmetic operators
    /**
     * @brief Polynomial addition
     */
    Polynomial operator+(const Polynomial& other) const;

    /**
     * @brief Polynomial subtraction
     */
    Polynomial operator-(const Polynomial& other) const;

    /**
     * @brief Polynomial multiplication
     */
    Polynomial operator*(const Polynomial& other) const;

    /**
     * @brief Compound addition assignment
     */
    Polynomial& operator+=(const Polynomial& other);

    /**
     * @brief Compound subtraction assignment
     */
    Polynomial& operator-=(const Polynomial& other);

    /**
     * @brief Compound multiplication assignment
     */
    Polynomial& operator*=(const Polynomial& other);

    // Evaluation
    /**
     * @brief Calculate polynomial value at x
     * @param x input value
     * @return result value
     */
    int evaluate(int x) const;

    /**
     * @brief Calculate polynomial derivative
     * @return derivative polynomial
     */
    Polynomial derivative() const;

    // Conversion
    /**
     * @brief Convert to standard format string
     * @return format: "n,c1,e1,c2,e2,..."
     */
    string to_standard_string() const;

    /**
     * @brief Convert to human readable string
     */
    string to_readable_string() const;

    /**
     * @brief Convert to LaTeX format string
     */
    string to_latex_string() const;

    /**
     * @brief Clear polynomial
     */
    void clear() { cnt_ = 0; }

    /**
     * @brief Parse polynomial from string
     * @param input format: "c1,e1,c2,e2,..." coefficient-exponent pairs
     */
    void parse_from_string(const string& input);
};

/**
 * @brief Polynomial manager class: Manages multiple polynomials (a,b,c,d,e)
 */
class PolynomialManager {
private:
    static mutex manager_mutex_;  // Thread safety mutex
    static unordered_map<char, Polynomial> polynomials_;  // Polynomial storage
    static const int MAX_POLYNOMIALS;  // Maximum number of polynomials
    static const char POLYNOMIAL_NAMES[];  // Available polynomial names 'a', 'b', 'c', 'd', 'e'

public:
    /**
     * @brief Create polynomial
     * @param name polynomial name ('a'-'e')
     * @param input input string
     * @return 0: success, -1: invalid name, -2: format error, -3: too many polynomials
     */
    static int create_polynomial(char name, const string& input);

    /**
     * @brief Get polynomial standard format string
     * @param name polynomial name
     * @param result output result
     * @return 0: success, -1: invalid name, -2: polynomial not found
     */
    static int get_polynomial_string(char name, string& result);

    /**
     * @brief Get polynomial both standard and LaTeX format strings
     * @param name polynomial name
     * @param result output result in format "standard|latex"
     * @return 0: success, -1: invalid name, -2: polynomial not found
     */
    static int get_polynomial_string_with_latex(char name, string& result);

    /**
     * @brief Calculate polynomial expression result
     * @param expr expression (e.g. "a+b", "a-b*c")
     * @param result output result
     * @return 0: success, other: error code
     */
    static int calculate_polynomials(const string& expr, string& result);

    /**
     * @brief Calculate polynomial expression result with LaTeX
     * @param expr expression (e.g. "a+b", "a-b*c")
     * @param result output result in format "standard|latex"
     * @return 0: success, other: error code
     */
    static int calculate_polynomials_with_latex(const string& expr, string& result);

    /**
     * @brief Calculate polynomial value at x
     * @param name polynomial name
     * @param x input value
     * @param result output result
     * @return 0: success, other: error code
     */
    static int evaluate_polynomial(char name, int x, int& result);

    /**
     * @brief Calculate polynomial derivative
     * @param name polynomial name
     * @param result output result
     * @return 0: success, other: error code
     */
    static int derivative_polynomial(char name, string& result);

    /**
     * @brief Calculate polynomial derivative with LaTeX
     * @param name polynomial name
     * @param result output result in format "standard|latex"
     * @return 0: success, other: error code
     */
    static int derivative_polynomial_with_latex(char name, string& result);

    /**
     * @brief Clear all polynomials
     */
    static void clear_all();

    /**
     * @brief Get created polynomial name list
     * @param names name list
     * @return polynomial count
     */
    static int get_polynomial_names(vector<char>& names);

    /**
     * @brief Parse polynomial expression and compute result
     * @param expr expression (e.g. "a+b-c*d")
     * @param result output result
     * @return 0: success, other: error code
     */
    static int parse_expression(const string& expr, Polynomial& result);
};