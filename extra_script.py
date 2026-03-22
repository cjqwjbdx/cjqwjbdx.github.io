import os
import shutil
from pathlib import Path

Import("env")

def post_build(source, target, env):
    """编译后处理：将编译好的三个固件复制到 src/newbin"""
    project_dir = Path(env.subst("$PROJECT_DIR"))
    build_dir = project_dir / ".pio" / "build" / env["PIOENV"]
    target_dir = project_dir / "src" / "newbin"
    
    target_dir.mkdir(parents=True, exist_ok=True)
    
    # 复制三个固件文件
    firmware_files = {
        "bootloader.bin": "0x1000",
        "partitions.bin": "0x8000",
        "firmware.bin": "0x10000"
    }
    
    for filename, address in firmware_files.items():
        src_file = build_dir / filename
        dst_file = target_dir / filename
        
        if src_file.exists():
            shutil.copy2(src_file, dst_file)
            size = dst_file.stat().st_size
            print(f"[POST_BUILD] {filename} 已复制 (地址: {address}, 大小: {size / 1024:.2f} KB)")
        else:
            print(f"[POST_BUILD] 警告: 找不到 {filename}")
    
    print(f"\n[POST_BUILD] 所有固件已复制到: {target_dir}")
    print(f"[POST_BUILD] 烧录命令:")
    print(f"           esptool.py --chip esp32 --port COM3 --baud 460800 write_flash \\")
    print(f"           0x1000 {target_dir / 'bootloader.bin'} \\")
    print(f"           0x8000 {target_dir / 'partitions.bin'} \\")
    print(f"           0x10000 {target_dir / 'firmware.bin'}")

# 注册构建后动作
env.AddPostAction("buildprog", post_build)
