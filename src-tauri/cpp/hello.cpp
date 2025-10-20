#include "stack.hpp"
#include <string>
#include <memory>
#include <mutex>
#include <cstring>
#include <iostream>

// 全局对象 - 线程安全
static std::mutex stack_mutex;
static std::unique_ptr<Stack<int>> stack_num;
static std::unique_ptr<Stack<char>> stack_sym;

// C 接口,可以使用c++特性，但是接口必须是C风格的
namespace{
    int priority_judge(char a,char b){
        if(a=='+'&&b=='-'||a=='-'&&b=='+'||a=='+'&&b=='+'||a=='-'&&b=='-'
            ||a=='*'&&b=='/'||a=='/'&&b=='*'||a=='*'&&b=='*'||a=='/'&&b=='/'
            ||a=='*'&&b=='+'||a=='*'&&b=='-'||a=='/'&&b=='+'||a=='/'&&b=='-'
            ||a=='^'&&b=='+'||a=='^'&&b=='-'||a=='^'&&b=='*'||a=='^'&&b=='/'||a=='^'&&b=='^'
            ||a=='-'&&b==')'||a=='+'&&b==')'||a=='*'&&b==')'||a=='/'&&b==')'||a=='('&&b==')'||a=='^'&&b==')')
                return 1;
        return 0;
    }    

}

extern "C" {
    // 初始化栈 - 使用 C++ 智能指针
    int init_stack(int capacity) {
        std::lock_guard<std::mutex> lock(stack_mutex);
        try {
            stack_num = std::make_unique<Stack<int>>(capacity); 
            stack_sym = std::make_unique<Stack<char>>(capacity);      
            return 0;  // 成功
        } catch (...) {
            return -1; // 失败
        }
    }

    int calculation(const char* input){
        std::lock_guard<std::mutex> lock(stack_mutex);

        // 检查栈是否已初始化
        if (!stack_num || !stack_sym) {
            return -1;  // 栈未初始化
        }

        if (!input) {
            return -2;  // 输入为空
        }
        std::string s = input;
        s.erase( std::remove_if( s.begin(), s.end(), ::isspace ), s.end() ); //去除空格
        int len=s.length();
        int result=0;

        // 清空栈，为新的计算做准备
        stack_num->clear();
        stack_sym->clear();
        if(s[0]=='-'){   //处理单目减法
            stack_sym->push('0');
        }
        for(int i=0;i<len;i++){
            if(s[i]>='0'&&s[i]<='9'){
                int j=i;
                while(j<len&&(s[j]>='0'&&s[j]<='9'))j++;
                stack_num->push(std::stoi(s.substr(i,j-i)));
                i=j-1;
                continue;
            }
            int match=0;
            while(stack_sym->size()&&priority_judge(stack_sym->top(),s[i])){
                if(stack_sym->top()=='('){
                    stack_sym->pop();
                    match=1;
                    break;
                }
                if(stack_num->size()<2){
                    return -5;  // 表达式错误，数字不足
                }
                int a=stack_num->pop(),b=stack_num->pop();
                // 计算结果，注意除零检查
                int calc_result = 0;
                switch(stack_sym->top()){
                    case '+':calc_result = b + a; break;
                    case '-':calc_result = b - a; break;
                    case '*':calc_result = b * a; break;
                    case '/':
                        if (a == 0) {
                            return -3;  // 除零错误
                        }
                        calc_result = b / a;
                        break;
                    case '^':
                        calc_result = 1;
                        for(int k=0;k<a;k++)calc_result*=b;
                        break;
                    default:
                        return -4;  // 未知运算符
                }
                stack_num->push(calc_result);
                stack_sym->pop();
            }
            if(s[i]!=')')stack_sym->push(s[i]);
            else if(!match)return -5;
        }
        // 完成剩余计算
        while(!stack_sym->empty()){
            // 检查数字栈是否有足够的元素
            if (stack_num->size() < 2) {
                return -5;  // 表达式错误，数字不足
            }

            int a=stack_num->pop(),b=stack_num->pop();
            int calc_result = 0;
            switch(stack_sym->top()){
                case '+':calc_result = b + a; break;
                case '-':calc_result = b - a; break;
                case '*':calc_result = b * a; break;
                case '/':
                    if (a == 0) {
                        return -3;  // 除零错误
                    }
                    calc_result = b / a;
                    break;
                case '^':    //注意别忘了break
                    calc_result = 1;
                    for(int k=0;k<a;k++)calc_result*=b;
                    break;
                default:
                    return -4;  // 未知运算符
            }
            stack_num->push(calc_result);
            stack_sym->pop();
        }

        // 检查最终结果
        if (stack_num->empty()) {
            return -6;  // 没有结果
        }

        return stack_num->pop();   
    }

}

