# V1：三键硬件，K1 语音输入

这是最小且已经实机验证的版本。

- K1（D2/GPIO3）：按住时发送左 Ctrl + 左 Windows，调用微信输入法语音输入；松开时结束。
- K2（D1/GPIO2）：预留。
- K3（D0/GPIO1）：预留。

`firmware/` 包含已经验证的预编译镜像，`tools/flash_firmware.ps1` 可用于刷写。详细接线、配置和验证数据见 `source/README.md` 与 `VALIDATION.txt`。

```powershell
py -m pip install esptool==4.12.0
.\tools\flash_firmware.ps1 -Port COM6
```

