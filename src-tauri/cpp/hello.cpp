#include "stack.hpp"
#include <string>
#include <memory>
#include <mutex>
#include <cstring>
#include <iostream>
#include <utility>
#include <algorithm>
using namespace std;
// 全局对象 - 线程安全
static std::mutex stack_mutex;
static std::unique_ptr<Stack<int>> stack_num;
static std::unique_ptr<Stack<char>> stack_sym;
static pair<pair<int,int>,int> stack_num_operation[10000];  //记录出入栈操作
static pair<pair<int,char>,int> stack_sym_operation[10000];
static int stack_num_op_cnt=0;
static int stack_sym_op_cnt=0;
static int abs_cnt=0;
static int time_stamp=0;
// C 接口,可以使用c++特性，但是接口必须是C风格的

int priority_judge(char a,char b){
    cout<<"priority_judge: "<<a<<" "<<b<<endl;
    if(a=='+'&&b=='-'||a=='-'&&b=='+'||a=='+'&&b=='+'||a=='-'&&b=='-'
        ||a=='*'&&b=='/'||a=='/'&&b=='*'||a=='*'&&b=='*'||a=='/'&&b=='/'
        ||a=='*'&&b=='+'||a=='*'&&b=='-'||a=='/'&&b=='+'||a=='/'&&b=='-'
        ||a=='^'&&b=='+'||a=='^'&&b=='-'||a=='^'&&b=='*'||a=='^'&&b=='/'||a=='^'&&b=='^'
        ||a=='-'&&b==')'||a=='+'&&b==')'||a=='*'&&b==')'||a=='/'&&b==')'||a=='('&&b==')'||a=='^'&&b==')'
        ||a=='-'&&b=='|'||a=='+'&&b=='|'||a=='*'&&b=='|'||a=='/'&&b=='|'||a=='|'&&b=='|'||a=='^'&&b=='|')
            return 1;
    return 0;
}    
void output_info(){
    for(int i=0;i<stack_sym_op_cnt;i++){
        cout<<stack_sym_operation[i].first.first<<" "<<stack_sym_operation[i].first.second<<" "<<stack_sym_operation[i].second<<endl;
    }
    for(int i=0;i<stack_num_op_cnt;i++){
        cout<<stack_num_operation[i].first.first<<" "<<stack_num_operation[i].first.second<<" "<<stack_num_operation[i].second<<endl;
    }        
}    
pair<int,string> calc_sym(char top_sym,int a,int b){
    int calc_result=0;
    switch(top_sym){
        case '+':return(make_pair(b + a, "")); break;
        case '-':return(make_pair(b - a, "")); break;
        case '*':return(make_pair(b * a, "")); break;
        case '/':
            if (a == 0) {
                cout<<"--------------0-------------"<<endl;
                output_info();
                return make_pair(-3, "Division by zero");
            }
            return make_pair(b / a, "");
            break;
        case '^':
            calc_result = 1;
            for(int k=0;k<a;k++)calc_result*=b;
            return make_pair(calc_result, "");
            break;
        default:
            cout<<"--------------unknown operator-------------"<<endl;
            output_info();
            return make_pair(-4, "Unknown operator");
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
        stack_num_op_cnt=0;
        stack_sym_op_cnt=0;
        abs_cnt=0;
        time_stamp=0;
        // if(s[0]=='-'){   //处理单目减法
        //     stack_sym->push('0');
        // }
        char top_sym;
        for(int i=0;i<len;i++){
            if(s[i]>='0'&&s[i]<='9'){
                int j=i;
                while(j<len&&(s[j]>='0'&&s[j]<='9'))j++;
                stack_num->push(stoi(s.substr(i,j-i)));
                stack_num_operation[stack_num_op_cnt++]=make_pair(make_pair(1,stoi(s.substr(i,j-i))),time_stamp++);
                i=j-1;
                continue;
            }
            int match=0;
            while(stack_sym->size()&&priority_judge(top_sym=stack_sym->top(),s[i])){
                // cout<<abs_cnt<<" "<<s[i]<<" "<< top_sym <<endl;
                if(!abs_cnt&&s[i]=='|'){
                    abs_cnt=1;
                    break;
                }
                // cout<<"top_sym: "<<top_sym<<endl;
                if(top_sym=='('&&s[i]==')'){
                    match=1;
                    stack_sym->pop();
                    stack_sym_operation[stack_sym_op_cnt++]=make_pair(make_pair(0,top_sym),time_stamp++);
                    break;
                }
                if(top_sym=='('&&s[i]=='|'||top_sym==')'&&s[i]=='|'
                    ||top_sym=='|'&&s[i]==')'){
                    return -7;  // 表达式错误，括号不匹配
                }
                if(top_sym=='|'&&s[i]=='|'){
                    match=1;
                    abs_cnt=0;
                    int val=stack_num->pop();
                    if(val<0)val=-val;
                    stack_num->push(val);
                    stack_sym->pop();
                    stack_sym_operation[stack_sym_op_cnt++]=make_pair(make_pair(0,'|'),time_stamp++);
                    break;
                }
                if(stack_num->size()<2){
                    cout<<"---------------------------"<<endl;
                    output_info();
                    return -5;  // 表达式错误，数字不足
                }
                int a=stack_num->pop(),b=stack_num->pop();
                stack_num_operation[stack_num_op_cnt++]=make_pair(make_pair(0,a),time_stamp++);
                stack_num_operation[stack_num_op_cnt++]=make_pair(make_pair(0,b),time_stamp++);
                stack_sym->pop();
                stack_sym_operation[stack_sym_op_cnt++]=make_pair(make_pair(0,top_sym),time_stamp++);                
                // 计算结果，注意除零检查
                pair<int,string> calc_res = calc_sym(top_sym, a, b);
                if (!calc_res.second.empty())return calc_res.first; // 返回错误代码
                stack_num->push(calc_res.first);
                stack_num_operation[stack_num_op_cnt++]=make_pair(make_pair(1,calc_res.first),time_stamp++);

                // cout<<top_sym<<" "<<stack_sym->size()<<" "<<endl;
            }
            if(!match){
                if(s[i]=='|'&&!abs_cnt){
                    abs_cnt=1;
                }
                stack_sym->push(s[i]);
                stack_sym_operation[stack_sym_op_cnt++]=make_pair(make_pair(1,s[i]),time_stamp++);
            }
        }
        // 完成剩余计算
        while(!stack_sym->empty()){
            // 检查数字栈是否有足够的元素
            top_sym=stack_sym->top();
            if (stack_num->size() < 2) {
                cout<<"---------------------------"<<endl;
                output_info();
                return -5;  // 表达式错误，数字不足
            }
            int a=stack_num->pop(),b=stack_num->pop();
            stack_num_operation[stack_num_op_cnt++]=make_pair(make_pair(0,a),time_stamp++);
            stack_num_operation[stack_num_op_cnt++]=make_pair(make_pair(0,b),time_stamp++);
            stack_sym->pop();
            stack_sym_operation[stack_sym_op_cnt++]=make_pair(make_pair(0,top_sym),time_stamp++);
            pair<int,string> calc_res = calc_sym(top_sym, a, b);
            if (!calc_res.second.empty())return calc_res.first; // 返回错误代码
            stack_num->push(calc_res.first);
            stack_num_operation[stack_num_op_cnt++]=make_pair(make_pair(1,calc_res.first),time_stamp++);
           
            
        }

        // 检查最终结果
        if (stack_num->empty()) {
            return -6;  // 没有结果
        }
        output_info();
        return stack_num->pop();
    }

    // 获取数字栈操作数量
    int get_num_operations_count() {
        return stack_num_op_cnt;
    }

    // 获取符号栈操作数量
    int get_sym_operations_count() {
        return stack_sym_op_cnt;
    }

    // 获取指定索引的数字栈操作
    // 通过指针参数返回: (操作类型, 数值, 时间戳)
    void get_num_operation_at(int index, int* op_type, int* value, int* timestamp) {
        if (index >= 0 && index < stack_num_op_cnt) {
            auto operation = stack_num_operation[index];
            *op_type = operation.first.first;
            *value = operation.first.second;
            *timestamp = operation.second;
        } else {
            *op_type = 0;
            *value = 0;
            *timestamp = 0;
        }
    }

    // 获取指定索引的符号栈操作
    // 通过指针参数返回: (操作类型, 符号, 时间戳)
    void get_sym_operation_at(int index, int* op_type, int* symbol, int* timestamp) {
        if (index >= 0 && index < stack_sym_op_cnt) {
            auto operation = stack_sym_operation[index];
            *op_type = operation.first.first;
            *symbol = operation.first.second;
            *timestamp = operation.second;
        } else {
            *op_type = 0;
            *symbol = 0;
            *timestamp = 0;
        }
    }

}


