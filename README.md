# XIAO ESP32S3 Windows 三键语音键盘

这是一个基于 Seeed Studio XIAO ESP32S3、INMP441 麦克风和 3 个实体按键的 Windows USB 语音键盘项目。设备枚举为标准 USB HID 键盘和 48 kHz / 16-bit / 单声道 USB 麦克风。

本仓库只包含经过整理的 Windows 版本。macOS 版本位于 [xiao-esp32s3-macos-voice-keyboard](https://github.com/Xiaoxiaokangkang/xiao-esp32s3-macos-voice-keyboard)。两个仓库使用相似的目录和版本说明，安装脚本与固件不混用。

## 版本选择

| 顺序 | 版本 | 3 个实体按键的功能 | 当前状态 |
|---:|---|---|---|
| 1 | [V1：K1 语音输入](variants/1-three-key-k1-voice/README.md) | K1 按住语音输入；K2/K3 预留 | 已有实机验证和预编译固件 |
| 2 | [V2：语音、发送、取消](variants/2-three-key-voice-send-cancel/README.md) | K1 语音；K2 发送；K3 长按取消/清空 | 源码已整理，待重新编译和实机验证 |
| 3 | [V3：TraeWork CN](variants/3-three-key-traework/README.md) | K1 调用 TraeWork；K2 语音；K3 发送 | 源码与 Windows 助手已整理，待重新编译和实机验证 |

不要混刷不同版本。V2、V3 没有附带预编译镜像，避免把 V1 的旧镜像误当成新固件。

## 接线

| 部件 | XIAO 引脚 | ESP32-S3 GPIO |
|---|---:|---:|
| K1 | D2 | GPIO3 |
| K2 | D1 | GPIO2 |
| K3 | D0 | GPIO1 |
| INMP441 SD | D8 | GPIO7 |
| INMP441 SCK/BCLK | D9 | GPIO8 |
| INMP441 WS/LRCLK | D10 | GPIO9 |
| INMP441 VDD | 3V3 | — |
| INMP441 GND、L/R | GND | — |

按键按下时向 GPIO 输出 3.3 V 高电平，固件使用内部下拉。ESP32-S3 GPIO 不耐受 5 V。

## 构建

推荐 ESP-IDF 5.5.5。进入所选版本的 `source` 目录后执行：

```powershell
idf.py set-target esp32s3
idf.py build
idf.py -p COM6 flash
```

更完整的构建、刷写和恢复步骤见 [构建说明](docs/BUILDING.md)，版本差异见 [版本说明](docs/VERSIONS.md)。

## 隐私与安全

公开目录已经针对账号凭据、签名私钥、个人日志、私人配置、本机绝对路径和常见 Token 形式进行检查。仓库不需要 Wi-Fi 密码或云 API 密钥，也没有网络上传逻辑。详细边界见 [隐私说明](docs/PRIVACY.md) 和 [安全策略](SECURITY.md)。

V3 的 Windows 助手只监听 F13、查找 TraeWork CN 窗口并发送其本地快捷键；它不读取聊天内容，不上传文件，也不连接网络。

V1 预编译固件的完整性校验值保存在 [SHA256SUMS.txt](SHA256SUMS.txt)。

## 许可

本仓库采用个人非商业用途的 source-available 许可。第三方组件仍遵循各自许可证，详见 [LICENSE](LICENSE) 和 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。
