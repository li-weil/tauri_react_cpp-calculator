import { useState, useEffect } from "react";
import { invoke } from "@tauri-apps/api/core";
import StackAnimation from "./StackAnimation";
import "./App.css";

// Function to render LaTeX
const renderLaTeX = (latex: string) => {
  if (typeof window !== 'undefined' && (window as any).katex) {
    try {
      return (window as any).katex.renderToString(latex, {
        throwOnError: false,
        displayMode: true
      });
    } catch (error) {
      console.error('LaTeX rendering error:', error);
      return latex;
    }
  }
  return latex;
};

function App() {
  const [expression, setExpression] = useState("");
  const [result, setResult] = useState<number | null>(null);
  const [error, setError] = useState("");
    const [operations, setOperations] = useState<any[]>([]);
  const [showAnimation, setShowAnimation] = useState(false);

  // 多项式计算器状态
  const [activeTab, setActiveTab] = useState<'expression' | 'polynomial'>('expression');
  const [polyName, setPolyName] = useState('a');
  const [polyInput, setPolyInput] = useState("");
  const [polyExpression, setPolyExpression] = useState("");
  const [polyResult, setPolyResult] = useState("");
  const [polyLatex, setPolyLatex] = useState("");
  const [polyError, setPolyError] = useState("");
  const [evaluateX, setEvaluateX] = useState("");
  const [evaluateResult, setEvaluateResult] = useState<number | null>(null);
  const [polynomialList, setPolynomialList] = useState<string[]>([]);
  const [showPolynomials, setShowPolynomials] = useState(false);
  const [katexLoaded, setKatexLoaded] = useState(false);

  // 应用启动时初始化栈
  useEffect(() => {
    async function initializeStack() {
      try {
        await invoke("init_stack_command", { capacity: 100 });
        console.log("Stack initialized successfully");
      } catch (err) {
        console.error("Error initializing stack:", err);
      }
    }
    initializeStack();
  }, []);

  // 加载多项式列表
  useEffect(() => {
    loadPolynomialList();
  }, []);

  // 加载 KaTeX
  useEffect(() => {
    const script = document.createElement('script');
    script.src = 'https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js';
    script.integrity = 'sha384-XjKyOOlGwcjNTAIQHIpgOno0Hl1YQqzUOEleOLALmuqehneUG+vnGctmUb0ZY0l8';
    script.crossOrigin = 'anonymous';
    script.onload = () => setKatexLoaded(true);
    document.head.appendChild(script);

    // Also load the CSS
    const link = document.createElement('link');
    link.rel = 'stylesheet';
    link.href = 'https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css';
    link.integrity = 'sha384-n8MVd4RsNIU0tAv4ct0nTaAbDJwPJzDEaqSD1odI+WdtXRGWt2kTvGFasHpSy3SV';
    link.crossOrigin = 'anonymous';
    document.head.appendChild(link);

    return () => {
      document.head.removeChild(script);
      document.head.removeChild(link);
    };
  }, []);

  async function loadPolynomialList() {
    try {
      const names = await invoke("get_polynomial_names_command");
      setPolynomialList(names as string[]);
    } catch (err) {
      console.error("Error loading polynomial list:", err);
    }
  }

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

  // 多项式功能函数
  async function createPolynomial() {
    if (!polyInput.trim()) {
      setPolyError("请输入多项式系数和指数");
      return;
    }

    try {
      setPolyError("");
      const message = await invoke("create_polynomial_command", {
        name: polyName,
        input: polyInput
      });
      setPolyResult(message as string);
      loadPolynomialList(); // 重新加载多项式列表
    } catch (err) {
      console.error("Error creating polynomial:", err);
      setPolyError(err as string);
      setPolyResult("");
    }
  }

  async function calculatePolynomialExpression() {
    if (!polyExpression.trim()) {
      setPolyError("请输入多项式表达式");
      return;
    }

    try {
      setPolyError("");
      const result = await invoke("calculate_polynomial_with_latex_command", {
        expression: polyExpression
      });
      const output = result as { standard: string; latex: string };
      setPolyResult(output.standard);
      setPolyLatex(output.latex);
    } catch (err) {
      console.error("Error calculating polynomial expression:", err);
      setPolyError(err as string);
      setPolyResult("");
      setPolyLatex("");
    }
  }

  async function evaluatePolynomialAtX() {
    if (!evaluateX.trim()) {
      setPolyError("请输入x的值");
      return;
    }

    try {
      setPolyError("");
      const x = parseInt(evaluateX);
      if (isNaN(x)) {
        setPolyError("x的值必须是整数");
        return;
      }

      const result = await invoke("evaluate_polynomial_command", {
        name: polyName,
        x: x
      });
      setEvaluateResult(result as number);
    } catch (err) {
      console.error("Error evaluating polynomial:", err);
      setPolyError(err as string);
      setEvaluateResult(null);
    }
  }

  async function getPolynomialDerivative() {
    try {
      setPolyError("");
      const result = await invoke("derivative_polynomial_with_latex_command", {
        name: polyName
      });
      const output = result as { standard: string; latex: string };
      setPolyResult(output.standard);
      setPolyLatex(output.latex);
    } catch (err) {
      console.error("Error getting polynomial derivative:", err);
      setPolyError(err as string);
      setPolyResult("");
      setPolyLatex("");
    }
  }

  async function clearAllPolynomials() {
    try {
      setPolyError("");
      const message = await invoke("clear_all_polynomials_command");
      setPolyResult(message as string);
      setPolyLatex("");
      setPolynomialList([]);
      setPolyInput("");
      setPolyExpression("");
    } catch (err) {
      console.error("Error clearing polynomials:", err);
      setPolyError(err as string);
      setPolyResult("");
      setPolyLatex("");
    }
  }

  async function getPolynomialString(name: string) {
    try {
      setPolyError("");
      const result = await invoke("get_polynomial_with_latex_command", { name });
      const output = result as { standard: string; latex: string };
      return output.standard; 
    } catch (err) {
      console.error("Error getting polynomial:", err);
      setPolyError(err as string);
      return "";
    }
  }

async function getPolynomialWithLatex(name: string) {
    try {
      setPolyError("");
      const result = await invoke("get_polynomial_with_latex_command", { name });
      const output = result as { standard: string; latex: string };
      setPolyResult(output.standard);
      setPolyLatex(output.latex);
    } catch (err) {
      console.error("Error getting polynomial with LaTeX:", err);
      setPolyError(err as string);
      setPolyResult("");
      setPolyLatex("");
    }
  }

  return (
    <main className="container">
      <h1>Tauri&C++ Stack Calculator</h1>

      {/* 标签页切换 */}
      <div style={{ marginBottom: "20px" }}>
        <button
          onClick={() => setActiveTab('expression')}
          style={{
            padding: "10px 20px",
            fontSize: "16px",
            backgroundColor: activeTab === 'expression' ? "#007bff" : "#6c757d",
            color: "white",
            border: "none",
            borderRadius: "4px 0 0 4px",
            cursor: "pointer",
            marginRight: "2px"
          }}
        >
          表达式计算器
        </button>
        <button
          onClick={() => setActiveTab('polynomial')}
          style={{
            padding: "10px 20px",
            fontSize: "16px",
            backgroundColor: activeTab === 'polynomial' ? "#007bff" : "#6c757d",
            color: "white",
            border: "none",
            borderRadius: "0 4px 4px 0",
            cursor: "pointer"
          }}
        >
          多项式计算器
        </button>
      </div>

      {/* 表达式计算器界面 */}
      {activeTab === 'expression' && (
        <div className="calculator" style={{ margin: "20px 0", padding: "20px", backgroundColor: "#f8f9fa", borderRadius: "8px", border: "1px solid #dee2e6" }}>
          <h2>表达式计算器</h2>
        <p style={{ marginBottom: "15px", color: "#6c757d" }}>
          输入表达式，运算符包括:+,-,*,/,^,(,),|.<br />
          仅支持整数运算，除法向下取整.<br />
          计算范围为32位有符号整数.<br />
          注意，绝对值计算符号'|'因无左右之分，故不支持嵌套 

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
            <li>10+3^|9-2*(2+4)| = 37</li>
            <li>(((6+6)*6+3)*2+6)*2 = 312</li>
            <li>(20+2)*(6/2) = 66</li>
            <li>5*6-4/2 = 28</li>
          </ul>
        </div>
        </div>
      )}

      {/* 多项式计算器界面 */}
      {activeTab === 'polynomial' && (
        <div className="polynomial-calculator" style={{ margin: "20px 0", padding: "20px", backgroundColor: "#f8f9fa", borderRadius: "8px", border: "1px solid #dee2e6" }}>
          <h2>一元稀疏多项式计算器</h2>

          {/* 多项式构建 */}
          <div style={{ marginBottom: "30px", padding: "20px", backgroundColor: "#e9ecef", borderRadius: "8px" }}>
            <h3 style={{ margin: "0 0 15px 0" }}>构建多项式</h3>
            <p style={{ marginBottom: "15px", color: "#6c757d", fontSize: "14px" }}>
              输入格式: 系数1,指数1,系数2,指数2,... (例如: 3,2,2,1,1,0 表示 3x²+2x+1)<br />
              无需项数n，无需保证指数递减排列
            </p>

            <div style={{ display: "flex", gap: "10px", alignItems: "center", marginBottom: "15px" }}>
              <select
                value={polyName}
                onChange={(e) => setPolyName(e.target.value)}
                style={{
                  padding: "10px",
                  fontSize: "16px",
                  border: "1px solid #ced4da",
                  borderRadius: "4px"
                }}
              >
                <option value="a">多项式 a</option>
                <option value="b">多项式 b</option>
                <option value="c">多项式 c</option>
                <option value="d">多项式 d</option>
                <option value="e">多项式 e</option>
              </select>

              <input
                type="text"
                value={polyInput}
                onChange={(e) => setPolyInput(e.target.value)}
                placeholder="例如: 3,2,2,1,1,0"
                style={{
                  flex: 1,
                  padding: "10px",
                  fontSize: "16px",
                  border: "1px solid #ced4da",
                  borderRadius: "4px"
                }}
              />

              <button
                onClick={createPolynomial}
                style={{
                  padding: "10px 20px",
                  fontSize: "16px",
                  backgroundColor: "#28a745",
                  color: "white",
                  border: "none",
                  borderRadius: "4px",
                  cursor: "pointer"
                }}
              >
                构建
              </button>
            </div>
          </div>

          {/* 多项式表达式计算 */}
          <div style={{ marginBottom: "30px", padding: "20px", backgroundColor: "#e9ecef", borderRadius: "8px" }}>
            <h3 style={{ margin: "0 0 15px 0" }}>多项式运算</h3>
            <p style={{ marginBottom: "15px", color: "#6c757d", fontSize: "14px" }}>
              支持运算: + (加法), - (减法), * (乘法), 括号 (例如: a+b, a-b*c, (a+b)*c)
            </p>

            <div style={{ display: "flex", gap: "10px", alignItems: "center", marginBottom: "15px" }}>
              <input
                type="text"
                value={polyExpression}
                onChange={(e) => setPolyExpression(e.target.value)}
                placeholder="例如: a+b*c"
                style={{
                  flex: 1,
                  padding: "10px",
                  fontSize: "16px",
                  border: "1px solid #ced4da",
                  borderRadius: "4px"
                }}
              />

              <button
                onClick={calculatePolynomialExpression}
                style={{
                  padding: "10px 20px",
                  fontSize: "16px",
                  backgroundColor: "#007bff",
                  color: "white",
                  border: "none",
                  borderRadius: "4px",
                  cursor: "pointer"
                }}
              >
                计算
              </button>
            </div>
          </div>

          {/* 多项式求值和求导 */}
          <div style={{ marginBottom: "30px", padding: "20px", backgroundColor: "#e9ecef", borderRadius: "8px" }}>
            <h3 style={{ margin: "0 0 15px 0" }}>多项式求值与求导</h3>

            <div style={{ display: "flex", gap: "10px", alignItems: "center", marginBottom: "15px" }}>
              <select
                value={polyName}
                onChange={(e) => setPolyName(e.target.value)}
                style={{
                  padding: "10px",
                  fontSize: "16px",
                  border: "1px solid #ced4da",
                  borderRadius: "4px"
                }}
              >
                <option value="a">多项式 a</option>
                <option value="b">多项式 b</option>
                <option value="c">多项式 c</option>
                <option value="d">多项式 d</option>
                <option value="e">多项式 e</option>
              </select>

              <input
                type="text"
                value={evaluateX}
                onChange={(e) => setEvaluateX(e.target.value)}
                placeholder="x的值"
                style={{
                  width: "100px",
                  padding: "10px",
                  fontSize: "16px",
                  border: "1px solid #ced4da",
                  borderRadius: "4px"
                }}
              />

              <button
                onClick={evaluatePolynomialAtX}
                style={{
                  padding: "10px 20px",
                  fontSize: "16px",
                  backgroundColor: "#17a2b8",
                  color: "white",
                  border: "none",
                  borderRadius: "4px",
                  cursor: "pointer"
                }}
              >
                求值
              </button>

              <button
                onClick={getPolynomialDerivative}
                style={{
                  padding: "10px 20px",
                  fontSize: "16px",
                  backgroundColor: "#ffc107",
                  color: "black",
                  border: "none",
                  borderRadius: "4px",
                  cursor: "pointer"
                }}
              >
                求导
              </button>

              <button
                onClick={clearAllPolynomials}
                style={{
                  padding: "10px 20px",
                  fontSize: "16px",
                  backgroundColor: "#dc3545",
                  color: "white",
                  border: "none",
                  borderRadius: "4px",
                  cursor: "pointer"
                }}
              >
                清空所有
              </button>
            </div>
          </div>

          {/* 已创建的多项式列表 */}
          <div style={{ marginBottom: "30px", padding: "20px", backgroundColor: "#e9ecef", borderRadius: "8px" }}>
            <div style={{ display: "flex", justifyContent: "space-between", alignItems: "center", marginBottom: "15px" }}>
              <h3 style={{ margin: 0 }}>已创建的多项式</h3>
              <button
                onClick={() => setShowPolynomials(!showPolynomials)}
                style={{
                  padding: "5px 15px",
                  fontSize: "14px",
                  backgroundColor: "#6c757d",
                  color: "white",
                  border: "none",
                  borderRadius: "4px",
                  cursor: "pointer"
                }}
              >
                {showPolynomials ? "隐藏" : "显示"}
              </button>
            </div>

            {showPolynomials && polynomialList.length > 0 && (
              <div style={{ marginTop: "15px" }}>
                {polynomialList.map((name) => (
                  <div key={name} style={{ marginBottom: "10px", padding: "10px", backgroundColor: "#f8f9fa", borderRadius: "4px" }}>
                    <strong>多项式 {name}:</strong> <span id={`poly-${name}`}></span>
                    <button
                      onClick={() => {
                        getPolynomialString(name).then(result => {
                          const element = document.getElementById(`poly-${name}`);
                          if (element) {
                            element.textContent = result;
                          }
                        });
                      }}
                      style={{
                        marginLeft: "10px",
                        padding: "3px 8px",
                        fontSize: "12px",
                        backgroundColor: "#007bff",
                        color: "white",
                        border: "none",
                        borderRadius: "3px",
                        cursor: "pointer"
                      }}
                    >
                      显示
                    </button>
                    <button
                      onClick={() => getPolynomialWithLatex(name)}
                      style={{
                        marginLeft: "5px",
                        padding: "3px 8px",
                        fontSize: "12px",
                        backgroundColor: "#28a745",
                        color: "white",
                        border: "none",
                        borderRadius: "3px",
                        cursor: "pointer"
                      }}
                    >
                      LaTeX
                    </button>
                  </div>
                ))}
              </div>
            )}

            {showPolynomials && polynomialList.length === 0 && (
              <p style={{ color: "#6c757d", fontStyle: "italic" }}>暂无已创建的多项式</p>
            )}
          </div>

          {/* 多项式结果显示 */}
          {polyResult && (
            <div style={{ marginTop: "20px", padding: "15px", backgroundColor: "#d4edda", borderRadius: "4px", border: "1px solid #c3e6cb" }}>
              <h3 style={{ margin: "0 0 10px 0", color: "#155724" }}>结果</h3>
              <div style={{ marginBottom: "10px" }}>
                <strong>标准格式:</strong>
                <p style={{ fontSize: "16px", margin: "5px 0", fontFamily: "monospace" }}>{polyResult}</p>
              </div>
              {polyLatex && katexLoaded && (
                <div>
                  <strong>LaTeX 格式:</strong>
                  <div
                    style={{
                      fontSize: "18px",
                      margin: "5px 0",
                      padding: "10px",
                      backgroundColor: "#f8f9fa",
                      borderRadius: "4px",
                      textAlign: "center"
                    }}
                    dangerouslySetInnerHTML={{ __html: renderLaTeX(polyLatex) }}
                  />
                </div>
              )}
            </div>
          )}

          {/* 求值结果显示 */}
          {evaluateResult !== null && (
            <div style={{ marginTop: "20px", padding: "15px", backgroundColor: "#d1ecf1", borderRadius: "4px", border: "1px solid #bee5eb" }}>
              <h3 style={{ margin: "0 0 10px 0", color: "#0c5460" }}>求值结果</h3>
              <p style={{ fontSize: "18px", fontWeight: "bold", margin: 0 }}>{evaluateResult}</p>
            </div>
          )}

          {/* 错误显示 */}
          {polyError && (
            <div style={{ marginTop: "20px", padding: "15px", backgroundColor: "#f8d7da", borderRadius: "4px", border: "1px solid #f5c6cb" }}>
              <h3 style={{ margin: "0 0 10px 0", color: "#721c24" }}>错误</h3>
              <p style={{ margin: 0 }}>{polyError}</p>
            </div>
          )}

          {/* 示例 */}
          <div style={{ marginTop: "20px", padding: "15px", backgroundColor: "#e2e3e5", borderRadius: "4px" }}>
            <h4 style={{ margin: "0 0 10px 0" }}>使用示例:</h4>
            <ul style={{ margin: 0, paddingLeft: "20px", fontSize: "14px" }}>
              <li>构建多项式 a: 输入 "3,2,2,1,1,0" (3x²+2x+1)</li>
              <li>构建多项式 b: 输入 "1,1,-1,0" (x-1)</li>
              <li>计算 a+b: 输入 "a+b"</li>
              <li>计算 a*b: 输入 "a*b"</li>
              <li>多项式求值: 选择多项式 a, 输入 x=2</li>
              <li>多项式求导: 选择多项式 a, 点击"求导"按钮</li>
            </ul>
          </div>
        </div>
      )}
    </main>
  );
}

export default App;
