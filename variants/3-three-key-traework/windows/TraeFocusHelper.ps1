[CmdletBinding()]
param(
    [string]$TraeExe = "$env:LOCALAPPDATA\Programs\TRAE SOLO CN\TRAE SOLO CN.exe",
    [switch]$ValidateOnly
)

$ErrorActionPreference = 'Stop'

if (-not (Test-Path -LiteralPath $TraeExe)) {
    throw "TraeWork CN executable not found: $TraeExe"
}

if ($ValidateOnly) {
    Write-Host 'TraeWork focus helper validation passed.' -ForegroundColor Green
    return
}

$createdNew = $false
$mutex = [Threading.Mutex]::new($true, 'Local\XiaoTraeWorkFocusHelper', [ref]$createdNew)
if (-not $createdNew) { return }

Add-Type -TypeDefinition @'
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading;

public static class TraeHotkeyHost
{
    private const int WM_HOTKEY = 0x0312;
    private const int HOTKEY_ID = 0x584B;
    private const uint VK_F13 = 0x7C;
    private const int SW_RESTORE = 9;
    private const uint KEYEVENTF_KEYUP = 0x0002;
    private const byte VK_CONTROL = 0x11;
    private const byte VK_U = 0x55;

    [StructLayout(LayoutKind.Sequential)]
    private struct POINT { public int X; public int Y; }

    [StructLayout(LayoutKind.Sequential)]
    private struct MSG {
        public IntPtr hwnd;
        public uint message;
        public UIntPtr wParam;
        public IntPtr lParam;
        public uint time;
        public POINT pt;
    }

    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool RegisterHotKey(IntPtr hWnd, int id, uint modifiers, uint key);
    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool UnregisterHotKey(IntPtr hWnd, int id);
    [DllImport("user32.dll")]
    private static extern int GetMessage(out MSG msg, IntPtr hWnd, uint min, uint max);
    [DllImport("user32.dll")]
    private static extern bool ShowWindow(IntPtr hWnd, int command);
    [DllImport("user32.dll")]
    private static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")]
    private static extern bool BringWindowToTop(IntPtr hWnd);
    [DllImport("user32.dll")]
    private static extern IntPtr GetForegroundWindow();
    [DllImport("user32.dll")]
    private static extern uint GetWindowThreadProcessId(IntPtr hWnd, IntPtr processId);
    [DllImport("kernel32.dll")]
    private static extern uint GetCurrentThreadId();
    [DllImport("user32.dll")]
    private static extern bool AttachThreadInput(uint from, uint to, bool attach);
    [DllImport("user32.dll")]
    private static extern void keybd_event(byte virtualKey, byte scanCode, uint flags, UIntPtr extraInfo);

    public static void Run(string exePath)
    {
        if (!RegisterHotKey(IntPtr.Zero, HOTKEY_ID, 0, VK_F13))
            throw new InvalidOperationException("F13 is already registered by another program.");

        try {
            MSG msg;
            while (GetMessage(out msg, IntPtr.Zero, 0, 0) > 0) {
                if (msg.message == WM_HOTKEY && msg.wParam.ToUInt32() == HOTKEY_ID)
                    ActivateTrae(exePath);
            }
        }
        finally { UnregisterHotKey(IntPtr.Zero, HOTKEY_ID); }
    }

    private static void ActivateTrae(string exePath)
    {
        Process target = FindMainWindow();
        if (target == null) {
            Process.Start(new ProcessStartInfo(exePath) { UseShellExecute = true });
            for (int i = 0; i < 100 && target == null; ++i) {
                Thread.Sleep(100);
                target = FindMainWindow();
            }
        }
        if (target == null || target.MainWindowHandle == IntPtr.Zero) return;

        IntPtr window = target.MainWindowHandle;
        ShowWindow(window, SW_RESTORE);
        IntPtr foreground = GetForegroundWindow();
        uint foregroundThread = GetWindowThreadProcessId(foreground, IntPtr.Zero);
        uint currentThread = GetCurrentThreadId();
        bool attached = foregroundThread != 0 && foregroundThread != currentThread &&
                        AttachThreadInput(currentThread, foregroundThread, true);
        try {
            BringWindowToTop(window);
            SetForegroundWindow(window);
        }
        finally {
            if (attached) AttachThreadInput(currentThread, foregroundThread, false);
        }

        Thread.Sleep(180);
        keybd_event(VK_CONTROL, 0, 0, UIntPtr.Zero);
        keybd_event(VK_U, 0, 0, UIntPtr.Zero);
        keybd_event(VK_U, 0, KEYEVENTF_KEYUP, UIntPtr.Zero);
        keybd_event(VK_CONTROL, 0, KEYEVENTF_KEYUP, UIntPtr.Zero);
    }

    private static Process FindMainWindow()
    {
        Process best = null;
        foreach (Process process in Process.GetProcessesByName("TRAE SOLO CN")) {
            try {
                if (process.MainWindowHandle != IntPtr.Zero &&
                    (best == null || process.StartTime < best.StartTime))
                    best = process;
            } catch { }
        }
        return best;
    }
}
'@

try {
    [TraeHotkeyHost]::Run($TraeExe)
}
finally {
    $mutex.ReleaseMutex()
    $mutex.Dispose()
}
