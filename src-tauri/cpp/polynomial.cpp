#include "polynomial.hpp"
#include <iostream>
#include <cctype>
#include <stack>
#include <stdexcept>
#include <algorithm>
#include <cstring>

using namespace std;

// ============================================================================
// Term Class Implementation
// ============================================================================

/**
 * @brief Term constructor
 */
Term::Term(int coefficient, int exponent)
    : coefficient_(coefficient), exponent_(exponent) {
}

/**
 * @brief Convert to string
 */
string Term::to_string() const {
    return "(" + std::to_string(coefficient_) + "x^" + std::to_string(exponent_) + ")";
}

// ============================================================================
// Polynomial Class Implementation
// ============================================================================

/**
 * @brief Default constructor
 */
Polynomial::Polynomial(size_t capacity)
    : capacity_(capacity), cnt_(0) {
    terms_ = new Term[capacity_];
}

/**
 * @brief Constructor from string
 */
Polynomial::Polynomial(const string& input, size_t capacity)
    : capacity_(capacity), cnt_(0) {
    terms_ = new Term[capacity_];
    parse_from_string(input);
}

/**
 * @brief Constructor from C-style array
 */
Polynomial::Polynomial(const Term* terms, int count, size_t capacity)
    : capacity_(capacity > count ? capacity : count * 2), cnt_(count) {
    terms_ = new Term[capacity_];
    for (int i = 0; i < cnt_; ++i) {
        terms_[i] = terms[i];
    }
    sort_terms();
    combine_like_terms();
    remove_zero_terms();
}

/**
 * @brief Copy constructor
 */
Polynomial::Polynomial(const Polynomial& other)
    : capacity_(other.capacity_), cnt_(other.cnt_) {
    terms_ = new Term[capacity_];
    for (int i = 0; i < cnt_; ++i) {
        terms_[i] = other.terms_[i];
    }
}

/**
 * @brief Move constructor
 */
Polynomial::Polynomial(Polynomial&& other) noexcept
    : terms_(other.terms_), cnt_(other.cnt_), capacity_(other.capacity_) {
    other.terms_ = nullptr;
    other.cnt_ = 0;
    other.capacity_ = 0;
}

/**
 * @brief Copy assignment operator
 */
Polynomial& Polynomial::operator=(const Polynomial& other) {
    if (this != &other) {
        delete[] terms_;
        capacity_ = other.capacity_;
        cnt_ = other.cnt_;
        terms_ = new Term[capacity_];
        for (int i = 0; i < cnt_; ++i) {
            terms_[i] = other.terms_[i];
        }
    }
    return *this;
}

/**
 * @brief Move assignment operator
 */
Polynomial& Polynomial::operator=(Polynomial&& other) noexcept {
    if (this != &other) {
        delete[] terms_;
        terms_ = other.terms_;
        cnt_ = other.cnt_;
        capacity_ = other.capacity_;
        other.terms_ = nullptr;
        other.cnt_ = 0;
        other.capacity_ = 0;
    }
    return *this;
}

/**
 * @brief Destructor
 */
Polynomial::~Polynomial() {
    delete[] terms_;
}

/**
 * @brief Resize array if needed
 */
void Polynomial::resize_if_needed() {
    if (cnt_ == capacity_) {
        size_t new_capacity = capacity_ * 2;
        Term* new_terms = new Term[new_capacity];
        for (int i = 0; i < cnt_; ++i) {
            new_terms[i] = terms_[i];
        }
        delete[] terms_;
        terms_ = new_terms;
        capacity_ = new_capacity;
    }
}

/**
 * @brief Sort terms by exponent descending
 */
void Polynomial::sort_terms() {
    // Simple bubble sort for small arrays
    for (int i = 0; i < cnt_ - 1; ++i) {
        for (int j = 0; j < cnt_ - i - 1; ++j) {
            if (terms_[j].get_exponent() < terms_[j + 1].get_exponent()) {
                Term temp = terms_[j];
                terms_[j] = terms_[j + 1];
                terms_[j + 1] = temp;
            }
        }
    }
}

/**
 * @brief Combine like terms
 */
