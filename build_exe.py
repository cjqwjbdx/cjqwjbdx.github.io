#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32 固件烧录工具 - 打包脚本
使用 PyInstaller 打包为 EXE
"""

import os
import sys
import subprocess

def build_exe():
    """打包为EXE"""

    # 确保已安装 pyinstaller
    try:
        import PyInstaller
    except ImportError:
        print("正在安装 PyInstaller...")
        subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pyinstaller'])

    # 打包命令
    cmd = [
        'pyinstaller',
        '--name=FlashTool',
        '--windowed',  # 无控制台窗口
        '--onefile',   # 单文件
        '--icon=flash.ico',  # 图标（如果有）
        'flash_tool.py'
    ]

    # 如果没有图标文件，移除图标选项
    if not os.path.exists('flash.ico'):
        cmd.remove('--icon=flash.ico')
        print("未找到图标文件，跳过图标设置")

    print("开始打包...")
    print(f"执行命令: {' '.join(cmd)}")

    try:
        subprocess.check_call(cmd)
        print("\n✅ 打包成功！")
        print(f"EXE 文件位置: {os.path.abspath('dist/FlashTool.exe')}")
        print("\n使用方法:")
        print("1. 将 FlashTool.exe 复制到任意位置")
        print("2. 双击运行")
        print("3. 确保电脑已安装 PlatformIO")
    except subprocess.CalledProcessError as e:
        print(f"\n❌ 打包失败: {e}")
        sys.exit(1)

if __name__ == '__main__':
    build_exe()
