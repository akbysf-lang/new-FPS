---
name: fps-gunplay-systems
description: Unreal Engine 5'te FPS sistemlerinin teknik uygulaması — silah mimarisi (hitscan/projectile), recoil ve yayılım, hasar hattı, GAS, first-person render, karakter hareketi, çok oyunculu ağ (prediction, lag compensation, replication), performans bütçesi. FPS kodu yazarken, silah/hareket/ağ hatası ayıklarken veya teknik mimari kararı verirken kullan. Triggers - UE5 weapon system, hitscan, projectile, recoil code, lag compensation, replication, GAS, FPS netcode, frame budget.
---

# FPS Sistemleri — UE5 Teknik El Kitabı

UE5 temelleri için `unreal-cpp-gameplay`, `unreal-blueprints` ve `unreal-behavior-trees` skill'lerini de yükle. Motor sürümüne özgü API'lerde hafızana güvenme: projedeki motor kaynağını veya resmi dokümanı kontrol et.

## 1. Mimari ilkeler

- **Veri odaklı:** silah, düşman ve ayar değerleri `UPrimaryDataAsset` / DataTable içinde. Tasarımcı kod derlemeden ayar yapabilmeli.
- **C++ çekirdek, Blueprint içerik:** sistemler ve sıcak yol C++; silah varyantları, VFX bağlama, UI Blueprint.
- **Bileşen tabanlı:** `UWeaponComponent`, `UHealthComponent`, `UInventoryComponent` — dev karakter sınıfı yok.
- **Hard reference'tan kaçın:** `TSoftObjectPtr` + Asset Manager; aksi halde bir silah tüm oyunu belleğe çeker.
- Lyra örnek projesi iyi bir referanstır ama olduğu gibi kopyalamak küçük ekip için ağırdır; kalıplarını al.

## 2. Silah sistemi

**Ateş modeli seçimi:**

| | Hitscan | Projectile |
|---|---|---|
| Kullanım | tüfek, tabanca, SMG | roket, bomba, yavaş/gözle görülen mermi, uzun menzil balistik |
| Maliyet | tek line trace | aktör veya havuzlanmış simülasyon |
| Ağ | lag compensation ile kolay | prediction daha zor |

- Hitscan izi **kameradan** başlar (nişangâh neredeyse oraya), görsel tracer **namludan** çıkar ve vuruş noktasına yönelir. Yakın duvar durumunda namlu-engel kontrolü yap.
- Çok sayıda mermi için aktör spawn etme: havuzla veya veri dizisi üzerinde elle simüle et.
- Atış döngüsü: `CanFire → cephane düş → yayılım hesapla → iz/mermi → hasar → recoil → kozmetik`. Kozmetik (ses, muzzle flash, animasyon) yerel istemcide anında, sunucu onayını beklemeden oynar.
- Atış hızı zamanlayıcısı frame-rate'ten bağımsız olmalı: bir frame'de birden fazla atış birikebilir, kalan süreyi taşı.

**Recoil ve yayılım:**

- Recoil = desen (eğri/dizi, atış indeksine göre) + küçük rastgelelik + toparlanma. Kamera recoil'i kontrolcü rotasyonuna, görsel tepme silah mesh'ine ayrı uygulanır.
- Yayılım = taban + hareket + art arda atış ısısı; ADS çarpanı. Ağda deterministik olması için atış başına seed kullan.
- ADS: FOV geçişi, hassasiyet çarpanı (FOV oranına göre), silah mesh'i nişangâha hizalanır.

**Hasar hattı:** taban hasar → mesafe düşüşü (eğri) → bölge çarpanı (physical material: kafa/gövde/uzuv) → zırh/direnç → son hasar. Tek bir fonksiyondan geçsin, loglanabilir olsun.

## 3. Gameplay Ability System (GAS)

Yetenek, bekleme süresi, buff/debuff ve öznitelik çokluğu varsa GAS kullan; sadece "ateş et ve can azalt" varsa aşırıdır. Kullanıyorsan: öznitelikler `AttributeSet`'te, hasar `GameplayEffect` + execution calculation ile, durumlar `GameplayTag` ile, kozmetik `GameplayCue` ile.

