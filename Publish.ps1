param (
    [string]$BuildDir = "$PSScriptRoot/out/build/clang"
)

$cacheFile = Join-Path $BuildDir "CMakeCache.txt"
$cache = Get-Content $cacheFile

function Get-CMakeVar($name) {
    $line = $cache | Where-Object { $_ -match "^${name}:" }
    if ($line) {
        return ($line -split "=")[1]
    }
    return $null
}

$env:USE_ASSIMP = Get-CMakeVar "USE_ASSIMP"
$env:USE_BULLET = Get-CMakeVar "USE_BULLET"

Write-Host "USE_ASSIMP = $env:USE_ASSIMP"
Write-Host "USE_BULLET = $env:USE_BULLET"

New-Item -ItemType Directory -Force -Path "$BuildDir/publish/include","$BuildDir/publish/bin"

robocopy "$PSScriptRoot/src" "$BuildDir/publish/include" *.h *.hpp /S /XO | Out-Null

Copy-Item -Force "$BuildDir/Syng.dll" "$BuildDir/publish/bin/"

$header="$BuildDir/publish/include/Syngine.hpp"
$content=Get-Content $header
$out=@()

foreach($line in $content){
  $out+=$line
  if($line -match "#pragma once"){
    if($env:USE_ASSIMP -eq "ON"){ $out+="#define USE_ASSIMP" }
    if($env:USE_BULLET -eq "ON"){ $out+="#define USE_BULLET" }
  }
}

$out | Set-Content $header
