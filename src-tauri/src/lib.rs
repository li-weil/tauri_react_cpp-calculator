// 声明外部 C++ 栈函数
extern "C" {
    fn init_stack(capacity: i32) -> i32;
    fn calculation(input: *const std::os::raw::c_char) -> i32;
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
            -1 => Err("Stack not initialized".to_string()),
            -2 => Err("Empty input".to_string()),
            -3 => Err("Division by zero".to_string()),
            -4 => Err("Unknown operator".to_string()),
            -5 => Err("Invalid expression".to_string()),
            -6 => Err("No result available".to_string()),
            -7 => Err("Invalid expression - mismatched parentheses".to_string()),
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

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![init_stack_command, calculate_expression])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
