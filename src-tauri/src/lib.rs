// 声明外部 C++ 栈函数
extern "C" {
    fn init_stack(capacity: i32) -> i32;
    fn calculation(input: *const std::os::raw::c_char) -> i32;
    fn get_num_operations_count() -> i32;
    fn get_sym_operations_count() -> i32;
    fn get_num_operation_at(index: i32, op_type: *mut i32, value: *mut i32, timestamp: *mut i32);
    fn get_sym_operation_at(index: i32, op_type: *mut i32, symbol: *mut i32, timestamp: *mut i32);
}

// 声明外部 C++ 多项式函数
extern "C" {
    fn create_polynomial(name: std::os::raw::c_char, input: *const std::os::raw::c_char) -> i32;
    fn get_polynomial_to_string(name: std::os::raw::c_char, output: *mut std::os::raw::c_char, buffer_size: i32) -> i32;
    fn calculate_polynomials(expression: *const std::os::raw::c_char, output: *mut std::os::raw::c_char, buffer_size: i32) -> i32;
    fn evaluate_polynomial(name: std::os::raw::c_char, x: i32, result: *mut i32) -> i32;
    fn derivative_polynomial(name: std::os::raw::c_char, output: *mut std::os::raw::c_char, buffer_size: i32) -> i32;
    fn clear_all_polynomials() -> i32;
    fn get_polynomial_names(names: *mut std::os::raw::c_char, max_count: i32) -> i32;
    fn polynomial_exists(name: std::os::raw::c_char) -> i32;
    fn get_polynomial_term_count(name: std::os::raw::c_char, count: *mut i32) -> i32;
    fn get_polynomial_error_description(error_code: i32) -> *const std::os::raw::c_char;
    // New LaTeX functions
    fn get_polynomial_string_with_latex(name: std::os::raw::c_char, output: *mut std::os::raw::c_char, buffer_size: i32) -> i32;
    fn calculate_polynomials_with_latex(expression: *const std::os::raw::c_char, output: *mut std::os::raw::c_char, buffer_size: i32) -> i32;
    fn derivative_polynomial_with_latex(name: std::os::raw::c_char, output: *mut std::os::raw::c_char, buffer_size: i32) -> i32;
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
            -7 => Err("无效表达式".to_string()),
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

// 多项式输出数据结构
#[derive(serde::Serialize, Clone)]
struct PolynomialOutput {
    standard: String,  // 标准格式
    latex: String,     // LaTeX格式
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



// 安全地创建多项式
fn create_polynomial_safe(name: char, input: &str) -> Result<String, String> {
    unsafe {
        let c_input = CString::new(input).map_err(|_| "Invalid input")?;
        let result = create_polynomial(name as std::os::raw::c_char, c_input.as_ptr());

        match result {
            0 => Ok(format!("多项式 '{}' 创建成功", name)),
            -1 => Err("无效的多项式名称 (必须是 a-e)".to_string()),
            -2 => Err("无效的输入格式".to_string()),
            -3 => Err("多项式数量超过限制 (最多5个)".to_string()),
            _ => Err("未知错误".to_string())
        }
    }
}

// 安全地获取多项式字符串
fn get_polynomial_string_safe(name: char) -> Result<String, String> {
    unsafe {
        let buffer_size = 1024;
        let mut buffer = vec![0u8; buffer_size];
        let result = get_polynomial_to_string(
            name as std::os::raw::c_char,
            buffer.as_mut_ptr() as *mut std::os::raw::c_char,
            buffer_size as i32
        );

        match result {
            0 => {
                let c_str = CStr::from_ptr(buffer.as_ptr() as *const std::os::raw::c_char);
                Ok(c_str.to_string_lossy().to_string())
            },
            -1 => Err("无效的多项式名称".to_string()),
            -2 => Err("多项式不存在".to_string()),
            _ => Err("未知错误".to_string())
        }
    }
}

// 安全地计算多项式表达式
fn calculate_polynomials_safe(expression: &str) -> Result<String, String> {
    unsafe {
        let c_expr = CString::new(expression).map_err(|_| "Invalid expression")?;
        let buffer_size = 2048;
        let mut buffer = vec![0u8; buffer_size];
        let result = calculate_polynomials(
            c_expr.as_ptr(),
            buffer.as_mut_ptr() as *mut std::os::raw::c_char,
            buffer_size as i32
        );

        match result {
            0 => {
                let c_str = CStr::from_ptr(buffer.as_ptr() as *const std::os::raw::c_char);
                Ok(c_str.to_string_lossy().to_string())
            },
            -1 => Err("无效的多项式名称".to_string()),
            -2 => Err("多项式不存在".to_string()),
            -3 => Err("无效的输入".to_string()),
            -4 => Err("多项式数量超过限制".to_string()),
            -5 => Err("空表达式".to_string()),
            -6 => Err("表达式解析错误".to_string()),
            -7 => Err("无效表达式".to_string()),
            -8 => Err("括号不匹配".to_string()),
            -9 => Err("表达式中有无效字符".to_string()),
            _ => Err("未知错误".to_string())
        }
    }
}
// 安全地计算多项式在x处的值
fn evaluate_polynomial_safe(name: char, x: i32) -> Result<i32, String> {
    unsafe {
        let mut result = 0;
        let ret = evaluate_polynomial(name as std::os::raw::c_char, x, &mut result);

        match ret {
            0 => Ok(result),
            -1 => Err("无效的多项式名称".to_string()),
            -2 => Err("多项式不存在".to_string()),
            _ => Err("未知错误".to_string())
        }
    }
}

// 安全地求多项式的导函数
fn derivative_polynomial_safe(name: char) -> Result<String, String> {
    unsafe {
        let buffer_size = 1024;
        let mut buffer = vec![0u8; buffer_size];
        let result = derivative_polynomial(
            name as std::os::raw::c_char,
            buffer.as_mut_ptr() as *mut std::os::raw::c_char,
            buffer_size as i32
        );

        match result {
            0 => {
                let c_str = CStr::from_ptr(buffer.as_ptr() as *const std::os::raw::c_char);
                Ok(c_str.to_string_lossy().to_string())
            },
            -1 => Err("无效的多项式名称".to_string()),
            -2 => Err("多项式不存在".to_string()),
            _ => Err("未知错误".to_string())
        }
    }
}

// 安全地清空所有多项式
fn clear_all_polynomials_safe() -> Result<String, String> {
    unsafe {
        let result = clear_all_polynomials();
        match result {
            0 => Ok("所有多项式已清空".to_string()),
            _ => Err("清空失败".to_string())
        }
    }
}

// 安全地获取多项式名称列表
fn get_polynomial_names_safe() -> Result<Vec<char>, String> {
    unsafe {
        let max_count = 5 as i32;
        let mut names = vec![0u8; max_count as usize];
        let count = get_polynomial_names(names.as_mut_ptr() as *mut std::os::raw::c_char, max_count);

        if count < 0 {
            return Err("获取多项式名称失败".to_string());
        }

        let result: Vec<char> = names[..count as usize].iter().map(|&b| b as char).collect();
        Ok(result)
    }
}

// 安全地检查多项式是否存在
fn polynomial_exists_safe(name: char) -> Result<bool, String> {
    unsafe {
        let result = polynomial_exists(name as std::os::raw::c_char);
        match result {
            1 => Ok(true),
            0 => Ok(false),
            _ => Err("检查多项式存在性失败".to_string())
        }
    }
}

// 安全地获取多项式项数
fn get_polynomial_term_count_safe(name: char) -> Result<i32, String> {
    unsafe {
        let mut count = 0;
        let result = get_polynomial_term_count(name as std::os::raw::c_char, &mut count);

        match result {
            0 => Ok(count),
            -1 => Err("无效的多项式名称".to_string()),
            -2 => Err("多项式不存在".to_string()),
            _ => Err("未知错误".to_string())
        }
    }
}

// 安全地获取多项式字符串（包含LaTeX格式）
fn get_polynomial_string_with_latex_safe(name: char) -> Result<(String, String), String> {
    unsafe {
        let buffer_size = 2048;
        let mut buffer = vec![0u8; buffer_size];
        let result = get_polynomial_string_with_latex(
            name as std::os::raw::c_char,
            buffer.as_mut_ptr() as *mut std::os::raw::c_char,
            buffer_size as i32
        );

        match result {
            0 => {
                let c_str = CStr::from_ptr(buffer.as_ptr() as *const std::os::raw::c_char);
                let combined = c_str.to_string_lossy().to_string();

                // Split by "|" to separate standard and LaTeX formats
                if let Some(pipe_pos) = combined.find('|') {
                    let standard = combined[..pipe_pos].to_string();
                    let latex = combined[pipe_pos + 1..].to_string();
                    Ok((standard, latex))
                } else {
                    // Fallback: treat as standard format only
                    Ok((combined.clone(), combined))
                }
            },
            -1 => Err("无效的多项式名称".to_string()),
            -2 => Err("多项式不存在".to_string()),
            _ => Err("未知错误".to_string())
        }
    }
}

// 安全地计算多项式表达式（包含LaTeX格式）
fn calculate_polynomials_with_latex_safe(expression: &str) -> Result<(String, String), String> {
    unsafe {
        let c_expr = CString::new(expression).map_err(|_| "Invalid expression")?;
        let buffer_size = 4096;
        let mut buffer = vec![0u8; buffer_size];
        let result = calculate_polynomials_with_latex(
            c_expr.as_ptr(),
            buffer.as_mut_ptr() as *mut std::os::raw::c_char,
            buffer_size as i32
        );

        match result {
            0 => {
                let c_str = CStr::from_ptr(buffer.as_ptr() as *const std::os::raw::c_char);
                let combined = c_str.to_string_lossy().to_string();

                //  "|" 分割 standard and LaTeX formats
                if let Some(pipe_pos) = combined.find('|') {
                    let standard = combined[..pipe_pos].to_string();
                    let latex = combined[pipe_pos + 1..].to_string();
                    Ok((standard, latex))
                } else {
                    Ok((combined.clone(), combined))
                }
            },
            -1 => Err("无效的多项式名称".to_string()),
            -2 => Err("多项式不存在".to_string()),
            -3 => Err("无效的输入".to_string()),
            -4 => Err("多项式数量超过限制".to_string()),
            -5 => Err("空表达式".to_string()),
            -6 => Err("表达式解析错误".to_string()),
            -7 => Err("无效表达式".to_string()),
            -8 => Err("括号不匹配".to_string()),
            -9 => Err("表达式中有无效字符".to_string()),
            _ => Err("未知错误".to_string())
        }
    }
}

/// 安全地求多项式的导函数（包含LaTeX格式）
fn derivative_polynomial_with_latex_safe(name: char) -> Result<(String, String), String> {
    unsafe {
        let buffer_size = 2048;
        let mut buffer = vec![0u8; buffer_size];
        let result = derivative_polynomial_with_latex(
            name as std::os::raw::c_char,
            buffer.as_mut_ptr() as *mut std::os::raw::c_char,
            buffer_size as i32
        );

        match result {
            0 => {
                let c_str = CStr::from_ptr(buffer.as_ptr() as *const std::os::raw::c_char);
                let combined = c_str.to_string_lossy().to_string();

                //  "|" 分割 standard and LaTeX formats
                if let Some(pipe_pos) = combined.find('|') {
                    let standard = combined[..pipe_pos].to_string();
                    let latex = combined[pipe_pos + 1..].to_string();
                    Ok((standard, latex))
                } else {
                    Ok((combined.clone(), combined))
                }
            },
            -1 => Err("无效的多项式名称".to_string()),
            -2 => Err("多项式不存在".to_string()),
            _ => Err("未知错误".to_string())
        }
    }
}

// ============================================================================
// Tauri 命令定义
// ============================================================================

/// Tauri 命令：创建多项式
#[tauri::command]
fn create_polynomial_command(name: char, input: String) -> Result<String, String> {
    create_polynomial_safe(name, &input)
}

/// Tauri 命令：获取多项式字符串
#[tauri::command]
fn get_polynomial_command(name: char) -> Result<String, String> {
    get_polynomial_string_safe(name)
}

/// Tauri 命令：计算多项式表达式
#[tauri::command]
fn calculate_polynomial_expression(expression: String) -> Result<String, String> {
    calculate_polynomials_safe(&expression)
}

/// Tauri 命令：计算多项式在x处的值
#[tauri::command]
fn evaluate_polynomial_command(name: char, x: i32) -> Result<i32, String> {
    evaluate_polynomial_safe(name, x)
}

/// Tauri 命令：求多项式的导函数
#[tauri::command]
fn derivative_polynomial_command(name: char) -> Result<String, String> {
    derivative_polynomial_safe(name)
}

/// Tauri 命令：清空所有多项式
#[tauri::command]
fn clear_all_polynomials_command() -> Result<String, String> {
    clear_all_polynomials_safe()
}

/// Tauri 命令：获取多项式名称列表
#[tauri::command]
fn get_polynomial_names_command() -> Result<Vec<char>, String> {
    get_polynomial_names_safe()
}

/// Tauri 命令：检查多项式是否存在
#[tauri::command]
fn polynomial_exists_command(name: char) -> Result<bool, String> {
    polynomial_exists_safe(name)
}

/// Tauri 命令：获取多项式项数
#[tauri::command]
fn get_polynomial_term_count_command(name: char) -> Result<i32, String> {
    get_polynomial_term_count_safe(name)
}

/// Tauri 命令：获取多项式字符串（包含LaTeX格式）
#[tauri::command]
fn get_polynomial_with_latex_command(name: char) -> Result<PolynomialOutput, String> {
    let (standard, latex) = get_polynomial_string_with_latex_safe(name)?;
    Ok(PolynomialOutput { standard, latex })
}

/// Tauri 命令：计算多项式表达式（包含LaTeX格式）
#[tauri::command]
fn calculate_polynomial_with_latex_command(expression: String) -> Result<PolynomialOutput, String> {
    let (standard, latex) = calculate_polynomials_with_latex_safe(&expression)?;
    Ok(PolynomialOutput { standard, latex })
}

/// Tauri 命令：求多项式的导函数（包含LaTeX格式）
#[tauri::command]
fn derivative_polynomial_with_latex_command(name: char) -> Result<PolynomialOutput, String> {
    let (standard, latex) = derivative_polynomial_with_latex_safe(name)?;
    Ok(PolynomialOutput { standard, latex })
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![
            init_stack_command,
            calculate_expression,
            get_animation_operations,
            create_polynomial_command,
            get_polynomial_command,
            calculate_polynomial_expression,
            evaluate_polynomial_command,
            derivative_polynomial_command,
            clear_all_polynomials_command,
            get_polynomial_names_command,
            polynomial_exists_command,
            get_polynomial_term_count_command,
            get_polynomial_with_latex_command,
            calculate_polynomial_with_latex_command,
            derivative_polynomial_with_latex_command
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
