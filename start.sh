#!/bin/bash

# 图书管理系统启动脚本

DEFAULT_API_PORT=8080
DEFAULT_WEB_PORT=3000
PORT_SCAN_LIMIT=50

is_port_in_use() {
    local port="$1"
    lsof -nP -iTCP:"${port}" -sTCP:LISTEN >/dev/null 2>&1
}

find_available_port() {
    local start_port="$1"
    local limit="$2"
    local port="$start_port"
    local count=0

    while [ "$count" -lt "$limit" ]; do
        if ! is_port_in_use "$port"; then
            echo "$port"
            return 0
        fi
        port=$((port + 1))
        count=$((count + 1))
    done

    return 1
}

wait_for_api_health() {
    local port="$1"
    local retries=20
    local count=0

    while [ "$count" -lt "$retries" ]; do
        if curl -fsS "http://localhost:${port}/api/health" >/dev/null 2>&1; then
            return 0
        fi
        sleep 1
        count=$((count + 1))
    done

    return 1
}

echo "================================"
echo "图书管理系统 - 启动脚本"
echo "================================"
echo ""

REQUESTED_API_PORT=${API_PORT:-$DEFAULT_API_PORT}
REQUESTED_WEB_PORT=${WEB_PORT:-$DEFAULT_WEB_PORT}

API_PORT=$(find_available_port "$REQUESTED_API_PORT" "$PORT_SCAN_LIMIT")
if [ -z "$API_PORT" ]; then
    echo "❌ 无法找到可用的 API 端口（从 ${REQUESTED_API_PORT} 开始扫描 ${PORT_SCAN_LIMIT} 个端口）"
    exit 1
fi

if [ "$API_PORT" != "$REQUESTED_API_PORT" ]; then
    echo "⚠️  API端口 ${REQUESTED_API_PORT} 被占用，已自动切换到 ${API_PORT}"
fi

# 检查数据库
if [ ! -f "library.db" ]; then
    echo "⚠️  数据库不存在，正在初始化..."
    sqlite3 library.db << 'EOF'
-- Users table
CREATE TABLE IF NOT EXISTS Users (
    id TEXT PRIMARY KEY,
    username TEXT UNIQUE NOT NULL,
    name TEXT,
    college TEXT,
    className TEXT,
    role TEXT NOT NULL,
    password TEXT NOT NULL,
    recoveryToken TEXT
);

-- Books table
CREATE TABLE IF NOT EXISTS Books (
    isbn TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    author TEXT NOT NULL,
    publisher TEXT,
    category TEXT,
    totalCopies INTEGER NOT NULL,
    availableCopies INTEGER NOT NULL
);

-- BorrowRecords table
CREATE TABLE IF NOT EXISTS BorrowRecords (
    recordId INTEGER PRIMARY KEY AUTOINCREMENT,
    userId TEXT NOT NULL,
    bookIsbn TEXT NOT NULL,
    borrowDate TEXT NOT NULL,
    dueDate TEXT NOT NULL,
    returnDate TEXT,
    FOREIGN KEY (userId) REFERENCES Users(id),
    FOREIGN KEY (bookIsbn) REFERENCES Books(isbn)
);

-- Logs table
CREATE TABLE IF NOT EXISTS Logs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    timestamp TEXT NOT NULL,
    level TEXT NOT NULL,
    message TEXT NOT NULL,
    user TEXT,
    action TEXT
);

-- Create indexes
CREATE INDEX IF NOT EXISTS idx_borrow_user ON BorrowRecords(userId);
CREATE INDEX IF NOT EXISTS idx_borrow_book ON BorrowRecords(bookIsbn);
CREATE INDEX IF NOT EXISTS idx_borrow_return ON BorrowRecords(returnDate);
CREATE INDEX IF NOT EXISTS idx_logs_timestamp ON Logs(timestamp);
CREATE INDEX IF NOT EXISTS idx_logs_user ON Logs(user);
CREATE INDEX IF NOT EXISTS idx_logs_level ON Logs(level);
CREATE INDEX IF NOT EXISTS idx_logs_action ON Logs(action);

-- Insert default admin user
INSERT OR IGNORE INTO Users (id, username, name, college, className, role, password, recoveryToken)
VALUES ('admin', 'admin', '管理员', '', '', 'ADMIN', 'admin', NULL);
EOF
    echo "✅ 数据库初始化完成"
    echo ""
fi

# 启动API服务器
echo "🚀 启动API服务器 (端口 ${API_PORT})..."
cd api-server
PORT="$API_PORT" npm start &
API_PID=$!
cd ..

if ! wait_for_api_health "$API_PORT"; then
    echo "❌ API服务器启动失败，请检查上方日志。"
    kill "$API_PID" 2>/dev/null
    wait "$API_PID" 2>/dev/null
    exit 1
fi

WEB_PORT=$(find_available_port "$REQUESTED_WEB_PORT" "$PORT_SCAN_LIMIT")
if [ -z "$WEB_PORT" ]; then
    echo "❌ 无法找到可用的前端端口（从 ${REQUESTED_WEB_PORT} 开始扫描 ${PORT_SCAN_LIMIT} 个端口）"
    kill "$API_PID" 2>/dev/null
    wait "$API_PID" 2>/dev/null
    exit 1
fi

if [ "$WEB_PORT" != "$REQUESTED_WEB_PORT" ]; then
    echo "⚠️  前端端口 ${REQUESTED_WEB_PORT} 被占用，已自动切换到 ${WEB_PORT}"
fi

# 启动前端开发服务器
echo "🚀 启动前端开发服务器 (端口 ${WEB_PORT})..."
cd webui
VITE_API_TARGET="http://localhost:${API_PORT}" npm run dev -- --port "$WEB_PORT" --strictPort &
WEB_PID=$!
cd ..

sleep 1
if ! kill -0 "$WEB_PID" 2>/dev/null; then
    echo "❌ 前端开发服务器启动失败，请检查上方日志。"
    kill "$API_PID" 2>/dev/null
    wait "$API_PID" 2>/dev/null
    exit 1
fi

echo ""
echo "================================"
echo "✅ 系统启动成功！"
echo "================================"
echo ""
echo "📱 前端地址: http://localhost:${WEB_PORT}"
echo "🔌 API地址: http://localhost:${API_PORT}"
echo ""
echo "👤 默认管理员账号:"
echo "   用户名: admin"
echo "   密码: admin"
echo ""
echo "💡 提示: 如果默认端口被占用，脚本会自动切换到可用端口。"
echo ""
echo "按 Ctrl+C 停止所有服务"
echo ""

# 等待用户中断
cleanup() {
    echo ""
    echo "正在停止服务..."
    if kill -0 "$WEB_PID" 2>/dev/null; then
        kill "$WEB_PID" 2>/dev/null
        wait "$WEB_PID" 2>/dev/null
    fi
    if kill -0 "$API_PID" 2>/dev/null; then
        kill "$API_PID" 2>/dev/null
        wait "$API_PID" 2>/dev/null
    fi
}

trap "cleanup; exit 0" INT TERM

wait
