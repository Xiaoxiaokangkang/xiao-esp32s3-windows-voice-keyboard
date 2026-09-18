[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$Helper = Join-Path $PSScriptRoot 'TraeFocusHelper.ps1'
$Startup = [Environment]::GetFolderPath('Startup')
$ShortcutPath = Join-Path $Startup 'XIAO TraeWork Focus Helper.lnk'
$WindowsPowerShell = Join-Path $env:SystemRoot 'System32\WindowsPowerShell\v1.0\powershell.exe'

if (-not (Test-Path -LiteralPath $WindowsPowerShell)) {
    throw "Windows PowerShell was not found: $WindowsPowerShell"
}

& $WindowsPowerShell -NoLogo -NoProfile -ExecutionPolicy Bypass -File $Helper -ValidateOnly
if ($LASTEXITCODE -ne 0) { throw 'TraeWork helper validation failed.' }

$Shell = New-Object -ComObject WScript.Shell
$Shortcut = $Shell.CreateShortcut($ShortcutPath)
$Shortcut.TargetPath = $WindowsPowerShell
$Shortcut.Arguments = "-NoLogo -NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File `"$Helper`""
$Shortcut.WorkingDirectory = $PSScriptRoot
$Shortcut.Description = 'Listens for XIAO keyboard F13 and focuses TraeWork CN'
$Shortcut.Save()

Start-Process -FilePath $WindowsPowerShell -WindowStyle Hidden -ArgumentList @(
    '-NoLogo', '-NoProfile', '-WindowStyle', 'Hidden',
    '-ExecutionPolicy', 'Bypass', '-File', "`"$Helper`""
)

Write-Host "TraeWork helper installed for the current user: $ShortcutPath" -ForegroundColor Green

