#include "stack.hpp"

// 标准库头文件 - 按字母顺序排列
#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

using namespace std;


static constexpr int MAX_OPERATIONS = 10000;
static constexpr char OPERATION_PUSH = 1;
static constexpr char OPERATION_POP = 0;

// 错误代码定义
static constexpr int ERROR_SUCCESS = 0;
static constexpr int ERROR_STACK_NOT_INITIALIZED = -1;
static constexpr int ERROR_EMPTY_INPUT = -2;
static constexpr int ERROR_DIVISION_BY_ZERO = -3;
static constexpr int ERROR_UNKNOWN_OPERATOR = -4;
static constexpr int ERROR_INVALID_EXPRESSION = -5;
static constexpr int ERROR_NO_RESULT = -6;
static constexpr int ERROR_PARENTHESIS_MISMATCH = -7;


static mutex stack_mutex;                                                      // 线程安全互斥锁
static unique_ptr<Stack<int>> stack_num;                                       // 数字栈
static unique_ptr<Stack<char>> stack_sym;                                      // 符号栈
static pair<pair<int, int>, int> stack_num_operation[MAX_OPERATIONS];           // 数字栈操作记录
static pair<pair<int, char>, int> stack_sym_operation[MAX_OPERATIONS];          // 符号栈操作记录
static int stack_num_op_cnt = 0;                                               // 数字栈操作计数
static int stack_sym_op_cnt = 0;                                               // 符号栈操作计数
static int abs_cnt = 0;                                                        // 绝对值计数器
static int time_stamp = 0;                                                      // 全局时间戳

// ============================================================================
// 辅助函数声明和实现
// ============================================================================
static bool should_operator_execute(char stack_top, char current_input);
static pair<int, string> perform_calculation(char operation, int operand_a, int operand_b);
static void record_num_operation(char op_type, int value);
static void record_sym_operation(char op_type, char symbol);
static void reset_calculation_state();
static bool is_digit(char c);
static int parse_number(const string& s, int start_pos, int& end_pos);



/**
 * @brief 判断是否应该执行栈顶操作符
 * @param stack_top 栈顶操作符
 * @param current_input 当前输入操作符
 * @return true: 应该执行, false: 不应该执行
 */
static bool should_operator_execute(char stack_top, char current_input) {
    return (stack_top == '+' && (current_input == '-' || current_input == '+' || current_input == ')' || current_input == '|')) ||
           (stack_top == '-' && (current_input == '-' || current_input == '+' || current_input == ')' || current_input == '|')) ||
           (stack_top == '*' && (current_input == '/' || current_input == '*' || current_input == '+' || current_input == '-' || current_input == ')' || current_input == '|')) ||
           (stack_top == '/' && (current_input == '/' || current_input == '*' || current_input == '+' || current_input == '-' || current_input == ')' || current_input == '|')) ||
           (stack_top == '^' && (current_input == '+' || current_input == '-' || current_input == '*' || current_input == '/' || current_input == '^' || current_input == ')' || current_input == '|')) ||
           (stack_top == '(' && current_input == ')') ||
           (stack_top == '|' && current_input == '|');
}

/**
 * @brief 执行数学运算
 * @param operation 操作符
 * @param operand_a 操作数A
 * @param operand_b 操作数B
 * @return pair<结果, 错误信息>
 */
static pair<int, string> perform_calculation(char operation, int operand_a, int operand_b) {
    switch (operation) {
        case '+':
            return make_pair(operand_b + operand_a, "");
        case '-':
            return make_pair(operand_b - operand_a, "");
        case '*':
            return make_pair(operand_b * operand_a, "");
        case '/':
            if (operand_a == 0) {
                return make_pair(ERROR_DIVISION_BY_ZERO, "Division by zero");
            }
            return make_pair(operand_b / operand_a, "");
        case '^': {
            int result = 1;
            for (int i = 0; i < operand_a; i++) {
                result *= operand_b;
            }
            return make_pair(result, "");
        }
        default:
            return make_pair(ERROR_UNKNOWN_OPERATOR, "Unknown operator");
    }
}

/**
 * @brief 记录数字栈操作
 * @param op_type 操作类型 (0: 出栈, 1: 入栈)
 * @param value 数值
 */
static void record_num_operation(char op_type, int value) {
    if (stack_num_op_cnt < MAX_OPERATIONS) {
        stack_num_operation[stack_num_op_cnt++] = make_pair(make_pair(op_type, value), time_stamp++);
    }
}