void Polynomial::combine_like_terms() {
    if (cnt_ == 0) return;

    int write_idx = 0;
    for (int read_idx = 1; read_idx < cnt_; ++read_idx) {
        if (terms_[write_idx].get_exponent() == terms_[read_idx].get_exponent()) {
            // Combine like terms
            int new_coeff = terms_[write_idx].get_coefficient() + terms_[read_idx].get_coefficient();
            terms_[write_idx].set_coefficient(new_coeff);
        } else {
            ++write_idx;
            terms_[write_idx] = terms_[read_idx];
        }
    }
    cnt_ = write_idx + 1;
}

/**
 * @brief Remove terms with coefficient 0
 */
void Polynomial::remove_zero_terms() {
    int write_idx = 0;
    for (int read_idx = 0; read_idx < cnt_; ++read_idx) {
        if (terms_[read_idx].get_coefficient() != 0) {
            terms_[write_idx] = terms_[read_idx];
            ++write_idx;
        }
    }
    cnt_ = write_idx;
}

/**
 * @brief Add term
 */
void Polynomial::add_term(const Term& term) {
    resize_if_needed();
    terms_[cnt_] = term;
    ++cnt_;
    sort_terms();
    combine_like_terms();
    remove_zero_terms();
}

/**
 * @brief Get term at specified index
 */
const Term& Polynomial::get_term(int index) const {
    if (index < 0 || index >= cnt_) {
        throw out_of_range("Term index out of range");
    }
    return terms_[index];
}

/**
 * @brief Polynomial addition
 */
Polynomial Polynomial::operator+(const Polynomial& other) const {
    Polynomial result(cnt_ + other.cnt_);

    // Copy terms from both polynomials
    for (int i = 0; i < cnt_; ++i) {
        result.add_term(terms_[i]);
    }
    for (int i = 0; i < other.cnt_; ++i) {
        result.add_term(other.terms_[i]);
    }

    return result;
}

/**
 * @brief Polynomial subtraction
 */
Polynomial Polynomial::operator-(const Polynomial& other) const {
    Polynomial result(cnt_ + other.cnt_);

    // Copy terms from first polynomial
    for (int i = 0; i < cnt_; ++i) {
        result.add_term(terms_[i]);
    }

    // Add negated terms from second polynomial
    for (int i = 0; i < other.cnt_; ++i) {
        result.add_term(Term(-other.terms_[i].get_coefficient(), other.terms_[i].get_exponent()));
    }

    return result;
}

/**
 * @brief Polynomial multiplication
 */
Polynomial Polynomial::operator*(const Polynomial& other) const {
    Polynomial result(cnt_ * other.cnt_ + 10);

    for (int i = 0; i < cnt_; ++i) {
        for (int j = 0; j < other.cnt_; ++j) {
            int new_coeff = terms_[i].get_coefficient() * other.terms_[j].get_coefficient();
            int new_exp = terms_[i].get_exponent() + other.terms_[j].get_exponent();
            result.add_term(Term(new_coeff, new_exp));
        }
    }

    return result;
}

/**
 * @brief Compound addition assignment
 */
Polynomial& Polynomial::operator+=(const Polynomial& other) {
    *this = *this + other;
    return *this;
}

/**
 * @brief Compound subtraction assignment
 */
Polynomial& Polynomial::operator-=(const Polynomial& other) {
    *this = *this - other;
    return *this;
}

/**
 * @brief Compound multiplication assignment
 */
Polynomial& Polynomial::operator*=(const Polynomial& other) {
    *this = *this * other;
    return *this;
}

/**
 * @brief Calculate polynomial value at x
 */
int Polynomial::evaluate(int x) const {
    int result = 0;

    for (int i = 0; i < cnt_; ++i) {
        int term_value = terms_[i].get_coefficient();
        for (int j = 0; j < terms_[i].get_exponent(); ++j) {
            term_value *= x;
        }
        result += term_value;
    }

    return result;
}

/**
 * @brief Calculate polynomial derivative
 */
Polynomial Polynomial::derivative() const {
    Polynomial result(cnt_ + 5);

    for (int i = 0; i < cnt_; ++i) {
        if (terms_[i].get_exponent() > 0) {
            int new_coeff = terms_[i].get_coefficient() * terms_[i].get_exponent();
            int new_exp = terms_[i].get_exponent() - 1;
            result.add_term(Term(new_coeff, new_exp));
        }
        // Constant term derivative is 0, no need to add
    }

    return result;
}

/**
 * @brief Convert to standard format string
 */
