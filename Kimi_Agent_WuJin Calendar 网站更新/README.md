# WuJin Calendar 网站

## 网站结构

- **index.html** - 主页，展示产品简介和进入商城的按钮
- **introshop.html** - 商城页面，包含完整的产品展示和购买功能
- **CNAME** - 域名配置文件，指定 wjbdx.qzz.io

## 页面功能

### 主页 (index.html)
- 产品品牌展示
- 核心功能介绍（墨水屏显示、超长续航、多种配色）
- 进入商城按钮
- 响应式设计，支持移动端

### 商城页面 (introshop.html)
- 完整的产品介绍
- 产品功能展示
- 规格参数
- 颜色选择
- 购买按钮（跳转到闲鱼）
- 导航栏包含"返回主页"按钮

## 部署说明

1. 确保所有文件都已上传到 GitHub
2. CNAME 文件已配置为 wjbdx.qzz.io
3. GitHub Pages 会自动部署网站

## 开发说明

商城页面基于 React + Vite 构建，如需修改：
```bash
cd app
npm install
npm run dev
npm run build
```

构建后的文件在 `app/dist/` 目录，`introshop.html` 会引用这些文件。
