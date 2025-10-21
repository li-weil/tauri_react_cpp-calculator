// 声明外部 C++ 栈函数
extern "C" {
    fn init_stack(capacity: i32) -> i32;
    fn calculation(input: *const std::os::raw::c_char) -> i32;
    fn get_num_operations_count() -> i32;
    fn get_sym_operations_count() -> i32;
    fn get_num_operation_at(index: i32, op_type: *mut i32, value: *mut i32, timestamp: *mut i32);
    fn get_sym_operation_at(index: i32, op_type: *mut i32, symbol: *mut i32, timestamp: *mut i32);
}

use std::ffi::{CStr, CString};

// 安全的 C++ 栈函数包装器
fn init_stack_safe(capacity: i32) -> Result<String, String> {
    unsafe {
        let result = init_stack(capacity);
        if result == 0 {
            Ok("Stack initialized successfully".to_string())
        } else {
            Err("Failed to initialize stack".to_string())
        }
    }
}

fn calculation_safe(expression: &str) -> Result<i32, String> {
    unsafe {
        let c_expr = CString::new(expression).map_err(|_| "Invalid expression")?;
        let result = calculation(c_expr.as_ptr());

        // 处理错误码
        match result {
            -1 => Err("栈未初始化".to_string()),
            -2 => Err("输入为空".to_string()),
            -3 => Err("除以零".to_string()),
            -4 => Err("未知操作符".to_string()),
            -5 => Err("无效表达式".to_string()),
            -6 => Err("没有结果".to_string()),
            -7 => Err("括号不匹配".to_string()),
            _ => Ok(result)
        }
    }
}

// Tauri 命令：初始化栈
#[tauri::command]
fn init_stack_command(capacity: i32) -> Result<String, String> {
    init_stack_safe(capacity)
}

// Tauri 命令：计算表达式
#[tauri::command]
fn calculate_expression(expression: String) -> Result<i32, String> {
    calculation_safe(&expression)
}

// 操作数据结构
#[derive(serde::Serialize, Clone)]
struct StackOperation {
    timestamp: i32,
    operation_type: i32,  // 0: 出栈, 1: 入栈
    value: i32,          // 数值栈的值，符号栈时为字符ASCII码
    stack_type: String,  // "num" 或 "sym"
}

// Tauri 命令：获取操作历史用于动画
#[tauri::command]
fn get_animation_operations() -> Result<Vec<StackOperation>, String> {
    unsafe {
        let mut operations = Vec::new();

        // 获取数字栈操作
        let num_count = get_num_operations_count();
        for i in 0..num_count {
            let mut op_type = 0;
            let mut value = 0;
            let mut timestamp = 0;
            get_num_operation_at(i, &mut op_type, &mut value, &mut timestamp);
            operations.push(StackOperation {
                timestamp,
                operation_type: op_type,
                value,
                stack_type: "num".to_string(),
            });
        }

        // 获取符号栈操作
        let sym_count = get_sym_operations_count();
        for i in 0..sym_count {
            let mut op_type = 0;
            let mut symbol = 0;
            let mut timestamp = 0;
            get_sym_operation_at(i, &mut op_type, &mut symbol, &mut timestamp);
            operations.push(StackOperation {
                timestamp,
                operation_type: op_type,
                value: symbol,
                stack_type: "sym".to_string(),
            });
        }

        // 按时间戳排序
        operations.sort_by(|a, b| a.timestamp.cmp(&b.timestamp));

        Ok(operations)
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![init_stack_command, calculate_expression, get_animation_operations])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
