$ErrorActionPreference = "Stop"

Write-Host "Expected application device: VID_303A&PID_4010"
pnputil /enum-devices /connected |
    Select-String -Pattern "VID_303A&PID_4010|VID_303A&PID_1001|Voice Keyboard" -Context 0,6

Write-Host "`nSerial ports:"
[System.IO.Ports.SerialPort]::GetPortNames()

Write-Host "`nPID_4010 means the keyboard + microphone application is running."
Write-Host "PID_1001 means the XIAO is still in ROM download mode."
