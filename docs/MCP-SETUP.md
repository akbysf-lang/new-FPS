# Blender + Unreal Engine MCP Bağlantısı

Bu depo, yerel makinede çalışan **Claude Code**'u aynı makinedeki **Blender** ve
**Unreal Engine** editörüne MCP (Model Context Protocol) üzerinden bağlar.
Depo kökündeki `.mcp.json` dosyası Claude Code tarafından otomatik okunur.

> Not: Bu bağlantı yalnızca Blender ve Unreal'ın kurulu olduğu makinede,
> o makinedeki Claude Code ile çalışır. Claude Code Web (bulut) oturumları
> yerel uygulamalara erişemez.

## Mimari

```
Claude Code ──stdio──▶ uvx blender-mcp ──TCP 9876──▶ Blender eklentisi (MCP for Blender)
Claude Code ──HTTP───▶ http://127.0.0.1:8000/mcp ──▶ Unreal Editor (Model Context Protocol eklentisi, UE 5.8+)
```

UE 5.5 – 5.7 için yerleşik eklenti yoktur; o durumda topluluk projesi
`chongdashu/unreal-mcp` kullanılır (aşağıda "Eski Unreal sürümleri").

## Hızlı kurulum

**Windows (PowerShell):**
```powershell
.\scripts\setup-mcp.ps1
```

**macOS / Linux:**
```bash
bash scripts/setup-mcp.sh
```

Betik `uv` kurar, Blender eklentisini yükler ve `claude mcp list` ile doğrular.

## Adım adım

### 1. Blender
1. Betiği çalıştırın veya elle: `uvx blender-mcp install-addon`
2. Blender → **Edit → Preferences → Add-ons** → "MCP for Blender" işaretleyin.
3. 3D View'da **N** tuşu → **MCP for Blender** sekmesi → **Connect to Claude**.
   Sunucu `127.0.0.1:9876` üzerinde dinlemeye başlar.

Gereksinimler: Blender 3.0+, Python 3.10+, `uv` (pip ile değil, resmi kurucu ile).

### 2. Unreal Engine 5.8+
1. Projenizi açın → **Edit → Plugins** → **Model Context Protocol** işaretleyin → editörü yeniden başlatın.
2. **Editor Preferences → General → Model Context Protocol → Auto Start Server** açın.
   (Ya da her açılışta konsoldan `StartServer` verin.)
3. Sunucu `http://127.0.0.1:8000/mcp` adresinde çalışır. `.mcp.json` zaten bu adrese ayarlı.

### 3. Claude Code
Depo kökünde:
```bash
claude
```
İlk açılışta Claude Code `.mcp.json` içindeki `blender` ve `unreal` sunucularını
onaylamanızı ister. Onaylayın, ardından `/mcp` yazarak bağlantı durumunu görün.

Test:
- "Blender'da sahnedeki nesneleri listele"
- "Unreal'da mevcut level'daki actor'ları listele"

## Eski Unreal sürümleri (5.5 – 5.7)

```powershell
$env:LEGACY_UNREAL=1; .\scripts\setup-mcp.ps1      # Windows
LEGACY_UNREAL=1 bash scripts/setup-mcp.sh          # macOS / Linux
```

Betik `tools/unreal-mcp` klasörüne depoyu klonlar ve `.mcp.json`'ı
`.mcp.unreal-legacy.json` şablonundan yeniden üretir. Sonra:

1. `tools/unreal-mcp/MCPGameProject/Plugins/UnrealMCP` klasörünü UE projenizin
   `Plugins/` klasörüne kopyalayın.
2. **Edit → Plugins → UnrealMCP** işaretleyin, editörü yeniden başlatın.
   Eklenti `55557` portunda dinler; Python sunucusu Claude Code tarafından otomatik başlatılır.

Gereksinim: Python 3.12+.

## Sorun giderme

| Belirti | Çözüm |
|---|---|
| `/mcp` içinde blender "failed" | Blender açık mı ve **Connect to Claude** basıldı mı? `uvx --version` çalışıyor mu? |
| `/mcp` içinde unreal "failed" | UE editörü açık mı, eklenti etkin mi, `curl http://127.0.0.1:8000/mcp` yanıt veriyor mu? |
| Port çakışması | Blender: `BLENDER_PORT` env'ini `.mcp.json`'da değiştirin. UE: Editor Preferences'ta portu değiştirip `url`'i güncelleyin. |
| `uvx` bulunamadı | Terminali yeniden açın; `~/.local/bin` (Linux/macOS) veya `%USERPROFILE%\.local\bin` (Windows) PATH'te olmalı. |

## Kaynaklar
- Blender MCP: https://github.com/ahujasid/blender-mcp
- Unreal MCP (topluluk, UE 5.5–5.7): https://github.com/chongdashu/unreal-mcp
- Epic yerleşik Unreal MCP (UE 5.8): https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor
