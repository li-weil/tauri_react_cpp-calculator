#include "polynomial.hpp"
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
// 辅助函数实现
// ============================================================================
static bool is_valid_polynomial_name(char name);
static bool is_valid_operator(char op);
static int get_operator_precedence(char op);


// 检查多项式名称是否合法
static bool is_valid_polynomial_name(char name) {
    return name >= 'a' && name <= 'e';
}

// 检查运算符是否合法
static bool is_valid_operator(char op) {
    return op == '+' || op == '-' || op == '*' || op == '(' || op == ')';
}

// 获取运算符优先级
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
// C 接口实现
// ============================================================================

extern "C" {

/**
 * @brief 多项式构建
 * @param name 多项式名称 ('a'-'e')
 * @param input 用户输入 format: "c1,e1,c2,e2,..."
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
 * @brief 得到标准输出字符串
 * @param name 多项式名称
 * @param output 指针输出
 * @param buffer_size  缓冲区大小
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
        if (result.length() >= static_cast<size_t>(buffer_size)) {
            return ERROR_INVALID_INPUT;
        }
        strcpy(output, result.c_str());
    }

    return ret;
}

/**
 * @brief 用户输入多项式算数表达式计算
 * @param expression 多项式表达式(e.g. "a+b", "a-b*c")
 * @param output 指针输出
 * @param buffer_size 缓冲区大小
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

    // 去除空格
    expr_str.erase(remove_if(expr_str.begin(), expr_str.end(), ::isspace), expr_str.end());

    if (expr_str.empty()) {
        return ERROR_EMPTY_EXPRESSION;
    }

    // 非法字符判断
    for (char c : expr_str) {
        if (!is_valid_polynomial_name(c) && !is_valid_operator(c)) {
            return ERROR_INVALID_CHARACTER;
        }
    }

    string result;
    int ret = PolynomialManager::calculate_polynomials(expr_str, result);

    if (ret == ERROR_SUCCESS) {
        if (result.length() >= static_cast<size_t>(buffer_size)) {
            return ERROR_INVALID_INPUT;
        }
        strcpy(output, result.c_str());
    }

    return ret;
}

/**
 * @brief 计算多项式在x值
 * @param name 多项式名称
 * @param x 
 * @param result 指针输出
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
 * @brief 计算多项式导数
 * @param name 多项式名称
 * @param output 指针输出
 * @param buffer_size 缓冲区大小
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


int clear_all_polynomials() {
    PolynomialManager::clear_all();
    return ERROR_SUCCESS;
}

/**
 * @brief 获取多项式名称
 * @param names 指针输出
 * @param max_count 
 * @return 多项式数量
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

int polynomial_exists(char name) {
    if (!is_valid_polynomial_name(name)) {
        return ERROR_INVALID_NAME;
    }

    string dummy;
    int ret = PolynomialManager::get_polynomial_string(name, dummy);
    return (ret == ERROR_SUCCESS) ? 1 : 0;
}

// 获取多项式项数
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
        if (result == "0") {
            *count = 0;
        } else {
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

// 错误信息
const char* get_polynomial_error_description(int error_code) {
    switch (error_code) {
        case ERROR_SUCCESS:
            return "Success";
        case ERROR_INVALID_NAME:
            return "多项式名称错误(must be 'a'-'e')";
        case ERROR_POLYNOMIAL_NOT_FOUND:
            return "多项式不存在";
        case ERROR_INVALID_INPUT:
            return "非法输入";
        case ERROR_TOO_MANY_POLYNOMIALS:
            return "数量超过上限 (maximum 5)";
        case ERROR_EMPTY_EXPRESSION:
            return "多项式为空";
        case ERROR_EXPRESSION_PARSE_ERROR:
            return "多项式解析错误";
        case ERROR_INVALID_EXPRESSION:
            return "非法表达式";
        case ERROR_PARENTHESES_MISMATCH:
            return "括号不匹配";
        case ERROR_INVALID_CHARACTER:
            return "非法字符";
        default:
            return "未知错误";
    }
}

} // extern "C"