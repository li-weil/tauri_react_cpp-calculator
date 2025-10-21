fn main() {
    // 编译 C++ 代码
    cc::Build::new()
        .cpp(true) // 启用 C++ 编译
        .file("cpp/calc_expression.cpp") // 表达式计算源文件
        .file("cpp/calc_polynomial.cpp") // 多项式计算源文件
        .file("cpp/polynomial.cpp") // 多项式类实现源文件
        .include("cpp") // 包含目录
        .flag("/utf-8") // 支持 UTF-8 编码，注释使用中文
        .compile("hello_cpp" ); // 编译为静态库

    // 重新编译条件
    println!("cargo:rerun-if-changed=cpp/calc_expression.cpp");
    println!("cargo:rerun-if-changed=cpp/calc_polynomial.cpp");
    println!("cargo:rerun-if-changed=cpp/polynomial.cpp");
    println!("cargo:rerun-if-changed=cpp/polynomial.hpp");
    println!("cargo:rerun-if-changed=cpp/stack.hpp");

    tauri_build::build()
}
