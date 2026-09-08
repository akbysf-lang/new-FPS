#!/usr/bin/env bash
# macOS / Linux icin Blender + Unreal MCP kurulum betigi.
# Kullanim: depo kokunde  ->  bash scripts/setup-mcp.sh
# UE 5.5-5.7 icin:        ->  LEGACY_UNREAL=1 bash scripts/setup-mcp.sh
set -euo pipefail
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "== 1/4 uv kontrolu =="
if ! command -v uv >/dev/null 2>&1; then
  echo "uv bulunamadi, kuruluyor..."
  curl -LsSf https://astral.sh/uv/install.sh | sh
  export PATH="$HOME/.local/bin:$PATH"
fi
uv --version

echo "== 2/4 Blender eklentisi =="
uvx blender-mcp install-addon
echo "Blender'i ac -> Edit > Preferences > Add-ons -> 'MCP for Blender' isaretle."
echo "Sonra 3D View'da N tusu -> 'MCP for Blender' sekmesi -> 'Connect to Claude'."

echo "== 3/4 Unreal =="
if [[ "${LEGACY_UNREAL:-0}" == "1" ]]; then
  UNREAL_MCP_DIR="$REPO_ROOT/tools/unreal-mcp"
  [[ -d "$UNREAL_MCP_DIR" ]] || git clone --depth 1 https://github.com/chongdashu/unreal-mcp "$UNREAL_MCP_DIR"
  sed "s#\${UNREAL_MCP_DIR}#$UNREAL_MCP_DIR#g" "$REPO_ROOT/.mcp.unreal-legacy.json" > "$REPO_ROOT/.mcp.json"
  echo "UnrealMCP eklentisini projeye kopyala:"
  echo "  $UNREAL_MCP_DIR/MCPGameProject/Plugins/UnrealMCP  ->  <UEProjen>/Plugins/UnrealMCP"
  echo "Sonra UE'de Edit > Plugins > UnrealMCP isaretle ve editoru yeniden baslat."
else
  echo "UE 5.8+ yerlesik sunucu kullanilacak (http://127.0.0.1:8000/mcp)."
  echo "UE'de: Edit > Plugins > 'Model Context Protocol' isaretle -> yeniden baslat."
  echo "Editor Preferences > General > Model Context Protocol > 'Auto Start Server' ac."
  echo "UE 5.5-5.7 kullaniyorsan: LEGACY_UNREAL=1 bash scripts/setup-mcp.sh"
fi

echo "== 4/4 Claude Code dogrulama =="
if command -v claude >/dev/null 2>&1; then
  (cd "$REPO_ROOT" && claude mcp list)
else
  echo "claude komutu PATH'te yok. Kurulum: npm install -g @anthropic-ai/claude-code"
fi
echo "Bitti. Bu klasorde 'claude' calistir, ilk acilista .mcp.json sunucularini onayla, sonra /mcp ile durumu gor."