string Polynomial::to_standard_string() const {
    if (cnt_ == 0) {
        return "0";
    }

    string result = to_string(cnt_);
    for (int i = 0; i < cnt_; ++i) {
        result += "," + to_string(terms_[i].get_coefficient()) + "," + to_string(terms_[i].get_exponent());
    }

    return result;
}

/**
 * @brief Convert to human readable string
 */
string Polynomial::to_readable_string() const {
    if (cnt_ == 0) {
        return "0";
    }

    string result;
    bool first = true;

    for (int i = 0; i < cnt_; ++i) {
        int coeff = terms_[i].get_coefficient();
        int exp = terms_[i].get_exponent();

        if (!first) {
            if (coeff > 0) {
                result += " + ";
            } else {
                result += " - ";
                coeff = -coeff;
            }
        } else {
            if (coeff < 0) {
                result += "-";
                coeff = -coeff;
            }
            first = false;
        }

        if (coeff != 1 || exp == 0) {
            result += to_string(coeff);
        }

        if (exp > 0) {
            result += "x";
            if (exp > 1) {
                result += "^" + to_string(exp);
            }
        }
    }

    return result;
}

/**
 * @brief Convert to LaTeX format string
 */
string Polynomial::to_latex_string() const {
    if (cnt_ == 0) {
        return "0";
    }

    string result;
    bool first = true;

    for (int i = 0; i < cnt_; ++i) {
        int coeff = terms_[i].get_coefficient();
        int exp = terms_[i].get_exponent();

        if (!first) {
            if (coeff > 0) {
                result += " + ";
            } else {
                result += " - ";
                coeff = -coeff;
            }
        } else {
            if (coeff < 0) {
                result += "-";
                coeff = -coeff;
            }
            first = false;
        }

        // Handle coefficient
        if (coeff != 1 || exp == 0) {
            result += to_string(coeff);
        }

        // Handle variable and exponent
        if (exp > 0) {
            result += "x";
            if (exp > 1) {
                result += "^{" + to_string(exp) + "}";
            }
        }
    }

    return result;
}

/**
 * @brief Rebuild polynomial from string
 */
void Polynomial::parse_from_string(const string& input) {
    cnt_ = 0;

    if (input.empty()) {
        return;
    }

    // Remove spaces
    string clean_input = input;
    clean_input.erase(remove_if(clean_input.begin(), clean_input.end(), ::isspace), clean_input.end());

    if (clean_input.empty()) {
        return;
    }

    // Parse "c1,e1,c2,e2,..." format
    size_t start = 0;
    size_t end = clean_input.find(',');

    while (end != string::npos) {
        string coeff_str = clean_input.substr(start, end - start);

        // Find exponent
        start = end + 1;
        end = clean_input.find(',', start);

        if (end == string::npos) {
            // Last exponent
            string exp_str = clean_input.substr(start);
            try {
                int coeff = stoi(coeff_str);
                int exp = stoi(exp_str);
                add_term(Term(coeff, exp));
            } catch (...) {
                // Parse error, clear polynomial
                cnt_ = 0;
                return;
            }
            break;
        }

        string exp_str = clean_input.substr(start, end - start);
        try {
            int coeff = stoi(coeff_str);
            int exp = stoi(exp_str);
            add_term(Term(coeff, exp));
        } catch (...) {
            // Parse error, clear polynomial
            cnt_ = 0;
            return;
        }

        start = end + 1;
        end = clean_input.find(',', start);
    }
}

// ============================================================================
// PolynomialManager Class Implementation
// ============================================================================

// Static member initialization
mutex PolynomialManager::manager_mutex_;
unordered_map<char, Polynomial> PolynomialManager::polynomials_;
const int PolynomialManager::MAX_POLYNOMIALS = 5;
const char PolynomialManager::POLYNOMIAL_NAMES[] = {'a', 'b', 'c', 'd', 'e'};

/**
 * @brief Create or update polynomial
 */
int PolynomialManager::create_polynomial(char name, const string& input) {
    lock_guard<mutex> lock(manager_mutex_);

    // Check if name is valid
    if (name < 'a' || name > 'e') {
        return -1; // Invalid name
    }

    // Check if exceeding limit
    if (polynomials_.size() >= MAX_POLYNOMIALS && polynomials_.find(name) == polynomials_.end()) {
        return -3; // Too many polynomials
    }

    try {
        Polynomial poly(input);
        polynomials_[name] = poly;
        return 0; // Success
    } catch (...) {
        return -2; // Format error
    }
}

