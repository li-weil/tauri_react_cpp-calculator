#include "polynomial.hpp"
#include "polynomial.cpp"
// Polynomial Calculator - C Interface Implementation
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

using namespace std;

// ============================================================================
// Error Codes
// ============================================================================
static constexpr int ERROR_SUCCESS = 0;
static constexpr int ERROR_INVALID_NAME = -1;
static constexpr int ERROR_POLYNOMIAL_NOT_FOUND = -2;
static constexpr int ERROR_INVALID_INPUT = -3;
static constexpr int ERROR_TOO_MANY_POLYNOMIALS = -4;
static constexpr int ERROR_EMPTY_EXPRESSION = -5;
static constexpr int ERROR_EXPRESSION_PARSE_ERROR = -6;
static constexpr int ERROR_INVALID_EXPRESSION = -7;
static constexpr int ERROR_PARENTHESES_MISMATCH = -8;
static constexpr int ERROR_INVALID_CHARACTER = -9;

// ============================================================================
// Helper Functions
// ============================================================================
static string copy_string_to_c_str(const string& str);
static bool is_valid_polynomial_name(char name);
static bool is_valid_operator(char op);
static int get_operator_precedence(char op);

// ============================================================================
// Helper Function Implementation
// ============================================================================

/**
 * @brief Safely copy string to C-style string buffer
 * @param str input string
 * @return copied string
 */
static string copy_string_to_c_str(const string& str) {
    return str;
}

/**
 * @brief Check if polynomial name is valid
 * @param name polynomial name
 * @return true: valid, false: invalid
 */
static bool is_valid_polynomial_name(char name) {
    return name >= 'a' && name <= 'e';
}

/**
 * @brief Check if operator is valid
 * @param op operator
 * @return true: valid, false: invalid
 */
static bool is_valid_operator(char op) {
    return op == '+' || op == '-' || op == '*' || op == '(' || op == ')';
}

/**
 * @brief Get operator precedence
 * @param op operator
 * @return precedence level (higher number = higher precedence)
 */
static int get_operator_precedence(char op) {
    switch (op) {
        case '*':
            return 2;
        case '+':
        case '-':
            return 1;
        case '(':
        case ')':
            return 0;
        default:
            return -1;
    }
}

// ============================================================================
// C Interface Functions
// ============================================================================