## 4. First-person sunum

- Ayrı FP kol/silah mesh'i (yalnızca sahibi görür) + dünya için tam vücut mesh'i (sahibi görmez veya gölge için kullanır).
- Silahın duvara girmesi: UE 5.5+ first-person rendering özelliklerini kontrol et; eski yöntem ayrı FOV'lu materyal/panini veya ölçek hilesi.
- Silah sway, bob, iniş tepkisi prosedürel (yay-sönüm) katmanlar; animasyon üstüne eklenir.
- Animasyon: FP için ayrı AnimBP, katmanlı (locomotion → ADS → recoil additive).

## 5. Hareket

- `UCharacterMovementComponent` (CMC) ağ prediction'ı hazır verir. Özel hareket (slide, dash, wall-run) için custom movement mode + `FSavedMove` uzantısı; hareket durumunu RPC ile değil saved move bayraklarıyla taşı.
- Mover eklentisi daha yeni bir alternatiftir; üretimde kullanmadan önce hedef motor sürümündeki olgunluğunu doğrula.
- Tasarımcının ayarlayacağı her değer (hız, ivme, sürtünme, zıplama, hava kontrolü) veri varlığında.

## 6. Çok oyunculu

- **Sunucu otoriter.** İstemci girdiyi ve "şunu vurduğumu düşünüyorum" bilgisini gönderir; sunucu doğrular.
- **Client-side prediction:** hareket ve atış kozmetiği anında; sunucu düzeltmesi yumuşatılır.
- **Lag compensation (server rewind):** sunucu hitbox geçmişini (~son 200–300 ms) tutar, atış zaman damgasına geri sarıp doğrular. Üst sınır koy, yoksa yüksek ping avantaja döner.
- **Replication:** yalnızca gerekeni, gerekene (`COND_OwnerOnly`, `COND_SkipOwner`). Sık değişenler için `OnRep`; tek seferlik olaylar için unreliable multicast. Reliable RPC'yi atış gibi yüksek frekanslı işlerde kullanma.
- Yeni projede Iris replication sistemini değerlendir; ölçek küçükse klasik sistem yeterli.
- **Anti-cheat asgarisi:** sunucuda atış hızı, cephane, menzil ve görüş hattı doğrulaması. İstemciye asla güvenme.
- Co-op PvE, rekabetçi PvP'den çok daha ucuzdur: listen server, gevşek doğrulama, anti-cheat baskısı düşük.
- Ağ koşulu testi: `Net PktLag`, `Net PktLoss` ile her özelliği 100–150 ms ve %2–5 kayıpta dene.

## 7. AI

Behavior Tree veya State Tree + EQS (siper/konum seçimi) + AI Perception (görme/duyma). Düşman atışlarında kasıtlı ilk-ıskalama ve token sistemi (aynı anda en fazla N düşman ateş eder) hissi belirgin iyileştirir. Detay: `unreal-behavior-trees`.

## 8. Performans

- Hedef: PC'de 60 fps tabanı = **16.6 ms**; rekabetçi için 120+. Bütçeyi böl: game thread / render thread / GPU.
- Ölç, tahmin etme: `stat unit`, `stat game`, `stat gpu`, Unreal Insights.
- Tick'i kapat; zamanlayıcı ve olay kullan. Sıcak yolda Blueprint'ten kaçın.
- Lumen/Nanite/VSM ağırdır: hedef donanımı baştan belirle (Steam Deck ve orta sınıf GPU'lar Batı PC pazarının büyük kısmıdır) ve ölçeklenebilirlik ayarlarını test et.
- Shader derleme takılmaları için PSO önbelleği/precaching: FPS'te ilk atışta takılma kabul edilemez.
- Havuzlama: mermi, decal, VFX, ses bileşenleri.

## 9. Tamamlandı tanımı

Derleniyor · editörde ve paketlenmiş build'de çalışıyor · çok oyunculuysa 2 istemci + gecikme ile denendi · ayar değerleri veri varlığında · performans bütçesi içinde · tasarımcıya nasıl ayarlanacağı yazıldı.
