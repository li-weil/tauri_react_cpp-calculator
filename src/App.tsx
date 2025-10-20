import { useState, useEffect } from "react";
import reactLogo from "./assets/react.svg";
import { invoke } from "@tauri-apps/api/core";
import "./App.css";

function App() {
  const [expression, setExpression] = useState("");
  const [result, setResult] = useState<number | null>(null);
  const [error, setError] = useState("");
  const [initStatus, setInitStatus] = useState("");

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
    } catch (err) {
      console.error("Error calculating expression:", err);
      setError("Error calculating expression. Please check your input.");
      setResult(null);
    }
  }

  const handleKeyPress = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') {
      calculate();
    }
  };

  return (
    <main className="container">
      <h1>C++ Stack Calculator</h1>

      <div className="row">
        <a href="https://vite.dev" target="_blank">
          <img src="/vite.svg" className="logo vite" alt="Vite logo" />
        </a>
        <a href="https://tauri.app" target="_blank">
          <img src="/tauri.svg" className="logo tauri" alt="Tauri logo" />
        </a>
        <a href="https://react.dev" target="_blank">
          <img src={reactLogo} className="logo react" alt="React logo" />
        </a>
      </div>

      {/* 初始化状态显示 */}
      <div className="row" style={{ margin: "20px 0", padding: "15px", backgroundColor: "#e8f5e8", borderRadius: "8px" }}>
        <h2>C++ Backend Status</h2>
        <p><strong>Stack Status:</strong> {initStatus}</p>
      </div>

      {/* 计算器界面 */}
      <div className="calculator" style={{ margin: "20px 0", padding: "20px", backgroundColor: "#f8f9fa", borderRadius: "8px", border: "1px solid #dee2e6" }}>
        <h2>Expression Calculator</h2>
        <p style={{ marginBottom: "15px", color: "#6c757d" }}>
          Enter mathematical expressions using +, -, *, / and parentheses
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
          Calculate
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
          Clear
        </button>

        {/* 结果显示 */}
        {result !== null && (
          <div style={{ marginTop: "20px", padding: "15px", backgroundColor: "#d4edda", borderRadius: "4px", border: "1px solid #c3e6cb" }}>
            <h3 style={{ margin: "0 0 10px 0", color: "#155724" }}>Result</h3>
            <p style={{ fontSize: "24px", fontWeight: "bold", margin: 0 }}>{result}</p>
          </div>
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
          <h4 style={{ margin: "0 0 10px 0" }}>Example Expressions:</h4>
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
