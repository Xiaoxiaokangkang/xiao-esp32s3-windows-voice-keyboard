# 版本说明

三个版本使用相同的 XIAO ESP32S3、INMP441 接线和 USB 麦克风实现，区别只在按键映射和 V3 的本地 Windows 助手。V1–V3 是与 macOS 仓库统一的课程功能编号，不表示操作系统或硬件批次。

## V1：K1 语音输入

- K1：按住时依次按下左 Ctrl、左 Windows，松开时反向释放。
- K2、K3：已消抖读取，但不执行动作。
- 适合先验证 USB 麦克风和微信输入法全局语音快捷键。

## V2：语音、发送、取消

- K1：按住语音输入，松开结束。
- K2：短按发送 Enter。
- K3：长按约 1.5 秒发送 `Ctrl+A`、`Backspace`，清空当前输入框；每次按住只触发一次。
- 请先松开 K1，再使用 K2 或 K3，避免组合键重叠。

## V3：TraeWork CN

- K1：发送 F13，由 Windows 助手启动或聚焦 TraeWork CN，并调用 `Ctrl+U` 聚焦聊天输入框。
- K2：按住调用微信输入法语音输入，松开结束。
- K3：短按发送 Enter。
- Windows 助手没有管理员权限要求，也不处理麦克风音频。

## USB 标识

| 版本 | PID | 产品名 | 序列号 |
|---|---:|---|---|
| V1 | `0x4010` | `XIAO Voice Keyboard` | `XIAO-S3-VOICE-01` |
| V2 | `0x4011` | `XIAO Voice Keyboard V2` | `XIAO-S3-VOICE-02` |
| V3 | `0x4012` | `XIAO Voice Keyboard V3 TraeWork` | `XIAO-S3-VOICE-03` |

不同 PID 和序列号可避免 Windows 复用旧版 USB 描述符缓存。
