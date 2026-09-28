param(
    [uint32]$Slices = 3000000000,
    [switch]$Attached
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$release = Join-Path $root 'build\Release'
$viewerExe = Join-Path $release 'hg_opengl_host.exe'
$gameExe = Join-Path $release 'hg_system_diagnostic.exe'
$probe = Join-Path $env:TEMP 'haunting-toc-probe'
$toc = Join-Path $probe 'synthetic-index-pattern.bin'
$preview = Join-Path $env:TEMP 'haunting-live-speaker.ppm'
$stdout = Join-Path $probe 'haunting-live-speaker.out.txt'
$stderr = Join-Path $probe 'haunting-live-speaker.err.txt'

foreach ($path in @($viewerExe, $gameExe, $toc)) {
    if (-not (Test-Path -LiteralPath $path)) { throw "Required launch input is missing: $path" }
}
New-Item -ItemType Directory -Path $probe -Force | Out-Null
Remove-Item -LiteralPath $preview,$stdout,$stderr -ErrorAction SilentlyContinue

if (-not $Attached) {
    # Codex commands execute on a private desktop. Hand a bootstrap to Explorer
    # so the run survives this task; the attached path below explicitly creates
    # the GUI viewer on WinSta0\Default, the user's interactive Windows desktop.
    $launchDir = Join-Path $env:TEMP 'haunting-live-launch'
    New-Item -ItemType Directory -Path $launchDir -Force | Out-Null
    $shell = New-Object -ComObject WScript.Shell
    $shortcutPath = Join-Path $launchDir 'Haunting Ground.lnk'
    $shortcut = $shell.CreateShortcut($shortcutPath)
    $shortcut.TargetPath = (Get-Command powershell.exe).Source
    $shortcut.Arguments = "-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File `"$PSCommandPath`" -Slices $Slices -Attached"
    $shortcut.WorkingDirectory = $root
    $shortcut.IconLocation = "$viewerExe,0"
    $shortcut.Save()
    Start-Process -FilePath explorer.exe -ArgumentList @($shortcutPath) | Out-Null
    [pscustomobject]@{Detached=$true;Shortcut=$shortcutPath;Slices=$Slices}
    return
}

# Create the GUI on the real interactive desktop. A normal Start-Process here
# inherits CodexSandboxDesktop-* and produces a valid HWND that the user cannot
# see on their actual screens/taskbar.
Add-Type -TypeDefinition @'
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class HgInteractiveDesktopLauncher {
    [StructLayout(LayoutKind.Sequential, CharSet=CharSet.Unicode)]
    public struct STARTUPINFO {
        public int cb;
        public string lpReserved;
        public string lpDesktop;
        public string lpTitle;
        public int dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars;
        public int dwFillAttribute, dwFlags;
        public short wShowWindow, cbReserved2;
        public IntPtr lpReserved2, hStdInput, hStdOutput, hStdError;
    }
    [StructLayout(LayoutKind.Sequential)]
    public struct PROCESS_INFORMATION {
        public IntPtr hProcess, hThread;
        public int dwProcessId, dwThreadId;
    }
    [DllImport("kernel32.dll", SetLastError=true, CharSet=CharSet.Unicode)]
    public static extern bool CreateProcessW(string applicationName, StringBuilder commandLine,
        IntPtr processAttributes, IntPtr threadAttributes, bool inheritHandles, uint creationFlags,
        IntPtr environment, string currentDirectory, ref STARTUPINFO startupInfo,
        out PROCESS_INFORMATION processInformation);
    [DllImport("kernel32.dll")]
    public static extern bool CloseHandle(IntPtr handle);
}
'@ -ErrorAction Stop

$viewerCommand = New-Object System.Text.StringBuilder ('"' + $viewerExe + '" --watch-display "' + $preview + '"')
$startup = New-Object HgInteractiveDesktopLauncher+STARTUPINFO
$startup.cb = [Runtime.InteropServices.Marshal]::SizeOf($startup)
$startup.lpDesktop = 'WinSta0\Default'
$viewerInfo = New-Object HgInteractiveDesktopLauncher+PROCESS_INFORMATION
if (-not [HgInteractiveDesktopLauncher]::CreateProcessW(
    $viewerExe, $viewerCommand, [IntPtr]::Zero, [IntPtr]::Zero, $false, 0,
    [IntPtr]::Zero, $root, [ref]$startup, [ref]$viewerInfo)) {
    throw "Cannot launch Haunting Ground viewer on WinSta0\Default (Win32 $([Runtime.InteropServices.Marshal]::GetLastWin32Error()))"
}
[HgInteractiveDesktopLauncher]::CloseHandle($viewerInfo.hThread) | Out-Null
[HgInteractiveDesktopLauncher]::CloseHandle($viewerInfo.hProcess) | Out-Null
$viewer = Get-Process -Id $viewerInfo.dwProcessId -ErrorAction Stop

$arguments = @(
    '--realtime',
    '--clock-profile', 'issue-slots',
    '--slices', $Slices,
    '--toc-record', $toc,
    '--preview-file', $preview,
    '--spu2-output-core', '0',
    '--input-at', '5630000', 'left',
    '--input-at', '5700000', 'none',
    '--input-at', '5760000', 'cross',
    '--input-at', '5870000', 'none'
)
$game = Start-Process -FilePath $gameExe -ArgumentList $arguments -WorkingDirectory $root `
    -RedirectStandardOutput $stdout -RedirectStandardError $stderr -WindowStyle Hidden -PassThru

[pscustomobject]@{
    ViewerPID = $viewer.Id
    GamePID = $game.Id
    Preview = $preview
    Stdout = $stdout
    Stderr = $stderr
}
