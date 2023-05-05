Param(
[string]$VsPath="C:\Program Files\Microsoft Visual Studio\2022\Community\"
)
# Assumes Visual Studio 2022
$VcVarsPath = Join-Path $VsPath "VC\Auxiliary\Build\vcvars64.bat"
$MakeCommand = "nmake /F ..\wincon\Makefile.vc UTF8=Y WIDE=Y pdcurses.lib"
$OutDir = Join-Path $PSScriptRoot "out"
if (Test-Path $VcVarsPath -PathType Leaf) {
    if (!(Test-Path $OutDir)) {
        mkdir $OutDir
    }
    &cmd "/c" "`"$VcVarsPath`" && cd `"$OutDir`" && $MakeCommand"
} else {
    Write-Host "Failed to find Visual Studio! Try changing the default path using the -VsPath parameter."
}