/**
 * @brief 记录符号栈操作
 * @param op_type 操作类型 (0: 出栈, 1: 入栈)
 * @param symbol 符号
 */
static void record_sym_operation(char op_type, char symbol) {
    if (stack_sym_op_cnt < MAX_OPERATIONS) {
        stack_sym_operation[stack_sym_op_cnt++] = make_pair(make_pair(op_type, symbol), time_stamp++);
    }
}

/**
 * @brief 重置计算状态
 */
static void reset_calculation_state() {
    stack_num->clear();
    stack_sym->clear();
    stack_num_op_cnt = 0;
    stack_sym_op_cnt = 0;
    abs_cnt = 0;
    time_stamp = 0;
}

/**
 * @brief 判断字符是否为数字
 * @param c 字符
 * @return true: 是数字, false: 不是数字
 */
static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

/**
 * @brief 解析数字
 * @param s 字符串
 * @param start_pos 开始位置
 * @param end_pos 结束位置 (输出参数)
 * @return 解析到的数字
 */
static int parse_number(const string& s, int start_pos, int& end_pos) {
    end_pos = start_pos;
    while (end_pos < s.length() && is_digit(s[end_pos])) {
        end_pos++;
    }
    return stoi(s.substr(start_pos, end_pos - start_pos));
}

/**
 * @brief 输出操作记录信息 (调试用)
 */
static void output_operation_info() {
    cout << "=== Symbol Stack Operations ===" << endl;
    for (int i = 0; i < stack_sym_op_cnt; i++) {
        cout << "Type: " << stack_sym_operation[i].first.first
             << ", Symbol: " << stack_sym_operation[i].first.second
             << ", Timestamp: " << stack_sym_operation[i].second << endl;
    }

    cout << "=== Number Stack Operations ===" << endl;
    for (int i = 0; i < stack_num_op_cnt; i++) {
        cout << "Type: " << stack_num_operation[i].first.first
             << ", Value: " << stack_num_operation[i].first.second
             << ", Timestamp: " << stack_num_operation[i].second << endl;
    }
}

// ============================================================================
// C接口实现
// ============================================================================

