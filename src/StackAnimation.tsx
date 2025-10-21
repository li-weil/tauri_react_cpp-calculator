import { useState, useEffect } from 'react';

interface StackOperation {
  timestamp: number;
  operation_type: number;  // 0: 出栈, 1: 入栈
  value: number;
  stack_type: string;      // "num" 或 "sym"
}

interface StackState {
  num_stack: number[];
  sym_stack: string[];
}

function StackAnimation({ operations }: { operations: StackOperation[] }) {
  const [currentStep, setCurrentStep] = useState(0);
  const [isPlaying, setIsPlaying] = useState(false);
  const [stackState, setStackState] = useState<StackState>({
    num_stack: [],
    sym_stack: []
  });
  const [animatingStack, setAnimatingStack] = useState<{ type: string; index: number } | null>(null);

  // 根据操作计算栈状态
  const calculateStackState = (stepIndex: number) => {
    const newState: StackState = { num_stack: [], sym_stack: [] };

    for (let i = 0; i <= stepIndex && i < operations.length; i++) {
      const op = operations[i];

      if (op.stack_type === 'num') {
        if (op.operation_type === 1) { // 入栈
          newState.num_stack.push(op.value);
        } else if (op.operation_type === 0 && newState.num_stack.length > 0) { // 出栈
          newState.num_stack.pop();
        }
      } else if (op.stack_type === 'sym') {
        const symbol = String.fromCharCode(op.value);
        if (op.operation_type === 1) { // 入栈
          newState.sym_stack.push(symbol);
        } else if (op.operation_type === 0 && newState.sym_stack.length > 0) { // 出栈
          newState.sym_stack.pop();
        }
      }
    }

    return newState;
  };

  // 更新栈状态
  useEffect(() => {
    const newState = calculateStackState(currentStep);
    setStackState(newState);

    // 设置动画效果
    if (currentStep > 0 && currentStep <= operations.length) {
      const currentOp = operations[currentStep - 1];
      setAnimatingStack({ type: currentOp.stack_type, index: currentStep });

      // 清除动画效果
      setTimeout(() => {
        setAnimatingStack(null);
      }, 500);
    }
  }, [currentStep, operations]);

  // 自动播放
  useEffect(() => {
    if (isPlaying && currentStep < operations.length) {
      const timer = setTimeout(() => {
        setCurrentStep(prev => prev + 1);
      }, 800); // 每步间隔800ms

      return () => clearTimeout(timer);
    } else if (isPlaying && currentStep >= operations.length) {
      setIsPlaying(false);
    }
  }, [isPlaying, currentStep, operations.length]);

  const handlePlay = () => {
    if (currentStep >= operations.length) {
      setCurrentStep(0);
    }
    setIsPlaying(true);
  };

  const handlePause = () => {
    setIsPlaying(false);
  };

  const handleReset = () => {
    setIsPlaying(false);
    setCurrentStep(0);
  };

  const handleStepForward = () => {
    if (currentStep < operations.length) {
      setCurrentStep(prev => prev + 1);
    }
  };

  const handleStepBackward = () => {
    if (currentStep > 0) {
      setCurrentStep(prev => prev - 1);
    }
  };

  // 获取当前操作描述
  const getCurrentOperation = () => {
    if (currentStep === 0 || currentStep > operations.length) return null;
    const op = operations[currentStep - 1];
    const action = op.operation_type === 1 ? '入栈' : '出栈';
    const value = op.stack_type === 'num' ? op.value : String.fromCharCode(op.value);
    const stackName = op.stack_type === 'num' ? '数字栈' : '符号栈';
    return `${stackName}: ${value} ${action}`;
  };

  return (
    <div className="stack-animation-container">
      <h3>栈操作动画</h3>

      {/* 控制按钮 */}
      <div className="animation-controls">
        <button onClick={handleReset}>重置</button>
        <button onClick={handleStepBackward} disabled={currentStep === 0}>
          上一步
        </button>
        {!isPlaying ? (
          <button onClick={handlePlay} disabled={currentStep >= operations.length}>
            播放
          </button>
        ) : (
          <button onClick={handlePause}>暂停</button>
        )}
        <button onClick={handleStepForward} disabled={currentStep >= operations.length}>
          下一步
        </button>
      </div>

      {/* 当前操作信息 */}
      <div className="current-operation">
        <strong>步骤 {currentStep}/{operations.length}:</strong>
        {getCurrentOperation() || '等待开始...'}
      </div>

      {/* 栈显示 */}
      <div className="stacks-display">
        {/* 数字栈 */}
        <div className="stack-container">
          <h4>数字栈</h4>
          <div className="stack">
            {stackState.num_stack.length === 0 ? (
              <div className="stack-item empty">空</div>
            ) : (
              stackState.num_stack.map((value, index) => (
                <div
                  key={index}
                  className={`stack-item num-item ${
                    animatingStack?.type === 'num' && index === stackState.num_stack.length - 1
                      ? 'animating'
                      : ''
                  }`}
                >
                  {value}
                </div>
              ))
            )}
          </div>
        </div>

        {/* 符号栈 */}
        <div className="stack-container">
          <h4>符号栈</h4>
          <div className="stack">
            {stackState.sym_stack.length === 0 ? (
              <div className="stack-item empty">空</div>
            ) : (
              stackState.sym_stack.map((symbol, index) => (
                <div
                  key={index}
                  className={`stack-item sym-item ${
                    animatingStack?.type === 'sym' && index === stackState.sym_stack.length - 1
                      ? 'animating'
                      : ''
                  }`}
                >
                  {symbol}
                </div>
              ))
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

export default StackAnimation;