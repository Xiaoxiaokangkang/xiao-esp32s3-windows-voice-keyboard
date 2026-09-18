# V3：三键 TraeWork CN 语音输入

- K1（D2/GPIO3）：发送 F13；Windows 助手接收后启动或聚焦 TraeWork CN，并发送 `Ctrl+U` 聚焦聊天输入框。
- K2（D1/GPIO2）：按住调用微信输入法语音输入，松开结束。
- K3（D0/GPIO1）：短按发送 Enter。

## Windows 助手

先确认 TraeWork CN 安装在默认位置，再在普通 PowerShell 中运行：

```powershell
.\windows\install.ps1
```

安装器只创建当前用户的启动快捷方式，不需要管理员权限。助手不读取聊天内容、不处理麦克风音频，也不连接网络。

本目录目前不附带预编译固件。请按仓库根目录 `docs/BUILDING.md` 从 `source/` 构建并完成实机验证。