extern "C" {

/**
 * @brief 初始化栈
 * @param capacity 栈容量
 * @return 0: 成功, -1: 失败
 */
int init_stack(int capacity) {
    lock_guard<mutex> lock(stack_mutex);
    try {
        stack_num = make_unique<Stack<int>>(capacity);
        stack_sym = make_unique<Stack<char>>(capacity);
        return ERROR_SUCCESS;
    } catch (...) {
        return ERROR_STACK_NOT_INITIALIZED;
    }
}

/**
 * @brief 计算表达式
 * @param input 输入表达式
 * @return 计算结果或错误代码
 */
int calculation(const char* input) {
    lock_guard<mutex> lock(stack_mutex);

    // 输入验证
    if (!stack_num || !stack_sym) {
        return ERROR_STACK_NOT_INITIALIZED;
    }

    if (!input) {
        return ERROR_EMPTY_INPUT;
    }

    // 预处理输入
    string expression = input;
    expression.erase(remove_if(expression.begin(), expression.end(), ::isspace), expression.end());

    for(int i = 0; i < expression.length(); i++) {
        if(expression[i] <'0'&& expression[i]>'9' &&
           expression[i] != '+' && expression[i] != '-' &&
           expression[i] != '*' && expression[i] != '/' &&
           expression[i] != '^' && expression[i] != '(' &&
           expression[i] != ')' && expression[i] != '|') {
            return ERROR_INVALID_EXPRESSION;
        }
    }

    reset_calculation_state();

    // 处理表达式
    for (size_t i = 0; i < expression.length(); i++) {
        char current_char = expression[i];

        // 处理数字
        if (is_digit(current_char)) {
            int end_pos;
            int number = parse_number(expression, i, end_pos);
            stack_num->push(number);
            record_num_operation(OPERATION_PUSH, number);
            i = end_pos - 1;
            continue;
        }

        // 处理操作符
        bool matched = false;
        while (!stack_sym->empty() && should_operator_execute(stack_sym->top(), current_char)) {
            char top_symbol = stack_sym->top();

            // 处理绝对值
            if (!abs_cnt && current_char == '|') {
                abs_cnt = 1;
                break;
            }

            // 处理括号匹配
            if (top_symbol == '(' && current_char == ')') {
                stack_sym->pop();
                record_sym_operation(OPERATION_POP, top_symbol);
                matched = true;
                break;
            }

            // 检查括号不匹配
            if ((top_symbol == '(' && current_char == '|') ||
                (top_symbol == ')' && current_char == '|') ||
                (top_symbol == '|' && current_char == ')')) {
                return ERROR_PARENTHESIS_MISMATCH;
            }

            // 处理绝对值运算
            if (top_symbol == '|' && current_char == '|') {
                matched = true;
                abs_cnt = 0;
                int value = stack_num->pop();
                if (value < 0) value = -value;
                stack_num->push(value);
                record_num_operation(OPERATION_POP, 0);  // 弹出原值
                record_num_operation(OPERATION_PUSH, value);  // 推入绝对值
                stack_sym->pop();
                record_sym_operation(OPERATION_POP, '|');
                break;
            }

            // 执行数学运算
            if (stack_num->size() < 2) {
                output_operation_info();
                return ERROR_INVALID_EXPRESSION;
            }

            int operand_a = stack_num->pop();
            int operand_b = stack_num->pop();

            record_num_operation(OPERATION_POP, operand_a);
            record_num_operation(OPERATION_POP, operand_b);

            auto calc_result = perform_calculation(top_symbol, operand_a, operand_b);
            if (!calc_result.second.empty()) {
                output_operation_info();
                return calc_result.first;
            }

            stack_num->push(calc_result.first);
            record_num_operation(OPERATION_PUSH, calc_result.first);

            stack_sym->pop();
            record_sym_operation(OPERATION_POP, top_symbol);
        }

        // 如果没有匹配，将当前操作符入栈
        if (!matched) {
            if (current_char == '|' && !abs_cnt) {
                abs_cnt = 1;
            }
            stack_sym->push(current_char);
            record_sym_operation(OPERATION_PUSH, current_char);
        }
    }

    // 完成剩余计算
    while (!stack_sym->empty()) {
        if (stack_num->size() < 2) {
            output_operation_info();
            return ERROR_INVALID_EXPRESSION;
        }

        char top_symbol = stack_sym->top();
        int operand_a = stack_num->pop();
        int operand_b = stack_num->pop();

        record_num_operation(OPERATION_POP, operand_a);
        record_num_operation(OPERATION_POP, operand_b);

        auto calc_result = perform_calculation(top_symbol, operand_a, operand_b);
        if (!calc_result.second.empty()) {
            output_operation_info();
            return calc_result.first;
        }

        stack_num->push(calc_result.first);
        record_num_operation(OPERATION_PUSH, calc_result.first);

        stack_sym->pop();
        record_sym_operation(OPERATION_POP, top_symbol);
    }

    // 检查最终结果
    if (stack_num->empty()) {
        return ERROR_NO_RESULT;
    }

    output_operation_info();
    return stack_num->pop();
}

/**
 * @brief 获取数字栈操作数量
 * @return 操作数量
 */
int get_num_operations_count() {
    return stack_num_op_cnt;
}

/**
 * @brief 获取符号栈操作数量
 * @return 操作数量
 */
int get_sym_operations_count() {
    return stack_sym_op_cnt;
}

/**
 * @brief 获取指定索引的数字栈操作
 * @param index 索引
 * @param op_type 操作类型 (输出参数)
 * @param value 数值 (输出参数)
 * @param timestamp 时间戳 (输出参数)
 */
void get_num_operation_at(int index, int* op_type, int* value, int* timestamp) {
    if (index >= 0 && index < stack_num_op_cnt) {
        const auto& operation = stack_num_operation[index];
        *op_type = operation.first.first;
        *value = operation.first.second;
        *timestamp = operation.second;
    } else {
        *op_type = 0;
        *value = 0;
        *timestamp = 0;
    }
}

/**
 * @brief 获取指定索引的符号栈操作
 * @param index 索引
 * @param op_type 操作类型 (输出参数)
 * @param symbol 符号 (输出参数)
 * @param timestamp 时间戳 (输出参数)
 */
void get_sym_operation_at(int index, int* op_type, int* symbol, int* timestamp) {
    if (index >= 0 && index < stack_sym_op_cnt) {
        const auto& operation = stack_sym_operation[index];
        *op_type = operation.first.first;
        *symbol = operation.first.second;
        *timestamp = operation.second;
    } else {
        *op_type = 0;
        *symbol = 0;
        *timestamp = 0;
    }
}

} // extern "C"