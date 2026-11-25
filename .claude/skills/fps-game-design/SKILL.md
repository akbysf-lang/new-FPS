---
name: fps-game-design
description: FPS oyun tasarımı el kitabı — core loop, gunplay hissi (3C, TTK, recoil, feedback), silah dengesi, düşman/encounter tasarımı, tempo, ilerleme ve etik monetizasyon. FPS mekaniği tasarlarken, GDD yazarken, silah/düşman dengelerken veya "bu oyun neden iyi hissettirmiyor" sorusunu çözerken kullan. Triggers - FPS design, gunplay, TTK, weapon balance, core loop, game feel, GDD, encounter design.
---

# FPS Oyun Tasarımı

Buradaki sayılar başlangıç değeridir; her biri playtest ile doğrulanır. Sayıyı değil, arkasındaki nedeni savun.

## 1. Önce üç soru

Her özellik bu üçünden birine hizmet etmiyorsa kapsam dışıdır:

1. **Fantezi** — oyuncu kendini ne olarak hissediyor? (durdurulamaz güç / kırılgan hayatta kalan / taktik uzman)
2. **Core loop** — 30 saniyede tekrar eden döngü ne? (gör → karar ver → ateş et → ödül al → yeniden konumlan)
3. **Kanca** — Steam sayfasındaki tek cümle ve ilk 10 saniyelik GIF ne gösteriyor?

Döngüleri üç ölçekte yaz: **30 saniye** (çatışma), **5 dakika** (encounter/oda), **1 saat** (görev/run + kalıcı ilerleme).

## 2. 3C: Character, Camera, Controls

İlk prototip sprint'inin tamamı buraya gider; 3C kötüyse içerik kurtarmaz.

- **Hareket:** hızlanma/yavaşlama eğrileri anlık hissettirmeli (yere basış ≤ 0.1 sn'de tam hıza yakın). Coyote time (~0.1 sn) ve jump buffer ekle.
- **Kamera:** FOV varsayılan 90–100 (PC), ayarlanabilir 70–120. Silah FOV'u dünya FOV'undan ayrı. Head-bob ve screen shake kapatılabilir olmalı.
- **Girdi:** fare ham girdi, ivme yok. Gamepad'de aim assist (yavaşlatma + hafif rotasyon), dead-zone ve tepki eğrisi ayarı zorunlu.
- **Mantling/slide/dash** gibi ek hareketler ancak silah döngüsünü zenginleştiriyorsa girer.

## 3. Gunplay hissi

Bir silahın "iyi hissetmesi" katmanlıdır; hepsi birlikte çalışır:

| Katman | İçerik |
|---|---|
| Girdi tepkisi | Tetik → ilk mermi gecikmesi 0; animasyon girdiyi asla bekletmez |
| Recoil | Öngörülebilir desen + küçük rastgelelik; kamera tepmesi ile silah modeli tepmesi ayrı |
| Yayılım | Hip-fire geniş, ADS dar; hareket/zıplama cezası tarza göre |
| Hit feedback | Hitmarker, ses (vuruş/kafa/öldürme farklı), düşman hit-react, kan/parçacık |
| Ses | Silah sesi oyun hissinin yarısıdır: katmanlı atış, kuyruk (iç/dış mekan), mekanik sesler |
| Animasyon | Kısa, okunaklı; reload iptal edilebilir noktalar; ekipman değişimi < 0.5 sn |
| Öldürme onayı | Ayrı ses + görsel; oyuncu sonucu görmeden bir sonraki hedefe dönebilmeli |

**TTK (time-to-kill) oyunun kimliğidir:**

- ~0.15–0.4 sn: taktik/hardcore (konum ve bilgi kazanır)
- ~0.4–0.9 sn: arcade-askeri (refleks + konum)
- ~1–3+ sn: arena/hero/movement shooter (takip ve hareket kazanır)

TTK'yı seç, sonra can, hasar, atış hızı ve harita ölçüsünü ona göre türet. Bir tabloda tut: `hasar × RPM × isabet varsayımı → teorik ve pratik TTK`.

## 4. Silah cephaneliği ve denge

- Her silahın **bir işi** olsun (menzil bandı × durum). İki silah aynı işi yapıyorsa biri gereksiz.
- Denge üçgeni: hasar / kullanım kolaylığı / esneklik. Üçünde de güçlü silah olmaz.
- Önce az silahla (3–5) derinlik, sonra genişlik. Her silah için tek sayfalık kart: rol, menzil, TTK, zayıflık, "bu silahı seçme nedeni".
- PvE'de güç fantezisi dengeden önce gelir; PvP'de algılanan adalet her şeyden önce gelir.

## 5. Düşman ve encounter (PvE)

- Düşman arketipleri oyuncuya **farklı soru** sorar: baskıcı (yaklaşır), nişancı (siper aldırır), alan kapatan (yer değiştirtir), tank (öncelik sıralatır), destek (önce beni öldür).
- Siluet ve ses ile 1 saniyede ayırt edilebilmeli.
- Saldırılar telegraf edilir: uyarı → pencere → sonuç. Haksız ölüm = okunamayan ölüm.
- Encounter = arena + düşman karışımı + dalga temposu. Önce 2 arketip kombinasyonu öğret, sonra karıştır.
- AI zeki görünmeli, zeki olmak zorunda değil: bark'lar ("flanking!"), kasıtlı ıskalar, siper değişimi.

## 6. Tempo

Gerilim eğrisi testere dişi olmalı: yüksel → zirve → nefes → daha yükseğe. Aralıksız çatışma yorar. Nefes anları hikâye, keşif ve yükseltme için kullanılır.

## 7. İlerleme ve tekrar oynanabilirlik

- Kısa kampanya tek başına zor satar; tekrar oynanabilirlik (roguelite run'lar, co-op, zorluk kademeleri, mod desteği) ömrü uzatır.
- Yatay ilerleme (yeni seçenek) > dikey ilerleme (düz sayı artışı).
- İlk 10 dakika: kontrolü ver, 60 saniye içinde ilk öldürme, 10 dakika içinde ilk "vay" anı.

## 8. Monetizasyon (Batı pazarı)

- Premium (tek seferlik satın alma) indie FPS için varsayılan modeldir. F2P, canlı operasyon ekibi ve büyük kullanıcı edinme bütçesi ister.
- Pay-to-win, loot box ve sahte aciliyet Batılı PC oyuncusunda itibar kaybıdır; bazı ülkelerde düzenlemeye tabidir.
- Kabul gören: kozmetik DLC, soundtrack, destekçi paketi, büyük içerik genişlemesi.

## 9. Kapsam disiplini

Küçük ekip için: tek güçlü kanca, dar ama cilalı içerik. Her özellik için "bunu çıkarırsak oyun hâlâ çalışır mı?" diye sor. Dikey dilim (vertical slice) = nihai kalitede 10–15 dakika.

## 10. Çıktı şablonları

- **Tek sayfa konsept:** logline, fantezi, core loop, kanca, hedef kitle, referanslar (X + Y), kapsam, riskler.
- **Özellik spesifikasyonu:** amaç, oyuncu deneyimi, kurallar, sayılar (ayarlanabilir), uç durumlar, kabul kriterleri, bağımlılıklar (dev/art).
- **Silah/düşman kartı:** rol, istatistikler, karşı oyun, üretim maliyeti.
