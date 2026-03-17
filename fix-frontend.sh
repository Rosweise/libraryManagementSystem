#!/bin/bash

echo "🔧 修复前端依赖问题..."
echo ""

cd /Volumes/Rita/libraryManagementSystem/webui

# 停止现有的vite进程
echo "1. 停止现有服务器..."
pkill -f "vite" 2>/dev/null
sleep 2

# 安装缺失的依赖
echo "2. 安装缺失的依赖..."
npm install vue-i18n@9

# 确保所有依赖都已 installed
echo "3. 验证依赖..."
npm install

echo ""
echo "✅ 依赖安装完成！"
echo ""
echo "现在启动前端服务器..."
npm run dev

