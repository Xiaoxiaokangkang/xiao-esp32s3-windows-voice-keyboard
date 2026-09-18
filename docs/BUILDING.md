# 构建与刷写

## 环境

- Windows 10/11
- Espressif ESP-IDF 5.5.5
- Python 3
- 一根支持数据传输的 USB-C 线

每个版本都是独立 ESP-IDF 工程。进入对应 `variants/<版本>/source` 目录后运行：

```powershell
idf.py set-target esp32s3
idf.py build
```

普通刷写：

```powershell
idf.py -p COM6 flash
```

将 `COM6` 替换为设备管理器中的实际串口。

## 进入下载模式

正常固件会把 USB 端口用作麦克风和键盘，因此可能看不到串口。保持 USB 连接，按住 BOOT，短按 RESET，松开 BOOT，再查看新的 COM 端口。刷写结束后松开 BOOT，仅短按一次 RESET。

## 预编译固件

只有 V1 附带已经验证的预编译固件和刷写脚本。V2、V3 的功能代码是本次整理新增内容，在完成 ESP-IDF 编译和目标硬件验证前不发布二进制镜像。

## 验证建议

1. 确认 Windows 同时识别 HID 键盘和 `Voice Keyboard Audio` 麦克风。
2. 连续开始、停止语音输入至少 20 次，确认第二次以后仍有非零音频。
3. 分别检查三个按键，每次操作只产生一次预期事件。
4. V2 长按 K3 时，确认仅清空当前输入框，不影响剪贴板。
5. V3 安装本地助手后，确认 K1 只聚焦 TraeWork CN，K2/K3 映射正确。

