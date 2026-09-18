$ErrorActionPreference = "Stop"

if (-not (Get-Module -ListAvailable -Name AudioDeviceCmdlets)) {
    throw "AudioDeviceCmdlets is not installed. Run: Install-Module AudioDeviceCmdlets -Scope CurrentUser"
}

Import-Module AudioDeviceCmdlets -Force
$device = Get-AudioDevice -List |
    Where-Object { $_.Type -eq "Recording" -and $_.Name -like "*Voice Keyboard Audio*" } |
    Select-Object -First 1

if (-not $device) {
    throw "Voice Keyboard Audio recording device was not found."
}

Set-AudioDevice -ID $device.ID | Out-Null
Set-AudioDevice -ID $device.ID -CommunicationOnly | Out-Null
Set-AudioDevice -RecordingMute $false | Out-Null
Set-AudioDevice -RecordingVolume 100 | Out-Null

Get-AudioDevice -Recording | Format-List Name, ID, Default, DefaultCommunication