extern "C" {

/**
 * @brief Create polynomial
 * @param name polynomial name ('a'-'e')
 * @param input input string format: "c1,e1,c2,e2,..."
 * @return 0: success, other: error code
 */
int create_polynomial(char name, const char* input) {
    if (!is_valid_polynomial_name(name)) {
        return ERROR_INVALID_NAME;
    }

    if (!input) {
        return ERROR_INVALID_INPUT;
    }

    string input_str(input);
    return PolynomialManager::create_polynomial(name, input_str);
}

/**
 * @brief Get polynomial string representation
 * @param name polynomial name
 * @param output output buffer
 * @param buffer_size buffer size
 * @return 0: success, other: error code
 */
int get_polynomial_to_string(char name, char* output, int buffer_size) {
    if (!is_valid_polynomial_name(name)) {
        return ERROR_INVALID_NAME;
    }

    if (!output || buffer_size <= 0) {
        return ERROR_INVALID_INPUT;
    }

    string result;
    int ret = PolynomialManager::get_polynomial_string(name, result);

    if (ret == ERROR_SUCCESS) {
        // Check buffer size
        if (result.length() >= static_cast<size_t>(buffer_size)) {
            return ERROR_INVALID_INPUT;
        }
        strcpy(output, result.c_str());
    }

    return ret;
}

/**
 * @brief Calculate polynomial expression
 * @param expression polynomial expression (e.g. "a+b", "a-b*c")
 * @param output output buffer
 * @param buffer_size buffer size
 * @return 0: success, other: error code
 */
int calculate_polynomials(const char* expression, char* output, int buffer_size) {
    if (!expression) {
        return ERROR_EMPTY_EXPRESSION;
    }

    if (!output || buffer_size <= 0) {
        return ERROR_INVALID_INPUT;
    }

    string expr_str(expression);

    // Remove spaces from expression
    expr_str.erase(remove_if(expr_str.begin(), expr_str.end(), ::isspace), expr_str.end());

    if (expr_str.empty()) {
        return ERROR_EMPTY_EXPRESSION;
    }

    // Validate expression characters
    for (char c : expr_str) {
        if (!is_valid_polynomial_name(c) && !is_valid_operator(c)) {
            return ERROR_INVALID_CHARACTER;
        }
    }

    string result;
    int ret = PolynomialManager::calculate_polynomials(expr_str, result);

    if (ret == ERROR_SUCCESS) {
        // Check buffer size
        if (result.length() >= static_cast<size_t>(buffer_size)) {
            return ERROR_INVALID_INPUT;
        }
        strcpy(output, result.c_str());
    }

    return ret;
}

/**
 * @brief Calculate polynomial value at x
 * @param name polynomial name
 * @param x input value
 * @param result output result
 * @return 0: success, other: error code
 */
int evaluate_polynomial(char name, int x, int* result) {
    if (!is_valid_polynomial_name(name)) {
        return ERROR_INVALID_NAME;
    }

    if (!result) {
        return ERROR_INVALID_INPUT;
    }

    return PolynomialManager::evaluate_polynomial(name, x, *result);
}

/**
 * @brief Calculate polynomial derivative
 * @param name polynomial name
 * @param output output buffer
 * @param buffer_size buffer size
 * @return 0: success, other: error code
 */
int derivative_polynomial(char name, char* output, int buffer_size) {
    if (!is_valid_polynomial_name(name)) {
        return ERROR_INVALID_NAME;
    }

    if (!output || buffer_size <= 0) {
        return ERROR_INVALID_INPUT;
    }

    string result;
    int ret = PolynomialManager::derivative_polynomial(name, result);

    if (ret == ERROR_SUCCESS) {
        // Check buffer size
        if (result.length() >= static_cast<size_t>(buffer_size)) {
            return ERROR_INVALID_INPUT;
        }
        strcpy(output, result.c_str());
    }

    return ret;
}

/**
 * @brief Clear all polynomials
 * @return 0: success
 */
int clear_all_polynomials() {
    PolynomialManager::clear_all();
    return ERROR_SUCCESS;
}

/**
 * @brief Get all created polynomial name list
 * @param names name buffer
 * @param max_count maximum name count
 * @return actual name count
 */
int get_polynomial_names(char* names, int max_count) {
    if (!names || max_count <= 0) {
        return ERROR_INVALID_INPUT;
    }

    vector<char> name_list;
    int count = PolynomialManager::get_polynomial_names(name_list);

    if (count > max_count) {
        count = max_count;
    }

    for (int i = 0; i < count; ++i) {
        names[i] = name_list[i];
    }

    return count;
}

/**
 * @brief Check if polynomial exists
 * @param name polynomial name
 * @return 1: exists, 0: not exists, other: error code
 */
int polynomial_exists(char name) {
    if (!is_valid_polynomial_name(name)) {
        return ERROR_INVALID_NAME;
    }

    string dummy;
    int ret = PolynomialManager::get_polynomial_string(name, dummy);
    return (ret == ERROR_SUCCESS) ? 1 : 0;
}

/**
 * @brief Get polynomial term count
 * @param name polynomial name
 * @param count term count
 * @return 0: success, other: error code
 */
int get_polynomial_term_count(char name, int* count) {
    if (!is_valid_polynomial_name(name)) {
        return ERROR_INVALID_NAME;
    }

    if (!count) {
        return ERROR_INVALID_INPUT;
    }

    string result;
    int ret = PolynomialManager::get_polynomial_string(name, result);

    if (ret == ERROR_SUCCESS) {
        // Parse term count from standard format string
        if (result == "0") {
            *count = 0;
        } else {
            // Find first comma to get term count
            size_t first_comma = result.find(',');
            if (first_comma != string::npos) {
                string count_str = result.substr(0, first_comma);
                *count = stoi(count_str);
            } else {
                *count = 0;
            }
        }
    }

    return ret;
}

/**
 * @brief Get error description
 * @param error_code error code
 * @return error description string
 */
const char* get_polynomial_error_description(int error_code) {
    switch (error_code) {
        case ERROR_SUCCESS:
            return "Success";
        case ERROR_INVALID_NAME:
            return "Invalid polynomial name (must be 'a'-'e')";
        case ERROR_POLYNOMIAL_NOT_FOUND:
            return "Polynomial not found";
        case ERROR_INVALID_INPUT:
            return "Invalid input";
        case ERROR_TOO_MANY_POLYNOMIALS:
            return "Too many polynomials (maximum 5)";
        case ERROR_EMPTY_EXPRESSION:
            return "Empty expression";
        case ERROR_EXPRESSION_PARSE_ERROR:
            return "Expression parse error";
        case ERROR_INVALID_EXPRESSION:
            return "Invalid expression";
        case ERROR_PARENTHESES_MISMATCH:
            return "Parentheses mismatch";
        case ERROR_INVALID_CHARACTER:
            return "Invalid character in expression";
        default:
            return "Unknown error";
    }
}

} // extern "C"