/**
 * @brief Get polynomial standard format string
 */
int PolynomialManager::get_polynomial_string(char name, string& result) {
    lock_guard<mutex> lock(manager_mutex_);

    if (name < 'a' || name > 'e') {
        return -1; // Invalid name
    }

    auto it = polynomials_.find(name);
    if (it == polynomials_.end()) {
        return -2; // Polynomial not found
    }

    result = it->second.to_standard_string();
    return 0; // Success
}

/**
 * @brief Get polynomial both standard and LaTeX format strings
 */
int PolynomialManager::get_polynomial_string_with_latex(char name, string& result) {
    lock_guard<mutex> lock(manager_mutex_);

    if (name < 'a' || name > 'e') {
        return -1; // Invalid name
    }

    auto it = polynomials_.find(name);
    if (it == polynomials_.end()) {
        return -2; // Polynomial not found
    }

    // Format: "standard|latex" - pipe separator for easy parsing
    result = it->second.to_standard_string() + "|" + it->second.to_latex_string();
    return 0; // Success
}

/**
 * @brief Parse polynomial expression and calculate result
 */
int PolynomialManager::parse_expression(const string& expr, Polynomial& result) {
    if (expr.empty()) {
        return -4; // Empty expression
    }

    // Use stack to parse expression
    stack<Polynomial> poly_stack;
    stack<char> op_stack;

    for (size_t i = 0; i < expr.length(); ++i) {
        char c = expr[i];

        if (c >= 'a' && c <= 'e') {
            // Polynomial variable
            auto it = polynomials_.find(c);
            if (it == polynomials_.end()) {
                return -5; // Polynomial not found
            }
            poly_stack.push(it->second);
        } else if (c == '+' || c == '-' || c == '*') {
            // Operator
            while (!op_stack.empty() && op_stack.top() != '(' &&
                   ((op_stack.top() == '*') || (op_stack.top() != '*' && c != '*'))) {
                if (poly_stack.size() < 2) {
                    return -6; // Expression error
                }

                char op = op_stack.top();
                op_stack.pop();

                Polynomial b = poly_stack.top();
                poly_stack.pop();
                Polynomial a = poly_stack.top();
                poly_stack.pop();

                switch (op) {
                    case '+':
                        poly_stack.push(a + b);
                        break;
                    case '-':
                        poly_stack.push(a - b);
                        break;
                    case '*':
                        poly_stack.push(a * b);
                        break;
                }
            }
            op_stack.push(c);
        } else if (c == '(') {
            op_stack.push(c);
        } else if (c == ')') {
            // Handle parentheses
            while (!op_stack.empty() && op_stack.top() != '(') {
                if (poly_stack.size() < 2) {
                    return -6; // Expression error
                }

                char op = op_stack.top();
                op_stack.pop();

                Polynomial b = poly_stack.top();
                poly_stack.pop();
                Polynomial a = poly_stack.top();
                poly_stack.pop();

                switch (op) {
                    case '+':
                        poly_stack.push(a + b);
                        break;
                    case '-':
                        poly_stack.push(a - b);
                        break;
                    case '*':
                        poly_stack.push(a * b);
                        break;
                }
            }

            if (op_stack.empty()) {
                return -7; // Parentheses mismatch
            }
            op_stack.pop(); // Pop '('
        } else if (c != ' ' && c != '\t') {
            return -8; // Invalid character
        }
    }

    // Process remaining operators
    while (!op_stack.empty()) {
        if (poly_stack.size() < 2) {
            return -6; // Expression error
        }

        char op = op_stack.top();
        op_stack.pop();

        Polynomial b = poly_stack.top();
        poly_stack.pop();
        Polynomial a = poly_stack.top();
        poly_stack.pop();

        switch (op) {
            case '+':
                poly_stack.push(a + b);
                break;
            case '-':
                poly_stack.push(a - b);
                break;
            case '*':
                poly_stack.push(a * b);
                break;
        }
    }

    if (poly_stack.size() != 1) {
        return -6; // Expression error
    }

    result = poly_stack.top();
    return 0; // Success
}

/**
 * @brief Calculate polynomial operation result
 */
