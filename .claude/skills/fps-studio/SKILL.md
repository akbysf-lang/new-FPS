---
name: fps-studio
description: Sanal FPS oyun stüdyosunu (1 GM, 2 developer, 2 artist, 1 designer) yönetme kılavuzu — kim hangi işi yapar, görev nasıl devredilir, aşama kapıları, teslim ve inceleme düzeni. Kullanıcı stüdyoya/ekibe iş verdiğinde, bir FPS görevi birden fazla disiplini ilgilendirdiğinde veya "ekibe yaptır", "GM'e sor", "stüdyo durumu" dendiğinde kullan. Triggers - studio, ekip, team, GM, sprint, görev dağıt, milestone.
---

# Sanal FPS Stüdyosu — İşletim Kılavuzu

## Ekip

| Agent (`subagent_type`) | Rol | Uzmanlık skill'leri |
|---|---|---|
| `studio-gm` | Genel Müdür / Yapımcı: planlar, böler, takip eder, kaliteyi denetler | fps-studio, fps-game-design, fps-market-research |
| `gameplay-developer` | Silah, hareket, kamera, AI, oynanış sistemleri | fps-gunplay-systems, fps-game-design, unreal-* |
| `systems-developer` | Ağ/çok oyunculu, performans, araçlar, build, altyapı | fps-gunplay-systems, unreal-* |
| `weapon-character-artist` | FP silah, kol, karakter, düşman, animasyon, VFX | fps-art-pipeline |
| `environment-artist` | Ortam, modüler kit, ışık, materyal, UI | fps-art-pipeline, fps-level-design |
| `game-designer` | Pazar araştırması, konsept, GDD, hikâye, seviye ve denge tasarımı | fps-game-design, fps-level-design, fps-market-research, fps-narrative |

Agent tanımları `.claude/agents/` altındadır. Agent türleri oturum başında yüklenir; aynı oturumda yeni eklenen bir tür görünmüyorsa `general-purpose` agent'a "önce `.claude/agents/<ad>.md` dosyasını ve orada sayılan skill dosyalarını oku, o rol olarak çalış" talimatıyla devret.

## Aşama kapıları

Bir aşamanın çıktısı onaylanmadan sonraki başlamaz:

1. **Keşif** — pazar araştırması, aday konseptler → *kullanıcı konsept seçer*
2. **Konsept** — tek sayfa konsept, hikâye, sanat yönü, teknik yaklaşım
3. **Prototip** — 3C + tek silah + tek düşman, gri kutu. Soru: "30 saniyelik döngü eğlenceli mi?"
4. **Dikey dilim** — nihai kalitede 10–15 dakika; Steam sayfası ve demo için temel
5. **Üretim** — içerik çoğaltma; kapsam kilitli
6. **Cila ve çıkış** — performans, hata, erişilebilirlik, mağaza sayfası, demo/Next Fest

## Görev devri

Her görev tek sahiplidir. Devir mesajı şunları içerir:

- **Amaç** ve bunun hangi aşamaya hizmet ettiği
- **Girdi:** okunacak dosyalar (yol vererek), bağlı kararlar
- **Çıktı:** tam dosya yolu ve biçim
- **Kabul kriterleri**
- **Kapsam dışı** olanlar

Bağımsız görevler paralel verilir (ör. iki artist aynı anda). Bağımlı işler sırayla: designer spesifikasyonu → developer + artist uygulaması → GM incelemesi.

Tipik bağımlılıklar: silah = designer (kart) → weapon-character-artist (model/anim) + gameplay-developer (sistem) · seviye = designer (blockout) → environment-artist (art pass) → systems-developer (performans).

## Dosya düzeni

```
docs/studio/
  README.md            ekip tüzüğü ve güncel durum
  design/              pazar araştırması, konsept, GDD, hikâye
  tech/                mimari kararlar, teknik notlar
  art/                 stil rehberi, varlık listeleri
```

Kararlar ilgili belgeye yazılır; sohbet içinde kalan karar kaybolur.

## GM denetimi

Teslim alınan her iş için: kabul kriterleri karşılandı mı · iddialar kaynaklı/doğrulanmış mı · diğer disiplinlerle çelişki var mı · kapsam şişti mi. Eksikse aynı agent'a somut düzeltme listesiyle geri gönderilir. Kullanıcıya rapor: ne bitti, ne bitmedi, hangi karar kullanıcıyı bekliyor.

## Araçlar

- **Blender MCP** (`mcp__blender__*`) ve **Unreal MCP** (`mcp__unreal__*`) yalnızca kullanıcının makinesinde Blender/Unreal açıkken çalışır. Bağlantı yoksa agent işi belge/kod/betik olarak hazırlar ve engeli açıkça bildirir; bağlantı varmış gibi davranmaz.
- Masaüstü uygulamalarında elle işlem gerekiyorsa computer-use kullanılabilir (kullanıcı izin verdi); önce `computer-use` skill'i okunur.
