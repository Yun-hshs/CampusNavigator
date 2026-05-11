# 微信小程序第一版骨架

本次新增了 `weapp/` 与 `backend/` 两个目录：

- `weapp/`：小程序基础页面骨架（登录/首页/路线/个人中心）
- `backend/docs/`：Qt 后端 API 合同文档
- `backend/config/example.env`：后端配置模板

## 运行方式（前端）
1. 使用微信开发者工具导入 `weapp/`
2. AppID 可先使用测试号 `touristappid`
3. 在 `app.js` 中调整 `baseURL` 指向你的 Qt 后端

## 下一步建议
1. 打通 `wx.login -> /api/auth/wx-login`
2. 接入 POI 搜索接口与路线规划接口
3. 补全组件化 UI（搜索栏、POI 卡片、路线卡片）
