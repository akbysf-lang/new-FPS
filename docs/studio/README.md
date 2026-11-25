# Sanal FPS Stüdyosu

Claude Code agent'larından oluşan 6 kişilik oyun stüdyosu. Hedef: Batılı (Kuzey Amerika/Avrupa) PC oyuncusuna yönelik, UE5 + Blender ile geliştirilen ticari bir FPS.

## Ekip

| Agent | Rol | Tanım dosyası |
|---|---|---|
| `studio-gm` | Genel Müdür / yapımcı: plan, görev dağıtımı, takip, denetim | `.claude/agents/studio-gm.md` |
| `gameplay-developer` | Silah, hareket, kamera, AI, oynanış sistemleri | `.claude/agents/gameplay-developer.md` |
| `systems-developer` | Ağ, performans, build, araçlar, Steam entegrasyonu | `.claude/agents/systems-developer.md` |
| `weapon-character-artist` | FP silah, karakter, düşman, animasyon, VFX | `.claude/agents/weapon-character-artist.md` |
| `environment-artist` | Sanat yönü, ortam, ışık, materyal, UI | `.claude/agents/environment-artist.md` |
| `game-designer` | Pazar araştırması, konsept, hikâye, seviye, denge | `.claude/agents/game-designer.md` |

## Eğitim (skill'ler)

Ekibin FPS bilgisi `.claude/skills/` altındaki skill'lerdedir. Her agent kendi uzmanlık skill'lerini başlangıçta yüklü olarak alır; ana Claude oturumu da hepsini kullanabilir.

| Skill | İçerik | Kullanan |
|---|---|---|
| `fps-studio` | Stüdyo işletimi: roller, aşama kapıları, görev devri, denetim | GM, ana oturum |
| `fps-game-design` | Core loop, 3C, gunplay hissi, TTK, denge, tempo, monetizasyon | Designer, GM, Gameplay Dev |
| `fps-gunplay-systems` | UE5 silah mimarisi, GAS, hareket, ağ, performans | Gameplay Dev, Systems Dev |
| `fps-level-design` | Ölçüler, blockout, siper/görüş hattı, akış, PvP harita | Designer, Environment Artist |
| `fps-art-pipeline` | Sanat yönü, Blender→UE5, FP silah, ortam, VFX, bütçeler | İki artist |
| `fps-market-research` | Pazar/rakip analizi yöntemi, gelir tahmini, Batılı kitle | Designer, GM |
| `fps-narrative` | FPS hikâyesi, dünya, kampanya yapısı, bark, Batı tonu | Designer |

Ek olarak hazır `unreal-cpp-gameplay`, `unreal-blueprints` ve `unreal-behavior-trees` skill'leri developer'lar tarafından kullanılır.

## Nasıl kullanılır

Claude Code'da doğal dille iş verin; örnekler:

- "GM, dikey dilim için 4 haftalık plan çıkar ve ekibe dağıt."
- "game-designer ilk üç silahın kartlarını yazsın."
- "weapon-character-artist ilk silahı Blender'da modellesin." (Blender MCP bağlı olmalı)
- "gameplay-developer hitscan silah bileşenini yazsın."

Agent tanımları oturum açılışında yüklenir; yeni eklenen agent'lar bir sonraki oturumda `subagent_type` olarak görünür.

## Güncel durum

| Aşama | Durum |
|---|---|
| Ekip kurulumu ve eğitim | Tamamlandı |
| 1. Keşif: pazar araştırması | [design/market-research.md](design/market-research.md) |
| 1. Keşif: hikâye taslağı | [design/game-story.md](design/game-story.md) |
| 2. Konsept | Designer önerisi: **EBBSHIFT** (co-op PvE, gelgit saati). Varsayılan olarak ilerleniyor; kullanıcı değiştirebilir |
| 3. Prototip | Kod yazıldı, **derlenmedi**: çekirdek, gelgit, iki silah, kanca, ganimet taşıma, iki düşman ([kurulum](tech/prototype-setup.md), [spesifikasyon](design/prototype-spec.md), [ADR](tech/adr-001-tide-and-network.md)). Sanat: [stil rehberi](art/style-guide.md), [varlık spesifikasyonu](art/weapon-character-specs.md). Sıradaki: kullanıcının makinesinde ilk derleme; Blender/Unreal bağlantısı |

## Klasörler

- `design/` — pazar araştırması, konsept, GDD, hikâye
- `tech/` — mimari kararlar ve teknik notlar
- `art/` — stil rehberi ve varlık listeleri
