param(
    [Parameter(Mandatory = $true)]
    [string]$Port,
    [string]$Python = "py"
)

$ErrorActionPreference = "Stop"
$packageRoot = Split-Path -Parent $PSScriptRoot
$firmware = Join-Path $packageRoot "firmware"

$bootloader = Join-Path $firmware "bootloader.bin"
$partitions = Join-Path $firmware "partition-table.bin"
$application = Join-Path $firmware "xiao_voice_keyboard.bin"

foreach ($file in @($bootloader, $partitions, $application)) {
    if (-not (Test-Path -LiteralPath $file)) {
        throw "Missing firmware file: $file"
    }
}

Write-Host "Flashing XIAO ESP32S3 on $Port..."
& $Python -m esptool --chip esp32s3 -p $Port -b 460800 `
    --before default_reset --after hard_reset write_flash `
    --flash_mode dio --flash_freq 80m --flash_size 8MB `
    0x0 $bootloader `
    0x8000 $partitions `
    0x10000 $application

if ($LASTEXITCODE -ne 0) {
    throw "Flashing failed with exit code $LASTEXITCODE"
}

Write-Host "Flash completed. If COM remains visible, release BOOT and tap RESET once."