int PolynomialManager::calculate_polynomials(const string& expr, string& result) {
    lock_guard<mutex> lock(manager_mutex_);

    Polynomial poly_result;
    int parse_result = parse_expression(expr, poly_result);

    if (parse_result != 0) {
        return parse_result;
    }

    result = poly_result.to_standard_string();
    return 0; // Success
}

/**
 * @brief Calculate polynomial operation result with LaTeX
 */
int PolynomialManager::calculate_polynomials_with_latex(const string& expr, string& result) {
    lock_guard<mutex> lock(manager_mutex_);

    Polynomial poly_result;
    int parse_result = parse_expression(expr, poly_result);

    if (parse_result != 0) {
        return parse_result;
    }

    // Format: "standard|latex"
    result = poly_result.to_standard_string() + "|" + poly_result.to_latex_string();
    return 0; // Success
}

/**
 * @brief Calculate polynomial value at x
 */
int PolynomialManager::evaluate_polynomial(char name, int x, int& result) {
    lock_guard<mutex> lock(manager_mutex_);

    if (name < 'a' || name > 'e') {
        return -1; // Invalid name
    }

    auto it = polynomials_.find(name);
    if (it == polynomials_.end()) {
        return -2; // Polynomial not found
    }

    result = it->second.evaluate(x);
    return 0; // Success
}

/**
 * @brief Calculate polynomial derivative
 */
int PolynomialManager::derivative_polynomial(char name, string& result) {
    lock_guard<mutex> lock(manager_mutex_);

    if (name < 'a' || name > 'e') {
        return -1; // Invalid name
    }

    auto it = polynomials_.find(name);
    if (it == polynomials_.end()) {
        return -2; // Polynomial not found
    }

    Polynomial derivative = it->second.derivative();
    result = derivative.to_standard_string();
    return 0; // Success
}

/**
 * @brief Calculate polynomial derivative with LaTeX
 */
int PolynomialManager::derivative_polynomial_with_latex(char name, string& result) {
    lock_guard<mutex> lock(manager_mutex_);

    if (name < 'a' || name > 'e') {
        return -1; // Invalid name
    }

    auto it = polynomials_.find(name);
    if (it == polynomials_.end()) {
        return -2; // Polynomial not found
    }

    Polynomial derivative = it->second.derivative();
    // Format: "standard|latex"
    result = derivative.to_standard_string() + "|" + derivative.to_latex_string();
    return 0; // Success
}

/**
 * @brief Clear all polynomials
 */
void PolynomialManager::clear_all() {
    lock_guard<mutex> lock(manager_mutex_);
    polynomials_.clear();
}

/**
 * @brief Get created polynomial name list
 */
int PolynomialManager::get_polynomial_names(vector<char>& names) {
    lock_guard<mutex> lock(manager_mutex_);

    names.clear();
    for (const auto& pair : polynomials_) {
        names.push_back(pair.first);
    }

    return static_cast<int>(names.size());
}

// ============================================================================
// C Interface Functions for FFI
// ============================================================================

/**
 * @brief C interface for get_polynomial_string_with_latex
 */
extern "C" int get_polynomial_string_with_latex(char name, char* output, int buffer_size) {
    try {
        string result;
        int code = PolynomialManager::get_polynomial_string_with_latex(name, result);
        if (code == 0 && result.length() < static_cast<size_t>(buffer_size)) {
            strcpy(output, result.c_str());
        }
        return code;
    } catch (...) {
        return -1;
    }
}

/**
 * @brief C interface for calculate_polynomials_with_latex
 */
extern "C" int calculate_polynomials_with_latex(const char* expression, char* output, int buffer_size) {
    try {
        string result;
        int code = PolynomialManager::calculate_polynomials_with_latex(string(expression), result);
        if (code == 0 && result.length() < static_cast<size_t>(buffer_size)) {
            strcpy(output, result.c_str());
        }
        return code;
    } catch (...) {
        return -1;
    }
}

/**
 * @brief C interface for derivative_polynomial_with_latex
 */
extern "C" int derivative_polynomial_with_latex(char name, char* output, int buffer_size) {
    try {
        string result;
        int code = PolynomialManager::derivative_polynomial_with_latex(name, result);
        if (code == 0 && result.length() < static_cast<size_t>(buffer_size)) {
            strcpy(output, result.c_str());
        }
        return code;
    } catch (...) {
        return -1;
    }
}