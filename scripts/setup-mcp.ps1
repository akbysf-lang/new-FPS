# Windows icin Blender + Unreal MCP kurulum betigi.
# Kullanim: PowerShell'de depo kokunde  ->  .\scripts\setup-mcp.ps1
# Ne yapar:
#   1. uv kurulu degilse resmi kurucu ile kurar (Blender MCP sunucusu icin gerekli).
#   2. Blender eklentisini (blender_mcp.py) indirip Blender'a kurar.
#   3. UE 5.5-5.7 icin: LEGACY_UNREAL=1 verilirse unreal-mcp deposunu klonlar ve .mcp.json'i ona gore ayarlar.
#   4. Claude Code'un MCP sunucularini gordugunu dogrular.

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot

Write-Host "== 1/4 uv kontrolu ==" -ForegroundColor Cyan
if (-not (Get-Command uv -ErrorAction SilentlyContinue)) {
    Write-Host "uv bulunamadi, kuruluyor..."
    powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"
    $env:Path = "$env:USERPROFILE\.local\bin;$env:Path"
}
uv --version

Write-Host "== 2/4 Blender eklentisi ==" -ForegroundColor Cyan
# uvx blender-mcp install-addon  -> eklentiyi Blender'in addons klasorune kopyalar.
uvx blender-mcp install-addon
Write-Host "Blender'i ac -> Edit > Preferences > Add-ons -> 'MCP for Blender' isaretle."
Write-Host "Sonra 3D View'da N tusu -> 'MCP for Blender' sekmesi -> 'Connect to Claude'."

Write-Host "== 3/4 Unreal ==" -ForegroundColor Cyan
if ($env:LEGACY_UNREAL -eq "1") {
    $UnrealMcpDir = Join-Path $RepoRoot "tools\unreal-mcp"
    if (-not (Test-Path $UnrealMcpDir)) {
        git clone --depth 1 https://github.com/chongdashu/unreal-mcp $UnrealMcpDir
    }
    $cfg = Get-Content (Join-Path $RepoRoot ".mcp.unreal-legacy.json") -Raw
    $cfg = $cfg -replace '\$\{UNREAL_MCP_DIR\}', ($UnrealMcpDir -replace '\\', '/')
    $cfg | Set-Content (Join-Path $RepoRoot ".mcp.json") -Encoding UTF8
    Write-Host "UnrealMCP eklentisini projeye kopyala:"
    Write-Host "  $UnrealMcpDir\MCPGameProject\Plugins\UnrealMCP  ->  <UEProjen>\Plugins\UnrealMCP"
    Write-Host "Sonra UE'de Edit > Plugins > UnrealMCP isaretle ve editoru yeniden baslat."
} else {
    Write-Host "UE 5.8+ yerlesik sunucu kullanilacak (http://127.0.0.1:8000/mcp)."
    Write-Host "UE'de: Edit > Plugins > 'Model Context Protocol' isaretle -> yeniden baslat."
    Write-Host "Editor Preferences > General > Model Context Protocol > 'Auto Start Server' ac."
    Write-Host "UE 5.5-5.7 kullaniyorsan bu betigi  `$env:LEGACY_UNREAL=1  ile tekrar calistir."
}

Write-Host "== 4/4 Claude Code dogrulama ==" -ForegroundColor Cyan
if (Get-Command claude -ErrorAction SilentlyContinue) {
    Set-Location $RepoRoot
    claude mcp list
} else {
    Write-Host "claude komutu PATH'te yok. Kurulum: npm install -g @anthropic-ai/claude-code"
}
Write-Host "Bitti. Bu klasorde 'claude' calistir, ilk acilista .mcp.json sunucularini onayla, sonra /mcp ile durumu gor."
