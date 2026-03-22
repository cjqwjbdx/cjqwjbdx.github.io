#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
ESP32 固件烧录工具
"""

import sys
import os
import subprocess
import threading
import json
from pathlib import Path

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QLabel, QPushButton, QComboBox, QLineEdit, QTextEdit,
    QProgressBar, QFileDialog, QMessageBox, QDialog
)
from PyQt6.QtCore import Qt, QThread, pyqtSignal, QTimer
from PyQt6.QtGui import QFont

try:
    import serial.tools.list_ports
except ImportError:
    serial = None


class BuildThread(QThread):
    log_signal = pyqtSignal(str)
    finished_signal = pyqtSignal(bool, str)

    def __init__(self, project_path, env_name):
        super().__init__()
        self.project_path = project_path
        self.env_name = env_name

    def run(self):
        try:
            process = subprocess.Popen(
                ['pio', 'run', '-e', self.env_name],
                cwd=self.project_path,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding='utf-8',
                errors='replace'
            )

            for line in process.stdout:
                self.log_signal.emit(line.strip())

            result = process.wait()
            self.finished_signal.emit(result == 0, "编译")
        except Exception as e:
            self.log_signal.emit(f"错误: {str(e)}")
            self.finished_signal.emit(False, "操作")


class FlashThread(QThread):
    log_signal = pyqtSignal(str)
    finished_signal = pyqtSignal(bool)

    def __init__(self, project_path, env_name, port, baud_rate):
        super().__init__()
        self.project_path = project_path
        self.env_name = env_name
        self.port = port
        self.baud_rate = baud_rate

    def run(self):
        try:
            self.log_signal.emit("=== 开始编译 ===")
            process = subprocess.Popen(
                ['pio', 'run', '-e', self.env_name],
                cwd=self.project_path,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding='utf-8',
                errors='replace'
            )

            for line in process.stdout:
                self.log_signal.emit(line.strip())

            result = process.wait()
            if result != 0:
                self.finished_signal.emit(False)
                return

            self.log_signal.emit("=== 开始烧录 ===")
            process = subprocess.Popen(
                [
                    'pio', 'run', '-t', 'upload',
                    '-e', self.env_name,
                    f'--upload-port={self.port}',
                    f'--upload-speed={self.baud_rate}'
                ],
                cwd=self.project_path,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                encoding='utf-8',
                errors='replace'
            )

            for line in process.stdout:
                self.log_signal.emit(line.strip())

            result = process.wait()
            self.finished_signal.emit(result == 0)
        except Exception as e:
            self.log_signal.emit(f"错误: {str(e)}")
            self.finished_signal.emit(False)


class LogWindow(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("日志")
        self.setFixedSize(600, 400)

        layout = QVBoxLayout(self)

        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setFont(QFont("Consolas", 9))
        layout.addWidget(self.log_text)

        btn = QPushButton("关闭")
        btn.clicked.connect(self.close)
        layout.addWidget(btn)

    def append_log(self, message):
        self.log_text.append(message)
        self.log_text.verticalScrollBar().setValue(self.log_text.verticalScrollBar().maximum())

    def clear_log(self):
        self.log_text.clear()


class FlashTool(QMainWindow):
    def __init__(self):
        super().__init__()

        self.config = self.load_config()
        self.build_thread = None
        self.flash_thread = None
        self.log_window = None

        self.init_ui()
        self.detect_ports()

    def load_config(self):
        config_file = 'config.json'
        if os.path.exists(config_file):
            try:
                with open(config_file, 'r', encoding='utf-8') as f:
                    return json.load(f)
            except:
                pass
        return {
            'project_path': '',
            'env_name': 'z98',
            'serial_port': '',
            'baud_rate': '460800'
        }

    def save_config(self):
        self.config = {
            'project_path': self.project_input.text(),
            'env_name': self.env_combo.currentText(),
            'serial_port': self.port_combo.currentText(),
            'baud_rate': self.baud_combo.currentText()
        }
        with open('config.json', 'w', encoding='utf-8') as f:
            json.dump(self.config, f, indent=2, ensure_ascii=False)

    def init_ui(self):
        self.setWindowTitle("ESP32 固件烧录工具")
        self.setFixedSize(400, 250)

        central_widget = QWidget()
        self.setCentralWidget(central_widget)

        main_layout = QVBoxLayout(central_widget)
        main_layout.setSpacing(10)
        main_layout.setContentsMargins(15, 15, 15, 15)

        # 项目路径
        path_layout = QHBoxLayout()
        path_layout.addWidget(QLabel("项目路径:"))
        self.project_input = QLineEdit()
        self.project_input.setText(self.config.get('project_path', ''))
        path_layout.addWidget(self.project_input)
        browse_btn = QPushButton("浏览")
        browse_btn.setFixedWidth(60)
        browse_btn.clicked.connect(self.select_project)
        path_layout.addWidget(browse_btn)
        main_layout.addLayout(path_layout)

        # 配置行
        config_layout = QHBoxLayout()
        config_layout.addWidget(QLabel("环境:"))
        self.env_combo = QComboBox()
        self.env_combo.addItems(['z98', 'z15', 'z21'])
        self.env_combo.setCurrentText(self.config.get('env_name', 'z98'))
        config_layout.addWidget(self.env_combo)

        refresh_btn = QPushButton("刷新")
        refresh_btn.setFixedWidth(60)
        refresh_btn.clicked.connect(self.detect_ports)
        config_layout.addWidget(refresh_btn)

        config_layout.addWidget(QLabel("串口:"))
        self.port_combo = QComboBox()
        config_layout.addWidget(self.port_combo)
        main_layout.addLayout(config_layout)

        baud_layout = QHBoxLayout()
        baud_layout.addWidget(QLabel("波特率:"))
        self.baud_combo = QComboBox()
        self.baud_combo.addItems(['460800', '921600', '115200', '230400'])
        self.baud_combo.setCurrentText(self.config.get('baud_rate', '460800'))
        baud_layout.addWidget(self.baud_combo)
        main_layout.addLayout(baud_layout)

        # 按钮
        button_layout = QHBoxLayout()
        self.build_btn = QPushButton("编译")
        self.build_btn.clicked.connect(self.build_firmware)
        button_layout.addWidget(self.build_btn)

        self.export_btn = QPushButton("导出bin")
        self.export_btn.clicked.connect(self.export_bin)
        button_layout.addWidget(self.export_btn)

        self.log_btn = QPushButton("查看日志")
        self.log_btn.clicked.connect(self.show_log_window)
        button_layout.addWidget(self.log_btn)
        main_layout.addLayout(button_layout)

        # 进度条
        self.progress = QProgressBar()
        self.progress.setMaximumHeight(10)
        self.progress.setTextVisible(False)
        main_layout.addWidget(self.progress)

        # 状态
        self.status_label = QLabel("准备就绪")
        main_layout.addWidget(self.status_label)

    def select_project(self):
        path = QFileDialog.getExistingDirectory(self, "选择项目目录")
        if path:
            self.project_input.setText(path)
            self.log(f"项目路径: {path}")
            self.save_config()

    def show_log_window(self):
        if self.log_window is None:
            self.log_window = LogWindow(self)
        self.log_window.show()
        self.log_window.raise_()
        self.log_window.activateWindow()

    def detect_ports(self):
        self.port_combo.clear()

        if serial is None:
            self.log("警告: 未安装pyserial库")
            return

        ports = [port.device for port in serial.tools.list_ports.comports()]

        if ports:
            self.port_combo.addItems(ports)
            self.log(f"检测到串口: {', '.join(ports)}")
            if self.config.get('serial_port') in ports:
                self.port_combo.setCurrentText(self.config['serial_port'])
        else:
            self.log("未检测到串口")

    def log(self, message):
        if self.log_window:
            self.log_window.append_log(message)

    def set_buttons_enabled(self, enabled):
        self.build_btn.setEnabled(enabled)
        self.export_btn.setEnabled(enabled)
        self.log_btn.setEnabled(enabled)

    def build_firmware(self):
        project = self.project_input.text()
        if not project or not os.path.exists(project):
            QMessageBox.critical(self, "错误", "请选择有效的项目路径")
            return

        if not os.path.exists(os.path.join(project, 'platformio.ini')):
            QMessageBox.critical(self, "错误", "未找到platformio.ini")
            return

        self.save_config()
        self.set_buttons_enabled(False)
        self.progress.setRange(0, 0)
        self.status_label.setText("编译中...")

        self.show_log_window()
        if self.log_window:
            self.log_window.clear_log()
        self.log("=== 开始编译 ===")

        self.build_thread = BuildThread(project, self.env_combo.currentText())
        self.build_thread.log_signal.connect(self.log)
        self.build_thread.finished_signal.connect(self.on_build_finished)
        self.build_thread.start()

    def on_build_finished(self, success, operation):
        self.progress.setRange(0, 100)
        self.progress.setValue(100)
        self.set_buttons_enabled(True)

        if success:
            self.status_label.setText(f"{operation}成功")
            self.log(f"{operation}成功")
            QMessageBox.information(self, "成功", f"{operation}完成")
        else:
            self.status_label.setText(f"{operation}失败")
            self.log(f"{operation}失败")
            QMessageBox.critical(self, "失败", f"{operation}失败")

        QTimer.singleShot(2000, lambda: self.progress.setValue(0))

    def export_bin(self):
        project = self.project_input.text()
        if not project or not os.path.exists(project):
            QMessageBox.critical(self, "错误", "请选择有效的项目路径")
            return

        env = self.env_combo.currentText()
        bin_dir = os.path.join(project, '.pio', 'build', env)

        if not os.path.exists(bin_dir):
            QMessageBox.critical(self, "错误", "未找到编译输出目录")
            return

        bin_files = list(Path(bin_dir).glob('*.bin'))

        if not bin_files:
            QMessageBox.critical(self, "错误", "未找到bin文件")
            return

        dest_dir = QFileDialog.getExistingDirectory(self, "选择导出目录")
        if not dest_dir:
            return

        copied = 0
        import shutil
        for bin_file in bin_files:
            dest_file = os.path.join(dest_dir, bin_file.name)
            shutil.copy2(bin_file, dest_file)
            self.log(f"导出: {bin_file.name}")
            copied += 1

        self.log(f"导出完成: {copied} 个文件")
        QMessageBox.information(self, "成功", f"已导出 {copied} 个文件")

    def flash_firmware(self):
        project = self.project_input.text()
        port = self.port_combo.currentText()

        if not project or not os.path.exists(project):
            QMessageBox.critical(self, "错误", "请选择有效的项目路径")
            return

        if not port:
            QMessageBox.critical(self, "错误", "请选择串口")
            return

        self.save_config()
        self.set_buttons_enabled(False)
        self.progress.setRange(0, 0)
        self.status_label.setText("烧录中...")

        self.show_log_window()
        if self.log_window:
            self.log_window.clear_log()

        self.flash_thread = FlashThread(
            project,
            self.env_combo.currentText(),
            port,
            self.baud_combo.currentText()
        )
        self.flash_thread.log_signal.connect(self.log)
        self.flash_thread.finished_signal.connect(self.on_flash_finished)
        self.flash_thread.start()

    def on_flash_finished(self, success):
        self.progress.setRange(0, 100)
        self.progress.setValue(100)
        self.set_buttons_enabled(True)

        if success:
            self.status_label.setText("烧录成功")
            self.log("烧录成功")
            QMessageBox.information(self, "成功", "烧录完成")
        else:
            self.status_label.setText("烧录失败")
            self.log("烧录失败")
            QMessageBox.critical(self, "失败", "烧录失败")

        QTimer.singleShot(2000, lambda: self.progress.setValue(0))

    def closeEvent(self, event):
        if self.build_thread and self.build_thread.isRunning():
            self.build_thread.terminate()
        if self.flash_thread and self.flash_thread.isRunning():
            self.flash_thread.terminate()
        event.accept()


def main():
    app = QApplication(sys.argv)

    font = QFont("Arial", 9)
    app.setFont(font)

    window = FlashTool()
    window.show()

    sys.exit(app.exec())


if __name__ == "__main__":
    main()
