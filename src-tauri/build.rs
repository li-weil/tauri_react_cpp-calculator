fn main() {
    // 编译 C++ 代码
    cc::Build::new()
        .cpp(true) // 启用 C++ 编译
        .file("cpp/hello.cpp") // C++ 源文件
        .include("cpp") // 包含目录
        .compile("hello_cpp" ); // 编译为静态库

    // 重新编译条件
    println!("cargo:rerun-if-changed=cpp/hello.cpp");

    tauri_build::build()
}
