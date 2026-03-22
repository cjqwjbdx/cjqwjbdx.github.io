import os
import shutil
from pathlib import Path

# 项目根目录
project_dir = Path(__file__).parent.resolve()

# 源文件和目标文件
source_file = project_dir / "dist" / "wujinos_1.1.7_z98.bin"
target_dir = project_dir / "src" / "newbin"
target_file = target_dir / "firmware.bin"

# 确保目标目录存在
target_dir.mkdir(parents=True, exist_ok=True)

# 复制文件
try:
    shutil.copy2(source_file, target_file)
    size = target_file.stat().st_size
    print(f"✅ 固件复制成功！")
    print(f"源文件: {source_file}")
    print(f"目标文件: {target_file}")
    print(f"文件大小: {size / (1024*1024):.2f} MB")
    print(f"\n烧录命令:")
    print(f"  esptool.py --chip esp32 --port COM3 --baud 460800 write_flash 0x10000 src\\newbin\\firmware.bin")
except Exception as e:
    print(f"❌ 复制失败: {e}")
