import os
import shutil

base = r"c:/Users/wjbdx/Desktop/Project/墨水屏/jcalendar-1.1.7(mogai)"
website_folder = os.path.join(base, "Kimi_Agent_WuJin Calendar 网站更新")

# 复制文件到根目录
shutil.copy2(os.path.join(website_folder, "index.html"), os.path.join(base, "index.html"))
shutil.copy2(os.path.join(website_folder, "introshop.html"), os.path.join(base, "introshop.html"))
shutil.copy2(os.path.join(website_folder, "README.md"), os.path.join(base, "README.md"))

# 复制app文件夹
app_src = os.path.join(website_folder, "app")
app_dst = os.path.join(base, "app")
if os.path.exists(app_dst):
    shutil.rmtree(app_dst)
shutil.copytree(app_src, app_dst)

print("文件复制完成")
