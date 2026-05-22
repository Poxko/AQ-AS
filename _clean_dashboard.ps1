$path = Join-Path $PSScriptRoot 'dashboard.html'
if (-not (Test-Path $path)) { exit 0 }
$c = Get-Content $path -Raw
$c = $c -replace '(?m)^\s*<style>[\s\S]*?</style>\s*', "<link rel=`"stylesheet`" href=`"dashboard.css`">`n"
$c = $c -replace '(?m)^\s*/\* ={3,}[^*]*\*{3,}/\s*\r?\n', ''
$c = $c -replace '(?m)\s*default:\s*\r?\n\s*break;\s*\r?\n', ''
$c = $c -replace 'document\.getElementById\("wrapCostoFinale"\)[^\n]+\n', ''
$c = $c -replace 'id="wrapCostoFinale"[^>]*>', ''
$c = $c -replace 'scaricaFile\([^\)]+\);\s*\r?\n\s*window\.open\([^\)]+\);\s*\r?\n', ''
Set-Content $path $c -Encoding UTF8
