import { useState, useEffect } from "react";
import reactLogo from "./assets/react.svg";
import { invoke } from "@tauri-apps/api/core";
import StackAnimation from "./StackAnimation";
import "./App.css";

function App() {
  const [expression, setExpression] = useState("");
  const [result, setResult] = useState<number | null>(null);
  const [error, setError] = useState("");
  const [initStatus, setInitStatus] = useState("");
  const [operations, setOperations] = useState<any[]>([]);
  const [showAnimation, setShowAnimation] = useState(false);

  // 应用启动时初始化栈
  useEffect(() => {
    async function initializeStack() {
      try {
        const message = await invoke("init_stack_command", { capacity: 100 });
        setInitStatus(message as string);
      } catch (err) {
        console.error("Error initializing stack:", err);
        setInitStatus("Failed to initialize C++ backend");
      }
    }
    initializeStack();
  }, []);

  async function calculate() {
    if (!expression.trim()) {
      setError("Please enter an expression");
      return;
    }

    try {
      setError("");
      const calcResult = await invoke("calculate_expression", { expression });
      setResult(calcResult as number);

      // 计算完成后获取操作历史用于动画
      await loadOperations();
    } catch (err) {
      console.error("Error calculating expression:", err);
      setError(err as string);
      setResult(null);
    }
  }

  async function loadOperations() {
    try {
      const ops = await invoke("get_animation_operations");
      setOperations(ops as any[]);
      setShowAnimation(true);
    } catch (err) {
      console.error("Error loading operations:", err);
    }
  }

  const handleKeyPress = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      calculate();
    }
  };

  return (
    <main className="container">
      <h1>Tauri&C++ Stack Calculator</h1>

      {/* 计算器界面 */}
      <div className="calculator" style={{ margin: "20px 0", padding: "20px", backgroundColor: "#f8f9fa", borderRadius: "8px", border: "1px solid #dee2e6" }}>
        <h2>表达式计算器</h2>
        <p style={{ marginBottom: "15px", color: "#6c757d" }}>
          输入表达式，包括:+,-,*,/,^,(,),|. 注意，绝对值计算符号'|'不支持嵌套 
          注意，绝对值计算符号'|'不支持嵌套
        </p>

        <div className="input-group" style={{ marginBottom: "15px" }}>
          <input
            type="text"
            value={expression}
            onChange={(e) => setExpression(e.target.value)}
            onKeyPress={handleKeyPress}
            placeholder="Enter expression (e.g., 3+4*2)"
            style={{
              width: "100%",
              padding: "10px",
              fontSize: "16px",
              border: "1px solid #ced4da",
              borderRadius: "4px",
              boxSizing: "border-box"
            }}
          />
        </div>

        <button
          onClick={calculate}
          style={{
            padding: "10px 20px",
            fontSize: "16px",
            backgroundColor: "#007bff",
            color: "white",
            border: "none",
            borderRadius: "4px",
            cursor: "pointer",
            marginRight: "10px"
          }}
        >
          计算
        </button>

        <button
          onClick={() => {
            setExpression("");
            setResult(null);
            setError("");
          }}
          style={{
            padding: "10px 20px",
            fontSize: "16px",
            backgroundColor: "#6c757d",
            color: "white",
            border: "none",
            borderRadius: "4px",
            cursor: "pointer"
          }}
        >
          清空
        </button>

        {/* 结果显示 */}
        {result !== null && (
          <div style={{ marginTop: "20px", padding: "15px", backgroundColor: "#d4edda", borderRadius: "4px", border: "1px solid #c3e6cb" }}>
            <h3 style={{ margin: "0 0 10px 0", color: "#155724" }}>Result</h3>
            <p style={{ fontSize: "24px", fontWeight: "bold", margin: 0 }}>{result}</p>
          </div>
        )}

        {/* 栈操作动画 */}
        {showAnimation && operations.length > 0 && (
          <>
            <button
              onClick={() => setShowAnimation(false)}
              style={{
                padding: "8px 16px",
                fontSize: "14px",
                backgroundColor: "#dc3545",
                color: "white",
                border: "none",
                borderRadius: "4px",
                cursor: "pointer",
                marginTop: "10px"
              }}
            >
              隐藏动画
            </button>
            <StackAnimation operations={operations} />
          </>
        )}

        {/* 显示动画按钮 */}
        {!showAnimation && operations.length > 0 && (
          <button
            onClick={() => setShowAnimation(true)}
            style={{
              padding: "10px 20px",
              fontSize: "16px",
              backgroundColor: "#28a745",
              color: "white",
              border: "none",
              borderRadius: "4px",
              cursor: "pointer",
              marginTop: "20px"
            }}
          >
            显示栈操作动画
          </button>
        )}

        {/* 错误显示 */}
        {error && (
          <div style={{ marginTop: "20px", padding: "15px", backgroundColor: "#f8d7da", borderRadius: "4px", border: "1px solid #f5c6cb" }}>
            <h3 style={{ margin: "0 0 10px 0", color: "#721c24" }}>Error</h3>
            <p style={{ margin: 0 }}>{error}</p>
          </div>
        )}

        {/* 示例表达式 */}
        <div style={{ marginTop: "20px", padding: "15px", backgroundColor: "#e2e3e5", borderRadius: "4px" }}>
          <h4 style={{ margin: "0 0 10px 0" }}>示例表达式:</h4>
          <ul style={{ margin: 0, paddingLeft: "20px" }}>
            <li>3+4*2 = 11</li>
            <li>(3+4)*2 = 14</li>
            <li>10/2+3 = 8</li>
            <li>5*6-4/2 = 28</li>
          </ul>
        </div>
      </div>
    </main>
  );
}

export default App;
