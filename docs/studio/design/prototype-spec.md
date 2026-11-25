# EBBSHIFT — Prototip Spesifikasyonu (Aşama 3)

**Hazırlayan:** game-designer · **Tarih:** 25 Kasım 2026 · **Durum:** v1, Revizyon 2 (artist incelemesi sonrası düzeltmeler; değişiklik dökümü §12) — uygulamaya hazır
**Bağlı belgeler:** [game-story.md](game-story.md) (§3 dünya kuralları, §9 teslim listeleri) · [market-research.md](market-research.md) (§6 geri çekilme kapısı, §7 konsept)

> Belge dili Türkçe; oyun içi adlar ve satırlar İngilizce.
> **Bu belgedeki her sayı başlangıç değeridir.** Hiçbiri playtest görmedi; hepsi veri varlığında / ayar dosyasında açık olmalı, koda gömülmemeli. Sayıyı değil, yanındaki gerekçeyi savunuyoruz.
> Birimler: uzunluk cm (UE birimi), hız cm/sn, açı derece, süre saniye. Yükseklikler dünya Z'sidir; **Z = 0 kumsalın deniz kenarıdır.**

---

## 1. Kapsam, soru ve ölçütler

### 1.1 Prototipin yanıtlayacağı tek soru

> **Su arkandan yükselirken yokuş yukarı savaşmak, 4 kişiyle ve tek başına, 30 saniyelik döngüde eğlenceli mi?**

30 saniyelik döngü (test edilen şey): **gör → ıslak/kuru konum seç → ateş et → sandığı al/bırak → bir üst sipere çık.** Prototipteki her öğe bu döngüye hizmet eder; etmeyen kapsam dışıdır.

### 1.2 Kapsam

| Kapsamda | Not |
|---|---|
| Tek harita: **Promenade** gri kutusu, üç kat (Low / Half / High) | §7. Yalnız ilkel geometri + ızgara materyali + su çizgisi bantları |
| Tek küresel su düzlemi, 12 dakikalık tek gelgit döngüsü | §3. Çizelge veriyle tanımlı (parça parça doğrusal eğri) |
| Islaklık durumu (oyuncu + AI ortak) | §3.3 |
| 3C: yürüme, koşu, zıplama, çömelme, ADS | §2 |
| Silahlar: **Purser**, **Gaffer**, **Kedge** | §4 |
| Düşmanlar: **Scuttler**, **Attendant** | §5 |
| Sandık (**Strongbox**) taşıma, 3 ganimet noktası, 1 vinç noktası | §6.4, §7 |
| Can, düşme/kaldırma, solo **spare line** | §6 |
| 1–4 oyuncu, listen server | Pazar raporu R1: ilk iş 4 oyunculu test |
| Asgari HUD: can, mermi, gelgit saati (evre + dönüşe kalan süre), sandık sayacı | Metin/çubuk yeter |
| Geçici ses: çan, tannoy cızırtısı, su uğultusu, iki düşmanın telgraf sesleri, hitmarker/öldürme onayı | Telgraf sesleri oynanıştır; sessiz prototip yanlış sonuç verir |
| Telemetri günlüğü (CSV) | §1.5 |
| Hata ayıklama başlangıcı: **Flood Drill** | §8.3 |

| Kapsam dışı (bilinçli) | Neden |
|---|---|
| Hub, ekonomi (Scrip), yükseltme, kalıcı ilerleme | Soru 30 sn döngüsü hakkında, 1 saatlik döngü hakkında değil |
| Diğer silahlar, aletler (Winch Line, Sluice Jack …), savaklar | Savak ikinci sorudur ("suyu geciktirmek eğlenceli mi?"); önce su tek başına sınanır |
| Diğer 8 arketip, boss | İki arketip suya **ters** tepki veriyor; çekirdek kural bununla sınanır |
| Elektrik-su zinciri, ateş | Storm Lamp/Mayday ile gelir |
| Yüzme, mantling, slide, dash | §2.4 |
| Seslendirme, hikâye vuruşu, karakter seçimi | Aggie/tannoy satırları ekranda altyazı + geçici ses olarak yeter |
| Sanat (model, doku, ışık cilası), su shader cilası | Blockout oturmadan sanat yapılmaz |
| Prosedürel yerleşim, çizelge varyantları, zorluk kademeleri | Tek sabit çizelge |
| Eşleşme, Steam oturumu arayüzü | Doğrudan IP/davet ile bağlanmak yeter |

### 1.3 Playtest düzeni

- En az **8 oturum**: 4 solo, 4 co-op (en az 2'si tam 4 kişi). Her oyuncu en az **3 döngü** oynar (ilk döngü öğrenmedir; ölçüm 2. ve 3. döngüden alınır).
- Mümkünse en az 4 oyuncu ekip dışından; gözlemci yönlendirme yapmaz, yalnız not alır.
- Her döngü sonunda 4 soru (1–5): *eğlence*, *ölümler adil miydi*, *suyu ne kadar düşündün*, *bir daha oynar mısın*.

### 1.4 Devam / geri çekilme ölçütleri

**Devam (Aday A, co-op) — aşağıdakilerin en az 5'i tutmalı, 1 ve 6 zorunlu:**

| # | Gözlem | Eşik (başlangıç değeri) |
|---|---|---|
| 1 | Co-op'ta flood evresi eğlence puanı (2.–3. döngü ortalaması) | ≥ 4,0 / 5 |
| 2 | Kendiliğinden "bir tur daha" (kimse istemeden yeni döngü başlatma) | Oturumların ≥ %70'i |
| 3 | **Açgözlülük:** 4:00 uyarısından sonra en az bir oyuncu hâlâ Low'da ya da geri iniyor | Co-op döngülerinin ≥ %50'si |
| 4 | **Su farkındalığı:** oyuncu suyu kasten kullanıyor (Attendant'ı suya düşürme/kısa devre, Scuttler'ı kuruya çekme, ıslak rotadan kaçınma) | 3. döngüde oyuncuların ≥ %60'ında en az 1 kez |
| 5 | Okunabilir ölüm: düşen oyuncu nedenini kendi söyleyebiliyor | Düşmelerin ≥ %80'i |
| 6 | 4 oyunculu listen server'da su yüksekliği ve ıslaklık durumu istemciler arasında tutarlı | Sapma ≤ 5 cm; 12 dakikalık döngü kopmadan bitiyor |
| 7 | Solo eğlence puanı ve yeniden oynama | ≥ 3,5 / 5 ve soloların ≥ %50'si 2. döngüyü kendisi başlatıyor |
| 8 | Kota başarı oranı (3. döngü) | %40–70 (altı = cezalandırıcı, üstü = su tehdit değil) |

**Solo roguelite'a (Aday C) geri çekilme — aşağıdakilerden biri yeter:**

| Tetik | Anlamı |
|---|---|
| Ölçüt 6, prototipin 6. haftası sonunda hâlâ tutmuyor | Pazar raporu §6'daki kapı: ağ yükü 2 developer'ı aşıyor. Tasarım aynen korunur, co-op EA sonrasına kalır |
| Solo puanı ≥ 3,5 **ve** co-op puanı < 3,5 (iki ayar turundan sonra) | Çekirdek tek kişiyle çalışıyor; kalabalık okunabilirliği bozuyor |
| Co-op'ta oyuncular birbirine ihtiyaç duymuyor: taşıyıcıya eşlik eden yok, herkes ayrı koşuyor (≥ %60 döngü) | Co-op'un katkısı yok; maliyetini haklı çıkarmıyor |

**Konsepte geri dönüş (ne A ne C) — ikisi birlikte görülürse GM'e konsept incelemesi önerilir:** iki ayar turundan sonra hem solo hem co-op flood puanı < 3,5 **ve** oyuncuların çoğu suyu "geri sayım sayacı" diye tarif ediyor (Ölçüt 4 < %30). Bu durumda sorun sayı değil kuraldır.

### 1.5 Telemetri (developer; CSV, döngü başına)

Teslim edilen sandık sayısı ve zamanı · her düşme: zaman, konum, neden (Scuttler / Attendant / boğulma), o andaki ıslaklık durumu · oyuncu başına ıslaklık durumlarında geçen süre · çan anında (5:00) her oyuncunun katı · silah × düşman × düşman ıslak/kuru öldürme sayıları · Attendant kısa devre sayısı · her sandığın alındığı ve bırakıldığı zaman/konum.

---

## 2. 3C değerleri

Fantezi "işini bilen, soğukkanlı işçi": hareket **ağır değil ama akrobatik de değil**. Hız, suyun aldığı cezanın hissedilmesi için yeterince yüksek olmalı (yavaş karakteri yavaşlatmak fark edilmez).

### 2.1 Karakter ve kamera

| Değer | Başlangıç | Gerekçe |
|---|---|---|
| Kapsül yarıçap / yarı yükseklik | 34 / 90 (boy 180) | fps-level-design metrikleri |
| Göz yüksekliği (ayakta / çömelmiş) | 164 / 104 (zeminden). Kapsül merkezine göre: `BaseEyeHeight` **74**, `CrouchedEyeHeight` **49** | 110'luk alçak siper çömelmiş gözü tam kapatır, ayakta üstünden ateş edilir |
| Çömelmiş kapsül yarı yüksekliği | **55 cm** (çömelmiş boy 110; yarıçap 34 aynı) | Çömelmiş kapsülün üstü alçak siperle (110) **aynı kotta**: siperin arkasında çömelen oyuncuya aynı kottan gelen mermi değemez. Eski değer 60 (boy 120) siperin 10 cm üstünde kalıyordu. Siper 110 kalır; değişen çömelmedir. Haritada 230'dan alçak tavan yok, göz ile kapsül üstü arasındaki 6 cm sorun çıkarmaz |
| Dünya FOV | 95 (ayar 70–120) | PC varsayılanı; dikey okuma (yukarıdaki düşman, aşağıdaki su) geniş FOV ister |
| Silah (viewmodel) FOV | 70, dünya FOV'undan bağımsız | |
| Head-bob / screen shake | Düşük; ikisi de kapatılabilir | |
| Fare | Ham girdi, ivme yok | |
| Gamepad | Prototipte yalnız temel bağlama; aim assist dikey dilimde | Kapsam |

### 2.2 Hareket

| Değer | Başlangıç | Gerekçe |
|---|---|---|
| Yürüme (varsayılan) `MaxWalkSpeed` | **500** | Siperden sipere 10–15 m = 2–3 sn |
| Koşu (sprint) | **650** (×1,3); yalnız ileri ±45°; ateş edilemez | Kaçış aracı, savaş aracı değil. Islak Scuttler'dan (504) ancak koşarak kaçılır |
| Koşudan ateşe geçiş | 0,20 | |
| ADS'de hareket | 300 (×0,6) | |
| Çömelme hızı | 250 | |
| `MaxAcceleration` | 5000 (0→500: 0,10 sn) | Anlık tepki (skill: ≤ 0,1 sn) |
| `BrakingDecelerationWalking` / `GroundFriction` | 4000 / 8 | Kaymadan durma; siper kenarında hassasiyet |
| `JumpZVelocity` / `GravityScale` | 450 / 1,2 | Zıplama yüksekliği **86 cm**, havada 0,77 sn. 110'luk alçak siperin üstünden atlanamaz: siper bilinçli karar olur |
| Yatay zıplama mesafesi (yürüme / koşu) | ~380 / ~500 | Gym'de doğrulanacak; haritada zorunlu boşluk atlaması yok |
| `AirControl` | 0,35 | |
| Coyote time / jump buffer | 0,10 / 0,10 | |
| `MaxStepHeight` | 45 | |
| Yürünebilir eğim | ≤ 40°; haritadaki en dik rampa 26,6° | |
| Silah değiştirme | 0,40 | Skill: < 0,5 sn. Yalnız Purser ↔ Gaffer için geçerlidir; **Kedge silah yuvası değildir**, ayrı tuşla silah değiştirmeden savrulur (§4.3) |

### 2.3 Sandık taşırken

| Değer | Başlangıç | Gerekçe |
|---|---|---|
| Yürüme hızı | **400** (×0,8) | Kuru Scuttler'dan (360) kıl payı hızlı, ıslak Scuttler'dan (504) yavaş: taşıyıcı **suyun önünde** kalmak zorunda |
| Koşu | Yok | Taşıyıcı eskorta muhtaç olur (co-op nedeni) |
| Zıplama | `JumpZVelocity` 360 → 55 cm | Basamak ve küçük engel geçilir |
| Çömelme | Var, 200 | |
| Silah | Yalnız **Purser**, yalnız hip-fire; ADS yok; **şarjör değiştirilemez** (tek el) | Purser'ın "işi" bu (hikâye §9.6). 6 mermi biter → sandığı bırak ya da arkadaşına güven |
| Kedge / Gaffer | Kullanılamaz | Sol kol sandıkta |
| Kadraj | Strongbox **görünür**: sol kolun altında, ekranın sol alt köşesinde (ekran alanının en çok ~%15'i; nişangâha ve ekran merkezine taşmaz). Purser sağ elde, bel hizasında eğik | Oyuncu ve klibi izleyen "yük taşıyor"u HUD'sız okur; tek el / reload yok / Kedge yok kuralının görsel nedeni budur. Prototipte kameraya bağlı vekil kutu yeter |
| Bırakma | Anında (tuş ya da silah değiştirme); sandık olduğu yere düşer, fırlatılmaz | |
| Alma | 0,5 sn basılı tut | |
| Hasar alınca | Düşmez; yalnız oyuncu düşerse (0 can) bırakılır | Her vuruşta düşen sandık sinir bozar |

### 2.4 Bilinçli olarak yok

Mantling, slide, dash, yüzme. Gerekçe: soru "yokuş yukarı **savaşmak**"; dikey kısayollar suyun baskısını ve kat geçişlerindeki çatışmayı deler. Playtest'te "takıldım" şikâyeti kat geçişlerinde yoğunlaşırsa ilk aday mantling'dir (dikey dilim kararı).

---

## 3. Gelgit sistemi

### 3.1 Yükseklikler

| Ad | Dünya Z | Ne demek |
|---|---|---|
| **Low Water** | 0 | Kumsalın tamamı kuru (deniz kenarı Y = 0) |
| Kumsal (Low kat) | 0 → 200 (eğimli, 1:30) | Su kıyıdan duvara doğru **gözle görülür biçimde** ilerler |
| **Half Tide** | 300 | Kumsal tamamen sular altında (duvar dibinde 100 = bel); Half kat kuru |
| Half kat zemini | 400 | |
| High kat zemini | 800 | |
| **High Water** | 830 | Terasta 30 cm su: oyuncu yavaşlamaz (< diz) ama Scuttler ıslaktır. Kuru adalar ve vinç platformu Z = 900 |

Gri kutuda tüm düşey yüzeylere iki bant boyanır: Z = 300 (sarı, 20 cm) ve Z = 830 (kırmızı, 20 cm). Oyuncu suyun nereye çıkacağını duvardan okur (hikâye §3 kural 2).

### 3.2 Döngü: 12:00 (720 sn), tek sefer

Hikâyedeki standart görev döngüsü ~19 dk; prototipte **12 dk**'ya indirildi. Gerekçe: tek harita ve altı sandıkla 6 dakikalık slack boş kalır; soru flood hakkında olduğu için kısalan ebb ve slack'tir, flood 5 dk kalır. Eğri veridir; 19 dakikalık sürüm aynı haritada denenebilir.

Su yüksekliği, sunucu zamanından parça parça doğrusal eğriyle hesaplanır (her istemci kendi hesaplar):

| Parça | Zaman | Su Z (başlangıç → bitiş) | Hız (cm/sn) | Evre adı |
|---|---|---|---|---|
| E1 | 0:00–1:30 | 830 → 200 | −7,00 | **Ebb** |
| E2 | 1:30–2:00 | 200 → 150 | −1,67 | Ebb (yavaşlama) |
| SL1 | 2:00–4:00 | 150 → 0 | −1,25 | **Slack Low** (son çekilme) |
| SL2 | 4:00–5:00 | 0 (sabit) | 0 | Slack Low (durgun su) |
| F1 | 5:00–6:30 | 0 → 300 | +3,33 | **Flood** — kumsal gider |
| F2 | 6:30–7:00 | 300 (sabit) | 0 | Flood — Half Tide duraklaması (nefes) |
| F3 | 7:00–8:45 | 300 → 700 | +3,81 | Flood — Half kat gider |
| F4 | 8:45–9:00 | 700 (sabit) | 0 | Flood — kısa duraklama |
| F5 | 9:00–10:00 | 700 → 830 | +2,17 | Flood — teras ıslanır |
| SH | 10:00–12:00 | 830 (sabit) | 0 | **Slack High** — vinç savunması |

Eğrinin gerekçeleri:

- **Basamaklı yükseliş (F2, F4 duraklamaları):** testere dişi tempo. Düz doğrusal su "geri sayım" gibi okunur; duraklama hem nefes hem "şimdi geri inip bir sandık daha alır mıyım?" kararı verir.
- **Slack Low'da su hâlâ çekiliyor (SL1):** en alttaki ganimet en geç açılır. C noktası dönüşten yalnızca ~75 sn önce kurur → açgözlülük kendiliğinden flood'a taşar.
- **Eğimli kumsal:** F1'de su çizgisi kumsalda yatay **~100 cm/sn** ilerler (3,33 × 30). Yüklü oyuncudan (400) yavaş, ama gözle izlenir: tek GIF'lik kanca budur.

Hesaplanmış anlar (eğriden türetilmiştir; eğri değişirse yeniden hesaplanır):

| Olay | Zaman |
|---|---|
| Half kat diz altına iner / tamamen kurur (ebb) | 0:54 / 1:01 |
| Ganimet A (Z 150): göğüs altı (erişilebilir) / kuru | 1:18 / 2:00 |
| Ganimet B (Z 80): göğüs altı / kuru | 1:28 / 2:56 |
| Ganimet C (Z 20): göğüs altı / diz altı / kuru | 1:57 / 3:04 / 3:44 |
| **Çan (dönüş)** | **5:00** |
| C: ıslak / diz / bel / baş | 5:06 / 5:21 / 5:36 / 5:56 |
| İskele çöküğü dibi (Z 50, orta rota): diz / bel | 5:30 / 5:45 |
| B: ıslak / diz / bel | 5:24 / 5:39 / 5:54 |
| A: ıslak / diz / bel | 5:45 / 6:00 / 6:15 |
| Doğu rampa dibi (Z 167) / batı merdiven dibi (Z 187): bel | 6:20 / 6:26 |
| Half kat: ıslak / diz / bel / göğüs / baş | 7:26 / 7:39 / 7:53 / 8:02 / 8:10 |
| High teras ıslanır (30 cm'de kalır) | 9:46 |

### 3.3 Islaklık durumu

Derinlik `d = Su Z − ayak Z`. Oyuncu ve AI aynı ölçümü kullanır; eşikler aktöre göre ayrı veridir. Durum geçişinde 8 cm histerezis ve 0,2 sn gecikme (su çizgisinde titreme olmasın).

**Oyuncu:**

| Durum | d (cm) | Hareket | Silah | Diğer |
|---|---|---|---|---|
| **Dry** | < 50 | Etki yok (20+ cm'de yalnız sıçrama sesi) | Etki yok | |
| **Knee** (diz) | 50–99 | Tüm hızlar **×0,75** (yürüme 375, yüklü 300); zıplama ×0,8 | Etki yok | Ayak sesi yüksek |
| **Waist** (bel) | 100–164 | **×0,5** (yürüme 250, yüklü 200); koşu ve zıplama yok | Gaffer kullanılamaz (iki el; otomatik Purser'a geçer). Purser yalnız hip-fire, yayılım ×1,5, reload süresi ×1,25 | Sandık alınabilir/taşınabilir |
| ↳ alt eşik **Chest** (göğüs) | 135–164 | ×0,4 (yürüme 200) | **Ateşli silah yok**; yalnız Kedge | Sandık alınamaz (eldeki taşınmaya devam eder) |
| **Overhead** (baş üstü) | ≥ 165 | ×0,4 | Hiçbiri | Nefes 8 sn, sonra 20 can/sn. Eldeki sandık düşer |

Gerekçe: hikâye §3 kural 3 ile birebir (diz üstü yavaşlar, göğüste silah yok, baş üstünde boğulur). Bel eşiği ayrıca Wardens'ın kısa devre eşiğidir; oyuncu "ben belime kadar girebilirim, o giremez" asimetrisini öğrenir.

**Adlar koddaki `ENFWetness` ile aynıdır: `Dry`, `Knee`, `Waist`, `Overhead`** (eski "Head" adı kaldırıldı; `Head` yalnızca isabet bölgesidir, §4). Göğüs ayrı bir enum değeri değil, Waist içinde bir derinlik sabitidir (`ChestDepth` = 135).

**Silah kuralı (bağlayıcı, tek cümle):** ateşli silah kilidi ıslaklık durumuna değil derinliğe bağlıdır — **d < 100: tüm silahlar; 100 ≤ d < 135 (Waist): Gaffer yok, Purser yalnız hip-fire; d ≥ 135 (`ChestDepth`): ateşli silah yok, yalnız Kedge; d ≥ 165 (Overhead): hiçbiri.** Koddaki "Waist: weapons cannot be used" yorumu ve `WeaponsDisabledFrom = Waist` varsayılanı bu kurala göre düzeltilir; "TOO DEEP TO SHOOT" uyarısı Waist'te değil d ≥ 135'te gösterilir.

Eşik özeti (su renk geçişi ve materyal için de bunlar geçerlidir): diz **50**, bel **100**, göğüs **135**, baş üstü **165** cm.

### 3.4 Telgraf zamanlaması

Su asla sürpriz yapmaz (hikâye §3 kural 1). Her geçiş **ses + söz + HUD** üçlüsüyle haber verilir. Satırlar hikâye §8.4/§8.8'den; ⊕ işaretliler bu belge için yeni taslaktır (editör gerekir).

| Zaman | Çan | Aggie (telsiz) / Tannoy | HUD |
|---|---|---|---|
| 0:00 | — | Tannoy: "Good morning, Carrow Sands. Low water at four minutes past." ⊕ · Aggie: "Water's going out. Follow it down." | Gelgit saati: EBB |
| 2:00 | — | Aggie: "That's slack. Three minutes. Make them count." ⊕ (süre uyarlandı) | SLACK LOW, geri sayım 3:00 |
| 4:00 (T−60) | — | Aggie: "She's about to turn. Start thinking uphill." | Saat sarı yanıp söner |
| 4:30 (T−30) | 1 vuruş | — | |
| 4:50 (T−10) | 3 vuruş | — | Saat kırmızı |
| **5:00** | **5 sn sürekli çan** | Aggie: "Tide's turned. Up. Now." · Tannoy: "High water is coming. It is owed." | FLOOD; su uğultusu katmanı açılır |
| 6:30 | 2 vuruş | Aggie: "Half tide. Lower streets are gone." | Half Tide işareti dolar |
| 7:00 | 1 vuruş | Aggie: "She's coming again. Keep climbing." ⊕ | |
| 9:00 | 1 vuruş | Aggie: "Terrace next. Get to the line." ⊕ | |
| 10:00 | 3 vuruş | Tannoy: "Take what you carried. Leave the rest. It isn't yours." | SLACK HIGH, geri sayım 2:00 |
| 10:30 | — | Aggie: "Line's down. Clip on." | Vinç işareti |
| 11:30 | — | Aggie: "Thirty seconds on the line." ⊕ | |
| 12:00 | — | Aggie: "That's a shift. Kettle's on." / başarısızsa "Hauling you up. Leave the haul." | Sonuç ekranı |
| Oyuncu Waist'e girince (15 sn'de en çok 1) | — | Aggie: "You're wading. Get out of it." | |

Uğultu sesinin şiddeti su yükselme hızına bağlıdır (duraklamalarda kısılır): oyuncu duraklamayı kulağıyla duyar.

---

## 4. Silah kartları

**TTK kimliği:** oyuncu → düşman **0–0,9 sn** (PvE güç fantezisi: yem düşman tek-iki atışta gider); düşman → oyuncu **3–8 sn** (hata yapma payı var, ama iki fraksiyon arasında kalmak öldürür). Baskıyı tek tek düşmanların dayanıklılığı değil, **sayı + su** üretir.

Alan adları developer iskeletindeki `UNFWeaponData` ile aynıdır. Prototip düşmanlarında yalnız iki bölge vardır: `Body` ve `Head` (= zayıf nokta); `LimbMultiplier` iki silahta da 1,0.

### 4.1 Purser (toplu tabanca)

**İşi:** orta menzilde zayıf nokta; sandık taşırken ve bel hizasında suda çalışan tek ateşli silah. **Seçme nedeni:** "her durumda elimde." **Zayıflığı:** sürüye yetmez (6 mermi, 2,2 sn reload).

| Alan | Başlangıç değeri |
|---|---|
| `Damage` | 50 |
| `HeadshotMultiplier` | 2,0 |
| `LimbMultiplier` | 1,0 |
| `RoundsPerMinute` | 150 (atış aralığı 0,40 sn) |
| `bFullAuto` / `FireInputBuffer` | false / 0,10 |
| `MagazineSize` | 6 |
| `bInfiniteReserve` | true (güvenilir yedek: asla silahsız kalınmaz) |
| `ReloadTime` | 2,2 |
| `RangeMeters` | 60 |
| `DamageFalloffCurve` (m → çarpan) | (0 → 1,0), (20 → 1,0), (40 → 0,6), (60 → 0,6) |
| `HipSpread` / `AdsSpread` | 1,2 / 0,1 |
| `MoveSpread` / `MoveSpreadReferenceSpeed` | 0,8 / 500 |
| `AirborneSpread` | 3,0 |
| `SpreadPerShot` / `MaxBloom` / `SpreadRecoveryPerSecond` | 0,6 / 2,0 / 6 |
| `AdsSpreadModifier` | 0,3 |
| `RecoilPattern` (yaw, pitch) | (0,0; 2,4) (0,2; 2,2) (−0,2; 2,2) (0,3; 2,0) (−0,3; 2,0) (0,0; 2,0) |
| `RecoilLoopStartIndex` / `RecoilRandomness` | 3 / 0,15 |
| `AdsRecoilMultiplier` | 0,7 |
| `RecoilKickSpeed` / `RecoilRecoveryDelay` / `RecoilRecoverySpeed` / `RecoilResetTime` | 30 / 0,10 / 14 / 0,45 |
| `AdsFOV` / `AdsTime` | 70 / 0,18 |

**Recoil karakteri:** sert dikey tekme, tam geri dönüş. İki atış arasında (0,40 − 0,10) × 14 = 4,2° toparlar; tekme 2,4°. Yani ritimle atan oyuncu her atışta aynı noktaya döner: ödüllendirilen şey **ritim ve nişan**.

### 4.2 Gaffer (pompalı tüfek)

**İşi:** 0–7 m sürü temizleme ve sendeletme. **Seçme nedeni:** "önümdeki her şey gitsin." **Zayıflığı:** 10 m'den sonra etkisiz; sandık taşırken ve bel hizasında suda kullanılamaz; mermisi sınırlı.

| Alan | Başlangıç değeri |
|---|---|
| `Damage` | 15 (saçma başına) × **8 saçma** = 120 |
| `HeadshotMultiplier` | 1,2 (zayıf nokta işi Purser'da kalsın) |
| `LimbMultiplier` | 1,0 |
| `RoundsPerMinute` | 70 (atış aralığı 0,857 sn; pompa) |
| `bFullAuto` / `FireInputBuffer` | false / 0,15 |
| `MagazineSize` | 6 |
| `bInfiniteReserve` / `StartingReserveAmmo` | false / 30; haritada 2 mermi sandığı (sınırsız dolum, açıkta: §7.6) |
| `ReloadTime` | 2,8 (prototipte tek parça tam dolum; fişek fişek dolum dikey dilimde) |
| `RangeMeters` | 25 |
| `DamageFalloffCurve` (m → çarpan) | (0 → 1,0), (7 → 1,0), (18 → 0,3), (25 → 0,3) |
| `HipSpread` / `AdsSpread` | 3,5 / 2,8 (saçma konisi yarı açısı; her saçma koni içinde bağımsız) |
| `MoveSpread` / `AirborneSpread` | 0,5 / 1,5 |
| `SpreadPerShot` / `MaxBloom` | 0 / 0 |
| `RecoilPattern` | (0,0; 5,0) tek eleman |
| `RecoilLoopStartIndex` / `RecoilRandomness` | 0 / 0,6 |
| `AdsRecoilMultiplier` | 0,8 |
| `RecoilKickSpeed` / `RecoilRecoveryDelay` / `RecoilRecoverySpeed` / `RecoilResetTime` | 40 / 0,15 / 10 / 0,5 |
| `AdsFOV` / `AdsTime` | 80 / 0,22 |

**Recoil karakteri:** tek büyük tekme, pompa sırasında yavaş dönüş ((0,857 − 0,15) × 10 = 7,1° ≥ 5°: bir sonraki atışa hazır).

Koni çapı ve saçma hasarı (türetilmiş): 5 m → 61 cm / 15 · 7 m → 86 cm / 15 · 10 m → 122 cm / 12,1 · 12 m → 147 cm / 10,2 · 15 m → 183 cm / 7,4.

### 4.3 Kedge (kanca, yakın dövüş — herkeste)

| Değer | Başlangıç |
|---|---|
| Hasar | 40 (zayıf nokta çarpanı yok) |
| Vuruş aralığı | 0,60 (100 vuruş/dk); tuştan isabete 0,15 |
| Erişim | 180 cm, 60° koni, ilk temas eden tek hedef |
| İtme | Scuttler 200 cm, Attendant 100 cm |
| Tutuş | **Silah yuvası değil, ayrı yakın dövüş tuşu.** Silah değiştirmeden **sol elle** savrulur; sağ el ateşli silahı tutmaya devam eder. 0,40'lık silah değiştirme süresi Kedge'e uygulanmaz (0,15'lik isabet bu yüzden mümkün) |
| Savurma sırasında | 0,60 boyunca ateş ve ADS kilitli (ADS'deyse çıkar); koşu kesilir. Savurma bitince silah anında hazırdır. Gerekçe: Purser + Kedge hasarı üst üste binmesin; yakın dövüş bir tercih olsun |
| Reload ile ilişkisi | **Savurma reload'u iptal eder:** süren reload kesilir, ilerlemesi sıfırlanır, mermi verilmez; kendiliğinden yeniden başlamaz (oyuncu tekrar başlatır). Reload sürerken savurma tuşu her zaman kabul edilir. Gerekçe: sol el ya fişek sürer ya savurur. Kedge "pompa dolarken" boşluğu doldurur ama bedeli reload'dur: karar oyuncuda |
| Kullanım | Göğüs hizasına kadar suda (d < 165) çalışır; d ≥ 135'te eldeki tek araçtır (ateşli silah iner, Kedge hazır bekler). Sandık taşırken çalışmaz (sol kol sandıkta) |

**İşi:** son çare ve "mermi bitti, pompa dolarken" boşluğu. Kurumuş (Parched) Scuttler'ı tek vuruşta öldürür: kuruya çekmenin ödülü.

Ayar adayı (ilk tur verisi gerektirirse): "reload'u kaybettim" şikâyeti yoğunsa iptal yerine duraklat-devam et denenir; bu animasyon işi doğurur, prototipte yapılmaz.

### 4.4 İskelette olmayan, prototip için gereken alanlar (developer'a istek)

| İhtiyaç | Öneri |
|---|---|
| Saçma sayısı | `PelletsPerShot` (int, varsayılan 1). `Damage` saçma başınadır |
| Taşırken kullanılabilirlik | `bUsableWhileCarrying` (Purser true) |
| İki el / ıslaklık kısıtı | `MaxUsableWetness` (Gaffer: `Knee`; Purser: `Waist`) + tüm ateşli silahlar için ortak derinlik kilidi `ChestDepth` = 135 (§3.3 silah kuralı). Purser'da Waist'te ADS kapalı |
| Sendeletme | Hasar olayında tek atışın toplam hasarı düşmana iletilmeli (Attendant eşiği için, §5.2) |
| Yakın dövüş | Kedge ayrı küçük veri varlığı ya da bileşen; karar developer'da. Ayrı girdi eylemi (silah yuvası değil); savurma reload'u iptal eder ve 0,60 boyunca ateş/ADS'yi kilitler (§4.3) |

### 4.5 TTK tablosu

Formül: `gereken atış N = ⌈can / atış hasarı⌉`, `teorik TTK = (N − 1) × 60 / RPM`. Pratik TTK: `N' = ⌈N / isabet⌉`, `(N' − 1) × 60 / RPM`. İsabet varsayımı: Purser gövde %70, Purser zayıf nokta %50; Gaffer için isabet yerine "kaç saçma değdi" varsayılır.

| Silah / durum | Atış hasarı | **Scuttler (60)** N · teorik · pratik | **Attendant (150)** N · teorik · pratik |
|---|---|---|---|
| Purser gövde, ≤ 20 m | 50 | 2 · 0,40 · 0,80 (3 atış) | 3 · 0,80 · 1,60 (5 atış) |
| Purser zayıf nokta, ≤ 20 m | 100 | 1 · 0,00 · 0,40 (2 atış) | 2 · 0,40 · 1,20 (4 atış) |
| Purser gövde, 40 m | 30 | 2 · 0,40 · 0,80 | 5 · 1,60 · 2,80 (8 atış) |
| Gaffer, ≤ 7 m, 8/8 saçma | 120 | 1 · 0,00 | 2 · 0,86 |
| Gaffer, ≤ 7 m, 6/8 saçma (pratik) | 90 | 1 · 0,00 | 2 · 0,86 |
| Gaffer, ≤ 7 m, 4/8 saçma | 60 | 1 · 0,00 | 3 · 1,71 |
| Gaffer, 12 m, 4/8 saçma (pratik) | 40,9 | 2 · 0,86 | 4 · 2,57 |
| Kedge | 40 | 2 · 0,60 | 4 · 1,80 |
| Parched Scuttler (aldığı hasar ×1,5) | — | Purser gövde 75 → **1 atış**; Kedge 60 → **1 vuruş** | — |

Okuma: Scuttler'ı Gaffer yakında tek atışta, Purser iki atışta indirir; Attendant'a Purser iki zayıf nokta atışı (0,40) Gaffer'ın iki atışından (0,86) hızlıdır. İki silahın işi çakışmaz. Sürekli hasar: Purser 125 can/sn, Gaffer 140 can/sn (tam isabet), Kedge 67 can/sn.

Tüm satırlar §5'teki can değerleriyle hesaplanarak doğrulandı; can, hasar ya da RPM değişirse tablo yeniden hesaplanır.

---

## 5. Düşman kartları

İki arketip suya ters tepki verir: **su yükseldikçe Attendant yukarı kaçar ve yukarıda yoğunlaşır, Scuttler aşağıdan suyla gelir.** Oyuncu ikisinin arasındadır. Prototipin sınadığı çatışma budur.

### 5.1 Scuttler (The Drift — baskıcı sürü)

**Sorduğu soru:** "Kaç tanesini aynı anda tutarsın?" **Siluet:** köpek boyu, alçak, geniş yengeç. **Ses:** tıkırtı korosu (sayı arttıkça koro kalınlaşır: oyuncu sayıyı duyar).

| Değer | Başlangıç | Gerekçe |
|---|---|---|
| Can | 60 | §4.5 |
| Kapsül | **yarıçap 35 / yarı yükseklik 35** (Ø 70 küre; toplam yükseklik 70) | UE kapsülünde yarı yükseklik yarıçaptan küçük olamaz; eski "yarıçap 45, yükseklik 70" geçersizdi. Tepe 70 (model 68): alçak siperin (110) arkasında 40 cm payla görünmez, ses önemli. Dar kapsül 400'lük geçitlerde (L1, H3) sürünün sıkışmasını azaltır; kabuk (78 en) kapsülden 4'er cm taşar, kabul |
| İsabet hacimleri | Gövde kutusu (kabuk ölçüsü, `Body`), kese küresi (`Head`), iki kıskaç (`Body`). **Bacaklar isabet almaz** ve çarpışmaya girmez: mermi aralarından geçer | 17'li sürüde 6 bacak × fizik gövdesi pahalı; hedef zaten alçak ve geniş. Playtest'te "bacağa vurdum saymadı" şikâyeti çıkarsa bacak gövdesi eklenmez, gövde kutusu yanlara 10 cm genişletilir |
| Zayıf nokta | Sırttaki turkuaz kese (`Head` bölgesi), ~25 cm küre; yalnız arkadan/üstten görünür | Yüksek zemindeki oyuncuyu ödüllendirir |
| **Wet** eşiği | kendi ayağından d ≥ 20 | Terastaki 30 cm suda ıslaktır |
| Hız — Wet | **504** (×1,4) | Yürüyen oyuncuya (500) eşit, diz suyundaki oyuncudan (375) hızlı |
| Hız — Dry | **360** | Yüklü oyuncudan (400) yavaş: kuruda taşıyıcı kaçabilir |
| Hız — Parched | 300 | |
| Can yenileme — Wet | 2 sn hasarsızlıktan sonra 6 can/sn | Yarım bırakılan Scuttler suda toparlar |
| **Parched** | Kesintisiz 20 sn Dry kalınca: aldığı hasar ×1,5, tıkırtı kurur. Görsel: **değer sabit kalır (gövde koyu, L* ≤ 26)**; doygunluk düşer, yüzey tam matlaşır, kabuk sırtında ince tebeşir beyazı tuz çizgileri çıkar, kese ışıltısı söner ve süt rengine döner. Suya girince anında sıfırlanır | "Kuruya çek" karşı oyunu (hikâye §4.2). Eski "renk solar" ifadesi gövdeyi açmak demek değildir: açılan gövde kuru zeminle karışır, tam "tek atışlık" hedefin seçileceği anda. Yeterince ayırt edilmezse ikinci aday siluet değişimidir (kabuk kenarlarının pul pul kalkması), değer değişimi değil |
| Saldırı 1 — **Pinch** | Erişim 150. Telgraf 0,5 sn (kıskaçlar kalkar + tek sert "klak") → 12 hasar → toparlanma 0,8 sn; bekleme 1,5 sn | Geri adımla kaçınılır |
| Saldırı 2 — **Lunge** (yalnız Wet) | 300–500 mesafeden. Telgraf 0,6 sn (çöker + tıslama, su sıçratır) → 0,4 sn sıçrayış → 18 hasar; ıskalarsa 1,0 sn açıkta | Yana adım; ıskalayan kolay hedef |
| Saldırı token'ı | **Hedef oyuncu başına 2** eşzamanlı saldıran; token'sız olanlar 250–400 mesafede döner | Okunabilirlik: 6 yengeç aynı anda vurmaz |
| Sendeleme | Ölmeyen her isabette 0,3 sn duraklar; Gaffer/Kedge iter | |
| Suyla davranış | Yolunu suyun içinden seçer (ıslak zemin maliyeti düşük); düşmüş oyuncuya saldırmaz | |
| Doğuş | D noktaları, yalnız nokta su altındayken (§7.5) | "Suyla gelir" |

**Bark yerine ses işaretleri (oynanış bilgisi):**

| Durum | Ses | Oyuncuya anlamı |
|---|---|---|
| Fark etti | Tıkırtı hızlanır | Geliyor |
| Pinch telgrafı | Tek sert "klak" | Geri çekil |
| Lunge telgrafı | Tıslama + sıçrama sesi | Yana kaç |
| Wet → Dry | Islak şapırtı kesilir, kuru tıkırtı | Yavaşladı |
| Parched | Çatırtılı, kuru ses | Tek atışlık |
| Suya geri girdi | "Şlup" + koro kalınlaşır | Hızlandı, iyileşiyor |

### 5.2 Attendant (The Wardens — temel menzilli)

**Sorduğu soru:** "Siperden çıkabilir misin?" **Siluet:** kasketli, ince, dik insansı; başında sinyal lambası. **Ses:** bilet zımbası "klik-klak".

| Değer | Başlangıç | Gerekçe |
|---|---|---|
| Can | 150 | §4.5 |
| Kapsül | yarıçap 40, yarı yükseklik 95 (toplam yükseklik 190) | |
| Zayıf nokta | Baştaki sinyal lambası (`Head`). İsabet küresi **yarıçap 18, merkez Z 172** (Z 154–190: kapsül tepesini aşmaz). Görsel lamba merkezi Z 175'te kalır; küre lamba yuvasını (Z 160–190) tümüyle kapsar | Lamba aynı zamanda telgraf göstergesi: bakılan yer = vurulacak yer. Yarıçap küçültülmedi: 12–25 m tercih menzilinde hedef zaten küçük, §4.5'teki %50 zayıf nokta isabet varsayımı bu boyuta göre |
| Alçak siperde lamba | Attendant 110'luk siperin arkasında diz kırar: gövde gizlenir, **lamba görünür kalır** (lamba merkezi Z ≥ 135; isabet küresi Z 117–153, tamamı siperin üstünde). Tam siperin (200+) arkasında tümüyle gizlidir; yaylım telgrafı her zaman lamba oyuncuya açıkken başlar (önce siperden çıkar, sonra telgraf) | Lamba telgraf göstergesidir; görünmeyen telgraf haksız ölümdür. Alçak siper 150 canlık gövdeyi saklar ama zayıf noktayı açık bırakır: Purser'ın işi |
| Hız | Devriye 300, yer değiştirme 420 | Yüklü oyuncuyu kovalayamaz; işi alan tutmak |
| Tercih menzili / azami | 1200–2500 / 3500 | Purser'ın tam hasar bandı (≤ 20 m) içinde |
| Saldırı 1 — **Punch Burst** | Telgraf 0,7 sn (lamba kırmızı + "klik-klak") → 0,15 sn arayla 3 mermi (yavaş mermi, 3500 cm/sn, öngörüsüz nişan, yayılım 1,5°) → mermi başına 10 hasar → bekleme 2,0 sn | 25 m'de uçuş 0,7 sn: yana yürüyen oyuncu kaçar, duran vurulur. Tam isabetle ~10 can/sn |
| İlk yaylım | Fark ettikten sonraki ilk yaylım kasten ıskalar | "Zeki görünür"; oyuncuya sipere girme süresi |
| Servis (reload) | 4 yaylımdan sonra 3,0 sn açıkta. Lamba **kırmızıya dönmez ama kararmaz**: sakin renginde (`#FFF4D6`) düşük şiddetle 1 Hz göz kırpar; baş öne eğik, göğüs kapağı açık. Zayıf nokta bu sürede etkin kalır (×2,0) | Bark ile fırsat penceresi. Oyuncunun en çok vurması gereken anda hedef göstergesi kapanmaz. Kural: kırmızı = ateş geliyor, yavaş göz kırpma = savunmasız. Kehribar kullanılmaz (Etkileşim sarısıyla karışır); ikinci işaret baş/kapak pozu |
| Saldırı 2 — **Shove** | Oyuncu ≤ 150: telgraf 0,4 sn (kol geri) → 8 hasar + 300 cm itme → **toparlanma 0,50 sn** (toplam 0,90; toparlanmada ateş etmez, yer değiştirmez) | Köşede sıkıştıran oyuncuya karşılık; oyuncuyu suya/rampadan aşağı atabilir. Toparlanma, ıskalatan ya da itilen oyuncuya karşılık penceresi verir (0,50 = Purser ile bir zayıf nokta atışı + hazırlık) |
| Ateş token'ı (küresel) | 1 oyuncu: 2 · 2 oyuncu: 3 · 3–4 oyuncu: 4 | Token'sız olan siperde bekler ya da yer değiştirir |
| Sendeleme | Tek atışta ≥ 80 hasar (Purser zayıf nokta, Gaffer 6+ saçma): 0,6 sn, telgrafı keser, 150 cm geri iter | Saldırganlığı ödüllendirir; suya itme aracı |
| **Su: kaçınma** | Kendi ayağından d ≥ 30 olan zemine gönüllü girmez | |
| **Su: diz (50–99)** | Hız ×0,7; kıvılcım | |
| **Su: kısa devre** | d ≥ 100 kesintisiz **2,0 sn** → devre dışı (ölüm) | Hikâye §4.1. Prototipte çevreye elektrik hasarı **yok** (kapsam) |
| **Su: geri çekilme** | Su yükselirken ayağının 150 cm altına geldiğinde bir üst kata/kuru bölgeye taşınır; taşınırken ateş etmez (elindeki yaylımı bitirir) | Sırtı dönük Attendant = ödül; aynı zamanda yukarıda yoğunlaşmayı üretir |
| Siper | Tam ya da alçak siper arkasında durur; 6–10 sn'de bir siper değiştirir | |
| Düşmüş oyuncu | Hedef almaz | |
| Doğuş | W noktaları, yalnız nokta kuruyken (§7.5); 1,5 sn önce kepenk sesi | |

**Bark'lar (hikâye §8.6'dan; her biri oynanış bilgisi, ≤ 2 sn):**

| Durum → oyuncuya anlamı | Satır |
|---|---|
| Fark etti | "Good afternoon. May I see your ticket?" |
| Çatışma başlıyor (ilk, ıskalayan yaylım) | "This area is closed to the public." |
| Servis → **bas** | "One moment, please. Your warden is being serviced." |
| Yer değiştiriyor/kanat → **yanını kolla** | "Please use the other entrance." |
| Oyuncu 8+ sn aynı siperde → üstüne geliyor | "Loitering is not permitted." |
| Diz hizasında suda → **it, kısa devre fırsatı** | "Warning. No bathing after s— s— six." |
| Geri çekiliyor → **sırtı dönük** | "Patrons are advised to move to higher ground." |
| Devre dışı | "Thank you for visiting Carrow Sands." |

### 5.3 Eşzamanlı düşman tavanı ve oyuncu sayısı çarpanı

Çarpan `M(N) = 1 + 0,6 × (N − 1)` → 1 oyuncu 1,0 · 2 oyuncu 1,6 · 3 oyuncu 2,2 · 4 oyuncu 2,8. **Düşman canı ve hasarı oyuncu sayısıyla değişmez**; yalnız sayı değişir (TTK her oyuncu sayısında aynı hissettirsin).

| | 1 oyuncu | 2 | 3 | 4 |
|---|---|---|---|---|
| Aynı anda canlı Scuttler tavanı | 6 | 10 | 13 | 17 |
| Aynı anda canlı Attendant tavanı | 3 | 5 | 7 | 8 |
| Dalga büyüklüğü (§8.2'deki solo sayı × M, en yakına yuvarla, en az 1) | ×1,0 | ×1,6 | ×2,2 | ×2,8 |

Tavan doluyken gelen dalga atlanır, kuyruğa alınmaz.

---

## 6. Oyuncu canı, düşme/kaldırma, solo kuralı

### 6.1 Can

| Değer | Başlangıç | Gerekçe |
|---|---|---|
| Azami can | 100 | İskelet varsayılanı |
| Yenilenme | 5 sn hasarsızlıktan sonra 8 can/sn, **en çok 50'ye** kadar | Kısa nefes verir; tam iyileşme için risk gerekir |
| İlk yardım kutusu | 2 adet (Half ve High, açıkta); kutu başına kullanım = oyuncu sayısı; 1,5 sn basılı tut → 100 can | Oyuncuyu siperden çıkarır (skill: alım riskli noktada) |
| Dost ateşi | Kapalı | |
| Ölüm süresi (türetilmiş) | 2 Scuttler: ~12 can/sn → ~8 sn · + tam isabetli 2 Attendant: ~32 can/sn → ~3 sn | Tek fraksiyon affeder, **arada kalmak** öldürür |

### 6.2 Düşme ve kaldırma (2–4 oyuncu)

| Değer | Başlangıç |
|---|---|
| 0 can → **Downed** | Sürünür (100 cm/sn), ateş edemez, sandığı düşer; düşmanlar hedef almaz |
| Kanama süresi | 60 sn |
| Suda düşmüşse (d ≥ 50) | Süre 3× hızlı akar (≈ 20 sn): su düşeni tahsil eder |
| Kaldırma | Arkadaş 3,0 sn basılı tutar (hasar kesmez, bırakınca sıfırlanır) → 40 can, 2 sn dokunulmazlık |
| Süre biterse | **Lifeline** çeker: oyuncu 45 sn sonra vinç platformunda 50 canla yeniden iner |
| Herkes aynı anda Downed / çekilmiş | Görev başarısız ("Hauling you up. Leave the haul.") |

Gerekçe: düşen oyuncuyu oyundan atmıyoruz (12 dakikalık döngüde seyirci kalmak playtest'i zehirler) ama bedeli var: 45 sn eksik adam ve en üst kattan yeniden iniş.

### 6.3 Tek oyuncu

| Kural | Başlangıç |
|---|---|
| Düşman sayısı | M(1) = 1,0 (§5.3); Attendant ateş token'ı 2 |
| **Spare line** | Görev başına 1 kez: 0 canda 3 sn sonra kendiliğinden kalkar, 40 can, 2 sn dokunulmazlık, 400 cm yarıçapta düşmanları iter |
| İkinci düşme | Görev başarısız |
| Kota | 2 sandık (§6.4) |
| Fark edilen risk | Taşırken eskort yok. Solo'nun aracı: sandığı bırak-savaş-al ve Purser'ın 6 mermisi. Playtest ölçütü 7 bunu sınar |

### 6.4 Sandık ve çıkış kuralları

| Kural | Başlangıç | Gerekçe |
|---|---|---|
| Ganimet noktası | 3 kilitli dolap (A, B, C); her biri **2 Strongbox** | Zamandan fazla ganimet: açgözlülük kararı doğar |
| Dolabı açma | 15 sn çevirme kolu (basılı tut; ilerleme korunur, birden çok oyuncu hızlandırmaz) | Ganimet noktasında savunulan bir an yaratır |
| Erişim | Göğüs altı suda açılır/alınır (d < 135) | Erken, ıslak ve riskli almak mümkün; özel kural yok |
| Taşıma | Oyuncu başına 1 sandık | |
| Teslim | Vinç platformundaki yük ağına (400 × 400) bırak; istenildiği an bırakılabilir, çıkışta sayılır | Erken teslim edip geri inmek serbest |
| **Kota** | Oyuncu sayısı + 1 (solo 2 … dört kişi 5); azami 6 | Dört kişi tek seferde bitiremez: en az bir oyuncu ikinci kez iner |
| Vinç | 10:30'da iner. Her oyuncu 3 sn'de kancalanır. Tüm ayaktakiler kancalanınca ya da 12:00'de görev biter | |
| Sonuç | Başarı: ağda kota kadar sandık. Skor: sandık sayısı | Ekonomi yok; skor yeter |
| Su altında kalan sandık | Yerinde kalır; ulaşabilen alır | |

---

## 7. Promenade gri kutu planı

### 7.1 Genel

- Ayak izi **12000 × 14000** (120 × 140 m). X doğu, Y kuzey (yokuş yukarı). Deniz güneyde (Y < 0).
- Üç teras kuzeye doğru yükselir: **Low** kumsal (Z 0→200, eğim 1:30) → **Half** sahil yolu ve binaların içi (Z 400) → **High** otel terası ve bina çatıları (Z 800).
- Oyuncu **vinç platformunda başlar** (çıkışı ilk gören oyuncu, dönüşte yolu arar değil hatırlar) ve çekilen suyu aşağı izler. Aynı rotalar flood'da ters yönde, düşmanlı ve yüklü yürünür.
- Vinç direği (Z 900 platform + 600 yüksek direk) her kattan görünen **landmark**'tır.

### 7.2 Üstten kroki

1 karakter = 200 cm (X), 1 satır = 400 cm (Y); satır etiketi satırın üst kenarıdır. Kroki şematiktir; **ölçü tablosu (§7.3) bağlayıcıdır.**

```
Y(cm)  X=0                        X=6000                        X=12000
14000  |############################################################|  otel cephesi (kapalı)
13600  |.....w........................w..............o.........w....|
13200  |............................................................|
12800  |............................WWWW............................|  HIGH  Z=800
12400  |............................WWWW............................|  (W: vinç, Z=900)
12000  |........................................+...................|
11600  |...............o............................................|
11200  |.....................a......................................|
10800  |............................................................|
10400  |..d...................d..................d...............d..|
10000  |:::::::::::==^^^^==:::%%%%%%%%%%%%%^^::::::::^^:::::::::::::|
 9600  |:::::::::::==^^^^==:::%%%%%%%%%%%%%^^::::::::^^:::::::::::::|  çatılar ':' Z=800
 9200  |:::::::::::========:::%%%%%%%%%%%%%^^::::::::^^:::::::::::::|  H1 plaza  H2 arcade  H3 ara sokak
 8800  |:::::::::::========:::%%%%%%%%%%%%%%%::::::::^^:::::::::::::|
 8400  |:::::::::::========:::%%%%%%%%%%%%%%%::::::::==:::::::::::::|
 8000  |:::::::::::========:::%%%%%%%%%%%%%%%::::::::==:::::::::::::|
 7600  |============================================================|
 7200  |=====w=========+====w====================w=============w====|  HALF  Z=400
 6800  |==================================a=========================|  (sahil yolu)
 6400  |====d====================d=============d================d===|
 6000  |,,,,,,,^^,,,,,,,,,,,,,,,,,,PPPPP,,,,,,,,,,,,,,,,,,^^^,,,,,,,|  deniz duvarı (Y=6000)
 5600  |,,,,,,,,,,,,,,,,,,,,,,,,,,,PPPPP,,,,,,,,,,,,,,,,,,^^^,,,,,,,|  L1 merdiven      L3 kızak
 5200  |,,,,,,,,,,,,,,,,,,,,,,,,,,,PPPPP,,,,,,,,,,,,,,,,,,^^^,,,,,,,|
 4800  |,,,,,,,,,,,,A,,,,,,,,,,,,,,PPPPP,,,,,,,,,,,,,,,,,,,,,,,,,,,,|
 4400  |,,,,,,,,,,,,,,,,,,,,,,,,,,,PPPPP,,,,,,,,,,,,,,,,,,,,,,,,,,,,|  LOW  Z=0→200
 4000  |,,,,,,,,,,,,,,,,,,,,,,,,,,,PPPPP,,,,,,,,,,,,,,,,,,,,,,,,,,,,|  (kumsal, eğimli)
 3600  |,,,,,,,,,,,,,,,,,,,,,,,,,,,PdPPP,,,,,,,,,,,,,,,,,,,,,,,,,,,,|  P: iskele güvertesi Z=400
 3200  |,,,,,,,,,,,,,,,,,,,,,,,,,,,PPPPP,,,,,,,,,,,,,,,,,,,,,,,,,,,,|
 2800  |,,,,,,,,,,,,,,,,,,,,,,,,,,,RRRRR,,,,,,,,,,,,,,,,,,,,,,,,,,,,|  R: çökmüş güverte (L2 rampa)
 2400  |,,,,,,,,,,,,,,,,,,,,,,,,,,,RRRBR,,,,,,,,,,,,,,,,,,,,,,,,,,,,|
 2000  |,,,,,,,,,,,,,,,,,,,,,,,,,,,RRRRR,,,,,,,,,,,,,,,,,,,,,,,,,,,,|
 1600  |,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,|
 1200  |,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,|
  800  |,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,C,,,,,,,|
  400  |,,,,,,,d,,,,,,,,,,,,d,,,,,,,,,,,,,,,,,,,d,,,,,,,,,,,,,,,,d,,|
    0  +------------------------------------------------------------+   deniz (Y<0)
```

Lejant: `,` kumsal · `=` Half zemin · `:` çatı (High, yürünür) · `.` High teras · `%` arcade salonu (içi Z 400, üstü çatı Z 800) · `^` merdiven/rampa · `P` iskele güvertesi · `R` çökmüş güverte rampası · `W` vinç platformu · `A B C` ganimet · `o` kuru ada · `w` Warden doğuş · `d` Drift doğuş · `+` ilk yardım · `a` mermi sandığı.

### 7.3 Ölçü tablosu (bağlayıcı)

| Öğe | X | Y | Z | Not |
|---|---|---|---|---|
| **Low — kumsal** | 0–12000 | 0–6000 | Y/30 (0→200) | Tek eğimli düzlem |
| Deniz duvarı | 0–12000 | 6000 | 200→400 | 200 yüksek; Half'tan kumsala atlanır (hasar yok), kumsaldan çıkılamaz |
| İskele güvertesi | 5500–6500 | 2700–6000 | 400 | 1000 geniş, kenarlarda 110 **dolu parapet** (alçak siper; açık korkuluk değil, §7.6); Half ile aynı kot |
| İskele ayakları | 5500–6500 | 1500–6000 | zemin→400 | 100 × 100 kolon, 500 aralık ızgara (tam siper) |
| **L2 — çökmüş güverte** | 5500–6500 | 1500–2700 | 50→400 | Rampa: koşu 1200, yükseliş 350 (16°), 1000 geniş. Merkezî, doğrudan, **açık**; dibi en erken su alır (bel 5:45) |
| **L1 — batı merdiveni** | 1500–1900 | 5600–6000 | 187→400 | 400 geniş, koşu 400 (26,6°), iki yanı 250 yüksek duvar. Dar, **korunaklı**; en geç kapanır (bel 6:26) |
| **L3 — doğu kızağı** | 10000–10600 | 5000–6000 | 167→400 | 600 geniş rampa, koşu 1000 (13°). Geniş, yumuşak, yarı açık (bel 6:20) |
| Ganimet **A** | 2500 | 4500 | 150 | Plaj kulübeleri arasında; L1'e 14 m |
| Ganimet **B** | 6000 | 2400 | 80 | Çökmüş güvertenin altında (üst boşluk ≥ 230), ayaklar arasında |
| Ganimet **C** | 10500 | 600 | 20 | Deniz kenarında tekne enkazı; L3'e 44 m açık kumsal |
| **Half — sahil yolu** | 0–12000 | 6000–7600 | 400 | 1600 derin açık şerit |
| Batı blok (dolu) | 0–2200 | 7600–10000 | çatı 800 | |
| **H1 — Grand Steps plazası** | 2200–3800 | 7600–10000 | 400 | Basamaklar X 2600–3400, Y 9200–10000 (800 geniş, koşu 800, 26,6°). Geniş, doğrudan, iki çatıdan **görülür** |
| Orta blok (dolu) | 3800–4500 | 7600–10000 | çatı 800 | |
| **H2 — Arcade salonu** | 4500–7500 | 7600–10000 | iç 400, tavan 780 | 2 ön kapı (150 × 250) X 5200 ve X 6800'de; iç merdiven X 7000–7400, Y 8900–10000: iki kol (her biri koşu 400, yükseliş 200) + 300 sahanlık; tavanda merdiven boşluğu. **Kapalı, yakın dövüş** (Gaffer alanı) |
| Doğu blok 1 (dolu) | 7500–9000 | 7600–10000 | çatı 800 | |
| **H3 — ara sokak rampası** | 9000–9400 | 7600–10000 | 400→800 | 400 geniş; düz 7600–8400, rampa 8400–10000 (koşu 1600, 14°). Uzun, dar, **tek görüş hattı** (Purser alanı) |
| Doğu blok 2 (dolu) | 9400–12000 | 7600–10000 | çatı 800 | |
| Çatı parapeti | çatıların Y 7600 ve plaza/sokak kenarları | — | 800→910 | 110 alçak siper; aşağıyı gören güç pozisyonu |
| **High — teras** | 0–12000 | 10000–13600 | 800 | |
| **Vinç platformu** | 5600–6400 | 12100–12900 | 900 | 800 × 800; dört yanında 3 basamak (33 cm). Yük ağı 400 × 400 ortada. **Oyuncu başlangıcı** |
| Kuru ada 1 — bandstand | merkez 3000 | 11500 | 900 | Ø 600, 100'lük alçak duvar |
| Kuru ada 2 — otel giriş sahanlığı | merkez 9000 | 13300 | 900 | 800 × 400 |
| Otel cephesi | 0–12000 | 13600–14000 | — | Kapalı; harita sınırı |
| İlk yardım | 3000 / 8000 | 6900 / 11800 | 400 / 800 | İkisi de açıkta |
| Mermi sandığı | 6900 / 4200 | 6800 / 11000 | 400 / 800 | İkisi de açıkta |

Kat geçişleri özeti: **Low → Half 3 çıkış** (L1, L2, L3), **Half → High 3 çıkış** (H1, H2, H3). Her kattan en az iki çıkış kuralı (hikâye §9.1) bir çıkış su altında kalsa da tutar. Çıkmaz yok: arcade'in iki ön kapısı ve merdiveni, iskelenin hem yola hem kumsala bağlantısı var.

### 7.4 Rota uzunlukları ve hedef koşu süresi

Hesap: düz parçalar + rampaların eğik uzunluğu; düşmansız.

| Rota | Uzunluk | Yüklü (400) | Yürüme (500) | Koşu (650) |
|---|---|---|---|---|
| A → L1 → H1 → vinç | ~101 m | **25 sn** | 20 sn | 15 sn |
| B → L2 (iskele) → H2 (arcade) → vinç | ~126 m | **31 sn** | 25 sn | 19 sn |
| C → L3 → H3 → vinç | ~139 m | **35 sn** | 28 sn | 21 sn |

**Hedef:** alt kattan vince **yüklü ve düşmansız 25–35 sn; çatışmalı 60–90 sn.** Hikâye §9.1'deki "60–90 sn" çatışmalı süre olarak yorumlandı: düşmansız 60–90 sn, yüklü hızla 240–360 m rota demektir ve gri kutuda boş koşu üretir. Çatışmalı süre telemetriden ölçülür (sandığın alınması → ağa bırakılması); 60 sn altı = harita küçük/düşman az, 120 sn üstü = flood penceresine sığmıyor.

Zaman bütçesi denetimi: C'den çan anında yüklü çıkan oyuncu L3 dibine düşmansız 11 sn'de varır; C 5:21'de diz, L3 dibi 6:20'de bel olur. Yani düşmansız bolca pay var; ~40 sn çatışma gecikmesiyle rampaya hâlâ kuru varılır, ~70 sn gecikmeyle rampa dibi bel suyundadır (yarı hız, Gaffer yok, arkada ıslak Scuttler). Su sert bir duvar değil, **daralan bir paydır.**

### 7.5 Düşman doğuş noktaları

| Tür | Nokta | Konum | Etkin olduğu koşul | Telgraf |
|---|---|---|---|---|
| Drift **D-Low** ×5 | Deniz kenarı ×4, iskele altı ×1 | Y ≈ 200 (X 1500, 4000, 8000, 11500); (5700, 3400) | Slack'te: deniz kenarındakiler her zaman (Scuttler suda doğar, kuru kumsalı geçer). Flood'da: su çizgisinin en çok 800 cm gerisindeki su altı nokta | Su yüzeyinde köpük + tıkırtı, 1,5 sn |
| Drift **D-Half** ×4 | Yol kenarı menfez ızgaraları | Y ≈ 6200 (X 800, 5000, 7800, 11200) | Su Z ≥ 350 (7:13'ten sonra) | Izgaradan su fışkırır, 1,5 sn |
| Drift **D-High** ×4 | Teras olukları | Y ≈ 10400 (X 500, 4500, 8300, 11500) | Su Z ≥ 750 (9:23'ten sonra) | Aynı |
| Warden **W-Half** ×4 | Bina cephesindeki kulübe/kepenk kapıları | Y ≈ 7600 cephesi (X 1000, 4100, 8200, 11000) | Nokta kuru (Su Z < 400) | Kepenk gürültüsü, 1,5 sn |
| Warden **W-High** ×3 | Otel kapıları | Y ≈ 13600 (X 1000, 6000, 11000) | Her zaman (teras 30 cm'de kalır; d < 100) | Aynı |

Kural: doğuş noktası oyunculardan en az 1500 cm uzakta ve görüş hattı dışında olanlardan seçilir; uygun nokta yoksa dalga 5 sn ertelenir. Drift noktaları her zaman oyuncunun **aşağısında/arkasında**, Warden noktaları **yukarısında/önünde** kalır: "arada kalma" düzeni haritadan gelir, betikten değil.

### 7.6 Siper yerleşim ilkeleri

| İlke | Sayı | Gerekçe |
|---|---|---|
| Siperden sipere mesafe (ana rota) | 800–1200 | Yüklü 2–3 sn (skill: 2–4 sn) |
| Siperden sipere (yan rota) | 500–800 | Yan yol uzun ama korunaklı |
| Alçak siper / tam siper | 110 / **200+** (arası, 120–190, yapılmaz) | Metrikler; stil rehberi §6 ile aynı. Eski "220+" eşiği 200'e çekildi: 200, ayaktaki oyuncuyu (180) ve Attendant'ı (190) kapatır. Çömelmiş kapsül üstü 110 (§2.1) |
| Siper sınıfları | **Dolu parapet 110 = alçak siper** (mermiyi durdurur). **Açık korkuluk 110 = siper değildir**: oyuncuyu ve AI'ı durdurur, mermiyi ve görüşü geçirir; siper karışımına sayılmaz. **Kiosk/büfe = tam siper** (300 × 300 × 300). **Oyun makinesi = tam siper** (yükseklik 200, taban en az 80 × 80) | Environment-artist sorusunun yanıtı. Okunaklılık: dolu olan korur, arkasını gördüğün korumaz. Bu belgede siper diye sayılan her 110'luk kenar (iskele güvertesi, çatı parapeti, durak arka duvarı, saksı, bandstand duvarı) doludur; prototipte açık korkuluk zorunlu değildir |
| Karışım | Kat başına ~%50 alçak, ~%50 tam | Alçak siper **diz suyunda** hâlâ çalışır, **bel suyunda** (d = 100) ancak 10 cm çıkıntı kalır: su siperi de alır. Tam siper ıslakken de siperdir ama çevresinde yavaşsın |
| Üç durumda test | Her siper kuru / diz / bel su ile gym'de denenir | Hikâye §9.1 |
| Kumsal | Dalgakıranlar (K–G doğrultulu ahşap perde, 200 yüksek, 1200 uzun, X'te 1500 aralık): doğu–batı görüşünü keser, kuzey–güney koridorları (60 m) açık bırakır. Plaj kulübeleri 300 × 300 × 250 (A çevresi). Tekne enkazı 800 × 300 × 150 (C) | Koridorlar Purser bandında; iskele güvertesinden kumsala atış mümkün, karşı açıları: iskele altı ayaklar ve dalgakıran gölgesi |
| Sahil yolu | Durak sığınakları 600 × 200 (110 arka duvar), büfeler 300 × 300 × 300, saksılar 200 × 100 × 110; doğu–batı 120 m'lik hat en çok 2500'de bir kırılır | Uzun hat bilinçli kırılır |
| Ganimet/mermi/ilk yardım | En yakın sipere ≥ 500 açıkta | Alım risk ister |
| Güç pozisyonları | Çatı parapeti, iskele güvertesi | Her birinin en az iki karşı açısı var: parapet ← plaza basamakları + arcade merdiven çıkışı + H3 tepesi; güverte ← iskele altı + iki yandaki kumsal + yol |
| Teras | Otel kolonları 100 × 100 × 400 (600 aralık, Y 13400 hattı), masalar (alçak), bandstand duvarı | Slack High'da kuru adalar güvenli ama küçük ve açık: Scuttler'dan korur, Attendant'a hedef yapar |
| Kapı / koridor | Kapı 150 × 250; çatışma koridoru ≥ 400; arcade içi kolon aralığı 600 | Metrikler |
| Tavan | Arcade iç yükseklik 380: su baş hizasındayken (565) üstte 215 hava | Hikâye §9.1 |

### 7.7 Blockout sırası (environment-artist + developer)

1. **Gym:** metrik sayfası (kapı, siper, basamak, rampa açıları, zıplama mesafesi) + derinliği 0→200 değişen su havuzu (ıslaklık eşikleri burada doğrulanır).
2. Üç teras + altı kat geçişi + vinç platformu + su düzlemi. Sipersiz, düşmansız yürü: §7.4 süreleri ±%15 içinde mi?
3. Siperler, su çizgisi bantları, doğuş noktaları.
4. Düşmanlı tek döngü; sonra §9'daki ayarlar.

---

## 8. Görev akışı: tek bir tam döngü

### 8.1 Dakika dakika beklenen deneyim

| Zaman | Su | Oyuncu ne yapıyor | Düşman | Duygu / tempo |
|---|---|---|---|---|
| 0:00–0:20 | 830 → 690 | Vinç platformunda başlar; terastaki su ayak bileğinde, hızla çekiliyor. Vinç direğini ve su çizgisi bantlarını görür | 2 Scuttler terasta (su çekilince ~2 sn'de Dry olurlar: yavaş, kolay) → **ilk öldürme < 60 sn** | Merak; silahı dene |
| 0:20–1:00 | 690 → 410 | Teras ve çatılar: ilk Attendant çifti. Parapetten aşağıda suyun yoldan çekildiğini izler (gözlem noktası) | 2 Attendant (W-High) | Öğretim: siper, zayıf nokta, kuru zeminde güvenli |
| 1:00–2:00 | 410 → 150 | H1/H2/H3'ten Half'a iner; yol kuru, menfezlerde su | 2 Attendant (W-Half) + 3 Scuttler menfez birikintilerinde (20 sn'de **Parched** olurlar: kuruma öğretimi) | Yükseliş |
| 2:00–3:00 | 150 → 75 | Kumsala iner. A açık; B ve C hâlâ diz/bel suyunda. İlk karar: ıslak ve erken mi, kuru ve geç mi? İlk sandık(lar) yukarı | Her 40 sn'de 2 Scuttler denizden (kuru kumsalda yavaş); 2:30'da 2 Attendant kumsala iner | "Low tide pays": güç fantezisi |
| 3:00–4:00 | 75 → 0 | B kurur (2:56), C kurur (3:44). Ekip bölünür: taşıyıcı + eskort. İkinci tur için geri iniş | 3:30'da 2 Attendant iskele güvertesinde (kumsala hâkim) | Zirve 1; açgözlülük başlar |
| 4:00–5:00 | 0 | Uyarı: "She's about to turn." Son sandık kararı. 4:40'tan sonra yeni düşman gelmez | 4:20 son Scuttler dalgası; sonra sessizlik | Çan öncesi nefes; gerilim |
| **5:00** | 0 ↑ | **Çan.** Su çizgisi kumsalda 1 m/sn ilerler | 6 Scuttler su çizgisiyle birlikte (ıslak, hızlı); kumsaldaki Attendant'lar sırtını dönüp kaçar | "Vay" anı |
| 5:00–6:30 | 0 → 300 | Yüklü tırmanış: L2 dibi 5:30'da diz, 5:45'te bel → orta rota kapanır; L1/L3'e yönelme. Geri geri ateş | Her 20 sn'de 3 Scuttler; 5:10'da 2 Attendant sahil yolunda çıkışları tutar | Zirve 2: **arada kalma** |
| 6:30–7:00 | 300 (sabit) | Half Tide duraklaması. Sahil yolunda toparlanma, ilk yardım, mermi. Kumsalda bel suyunda kalmış sandık için son dalış kararı | Yeni doğuş yok | Nefes + son açgözlülük |
| 7:00–8:45 | 300 → 700 | Yol 7:26'da ıslanır, 7:39 diz, 7:53 bel. H1 (açık, hızlı), H2 (kapalı, Gaffer), H3 (uzun, Purser) arasında seçim. Attendant'ları suya itme fırsatı | Her 20 sn'de 3 Scuttler (menfezlerden, 7:13+); 7:05'te 3 Attendant merdiven başlarında ve parapette | Zirve 3 (en yüksek): yokuş yukarı savaş |
| 8:45–9:00 | 700 (sabit) | Çatılarda/terasta toparlanma | — | Kısa nefes |
| 9:00–10:00 | 700 → 830 | Sandıklar yük ağına. Kuru adalar ve vinç platformu tutulur. 9:46'da teras ıslanır | Her 20 sn'de 2 Scuttler; 9:00'da 2 Attendant vinç çevresinde | Yükseliş |
| 10:00–10:30 | 830 | Savunma: teras 30 cm su, **Scuttler'lar artık kurumuyor** | Her 20 sn'de 3 Scuttler (oluklardan) | Final |
| 10:30–12:00 | 830 | "Line's down." Kancalan (3 sn). Kota tamam değilse son sandık için risk | Aynı tempo; 12:00'de kesilir | Final → rahatlama |

### 8.2 Dalga betiği (solo sayıları; ×M(N), tavanlar §5.3)

| Zaman | Wardens | Drift |
|---|---|---|
| 0:05 | — | 2 (terasta, önceden yerleşik) |
| 0:20 | 2 (W-High) | — |
| 1:00 (ilk oyuncu Half'a basınca, en geç 1:15) | 2 (W-Half) | 3 (Half'ta, önceden yerleşik) |
| 2:20, 3:00, 3:40, 4:20 | — | 2'şer (D-Low deniz kenarı) = 8 |
| 2:30 | 2 (kumsala iner) | — |
| 3:30 | 2 (iskele güvertesi) | — |
| 5:00 | — | 6 (su çizgisi) |
| 5:10 | 2 (sahil yolu) | — |
| 5:20, 5:40, 6:00, 6:20 | — | 3'er = 12 |
| 6:30–7:00 | — | — (nefes) |
| 7:05 | 3 (H1 tepesi, parapet, H3 tepesi) | — |
| 7:15, 7:35, 7:55, 8:15, 8:35 | — | 3'er (D-Half) = 15 |
| 8:50, 9:10, 9:30, 9:50 | — | 2'şer (D-Half/D-High) = 8 |
| 9:00 | 2 (vinç çevresi) | — |
| 10:00–11:40, 20 sn'de bir (6 dalga) | — | 3'er (D-High) = 18 |
| **Solo toplam (tavan kesmezse)** | **15** | **72** |

### 8.3 Flood Drill (hata ayıklama başlangıcı — zorunlu)

Çekirdek soruyu 12 dakikada bir değil **7,5 dakikada bir** sınamak için: döngü **4:30**'da başlar; oyuncular C ganimet noktasında, dolap açık, kumsalda 2 Attendant + 4 Scuttler hazır; kota 1 sandık/2 oyuncu (en az 1). Ayar turlarının çoğu bu modda yapılır; tam döngü yalnız tempo ve açgözlülük ölçütleri (3, 8) için oynanır.

---

## 9. Ayar tablosu: playtest'te ilk oynanacak 10 değer

Sıra önceliktir. Her turda **en çok iki değer** değiştirilir; değişiklik ve gözlem günlüğe yazılır.

| # | Değer | Başlangıç | Aralık | Hangi belirtide hangi yöne |
|---|---|---|---|---|
| 1 | F1 yükselme hızı (kumsal) | 3,33 cm/sn (90 sn) | 2,5–6,0 | "Su tehdit değil / fark etmedim" → artır. Çan anında Low'daki oyuncuların çoğu boğuluyorsa → azalt |
| 2 | F3 yükselme hızı (Half kat) | 3,81 cm/sn (105 sn) | 3,0–6,0 | Oyuncular Half'ta rahatça kamp kuruyorsa → artır; H geçişlerinde yığılıp ölüyorlarsa → azalt |
| 3 | Half Tide duraklaması (F2) | 30 sn | 0–60 | Nefes yoksa/bunalma → uzat; tempo düşüyorsa → kısalt. 0 = düz yükseliş karşılaştırması (basamaklı eğri varsayımının sınaması) |
| 4 | Diz hızı çarpanı / bel hızı çarpanı | 0,75 / 0,5 | 0,6–0,9 / 0,4–0,7 | Islaklık hissedilmiyorsa → düşür; "suya değdim, öldüm" şikâyeti → yükselt |
| 5 | Yüklü yürüme hızı | 400 | 350–450 | Taşıyıcı sıkılıyor/eskortu beklemiyorsa → artır; taşımak bedelsizse → azalt. Kuru Scuttler hızıyla (360) arasındaki fark korunur |
| 6 | Scuttler Wet hız çarpanı | ×1,4 (504) | 1,2–1,6 | Islak/kuru farkı fark edilmiyorsa → artır; ıslak Scuttler kaçılmazsa → azalt |
| 7 | Scuttler canlı tavanı ve dalga büyüklüğü (solo taban; M eğimi 0,6) | 6 · dalga 2–6 · 0,6 | tavan 4–10 · eğim 0,4–0,8 | Solo ezilirse taban ↓; 4 kişide boşluk varsa eğim ↑ |
| 8 | Attendant yaylım hasarı ve ateş token'ı | 3 × 10 · 2/3/4/4 | 8–14 · ±1 | Yukarısı duvar gibiyse ↓; Attendant yok sayılıyorsa ↑ |
| 9 | High Water yüksekliği | 830 (terasta 30) | 790–860 | 790 = teras kuru (final kolay, Scuttler kurur); 860 = oyuncu dizde (final sert). Slack High puanına göre |
| 10 | Kota ve dolap açma süresi | N + 1 · 15 sn | N…N+2 · 8–25 | Flood başlamadan iş bitiyorsa kota ↑ ya da süre ↑; kimse kotaya yetişemiyorsa ↓ |

Bu ondan sonra sırada bekleyenler (ilk tur verisi gerektirirse): Purser hasarı (50), Gaffer tam hasar menzili (7 m), Parched süresi (20 sn), yenilenme tavanı (50), Slack Low uzunluğu (SL2 60 sn).

---

## 10. Teslimler: kim ne alıyor

### gameplay-developer

| İş | Bölüm |
|---|---|
| Karakter hareketi ve taşıma durumu | §2 |
| Islaklık durumu (oyuncu + AI ortak) ve etkileri | §3.3 |
| `DA_Purser`, `DA_Gaffer` veri varlıkları; Kedge; eksik alanlar | §4.1–4.4 |
| Scuttler ve Attendant (Behavior Tree, token yöneticisi, su farkındalığı servisi) | §5 |
| Can, düşme/kaldırma, spare line, sandık/dolap/vinç etkileşimleri | §6 |
| Dalga betiği ve doğuş noktası koşulları | §7.5, §8.2 |

### systems-developer

| İş | Bölüm |
|---|---|
| Gelgit çizelgesi (veri eğrisi → sunucu zamanından su Z), 4 oyunculu senkron | §3.2, ölçüt 6 |
| Listen server oturumu, 1–4 oyuncu, M(N) ölçekleme | §5.3 |
| Telemetri CSV | §1.5 |
| Flood Drill başlangıcı ve çizelge/ayar değerlerinin çalışma anında değiştirilmesi (konsol değişkeni ya da veri tablosu) | §8.3, §9 |

### environment-artist

| İş | Bölüm |
|---|---|
| Gym (metrik + su havuzu) | §7.7 |
| Promenade gri kutusu: ölçü tablosuna göre, yalnız ilkel geometri + ızgara materyali | §7.2–7.3, §7.6 |
| Su çizgisi bantları (Z 300 sarı, Z 830 kırmızı), geçici su düzlemi materyali (yarı saydam, düz renk; derinlikle koyulaşma yeter) | §3.1 |
| Asgari HUD: gelgit saati, sandık sayacı | §1.2 |

### weapon-character-artist

| İş | Bölüm |
|---|---|
| Ölçüye uygun **vekil** modeller (ilkel geometri, tek renk): Scuttler (alçak-geniş, sırtta parlak kese), Attendant (dik-ince, başta lamba: sakin beyaz / kırmızı / servis = 1 Hz yavaş göz kırpma, üç durum) | §5 — siluet 1 sn'de ayırt edilmeli |
| Purser ve Gaffer birinci şahıs vekil modelleri + namlu soketi; Strongbox (60 × 40 × 40), dolap, vinç direği | §4, §6.4 |
| Telgraf göstergeleri: lamba rengi, Scuttler kıskaç kaldırma/çökme pozları (iki poz yeter) | §5 |
| Geçici ses seti (kaynak kütüphaneden): çan, tannoy cızırtısı, su uğultusu, klik-klak, tıkırtı korosu, tıslama, kepenk, hitmarker/zayıf nokta/öldürme onayı | §3.4, §5 |

---

## 11. Bu belgenin hikâye belgesinden ayrıldığı yerler

| Konu | Hikâye belgesi | Prototip | Neden |
|---|---|---|---|
| Döngü süresi | ~19 dk (4/6/7/2) | 12 dk (2/3/5/2) | Tek harita, altı sandık; flood korunup gerisi kısaldı (§3.2) |
| Slack Low | Su sabit | İlk 2 dk su hâlâ yavaşça çekiliyor | En alttaki ganimet en geç açılsın (§3.2) |
| Alt kattan vince 60–90 sn | Koşu süresi | Çatışmalı süre olarak yorumlandı; düşmansız yüklü 25–35 sn | §7.4 |
| Attendant kısa devresi | Çevreye elektrik | Yalnız devre dışı kalma | Elektrik-su kapsam dışı |
| Scuttler / Attendant ayrıntıları | Arketip tanımı | Sayılar eklendi; kurallar aynı | — |

---

## 12. Revizyon 2 (25 Kasım 2026) — artist incelemesi sonrası

Kaynak: [weapon-character-specs.md](../art/weapon-character-specs.md) §10 ve [style-guide.md](../art/style-guide.md) §13. Hepsi başlangıç değeridir.

| # | Madde | Eski | Yeni | Gerekçe |
|---|---|---|---|---|
| 1 | Scuttler kapsülü (§5.1) | yarıçap 45, yükseklik 70 | yarıçap 35 / yarı yükseklik 35 (toplam 70) | Eski değer UE'de geçersiz. Tepe 70: 110'luk siper arkasında görünmezlik korunur; dar geçitlerde sürü daha az sıkışır |
| 2 | Kedge tutuşu (§4.3, §2.2, §4.4) | Belirsiz; 0,15 isabet ile 0,40 silah değiştirme çelişiyordu | Ayrı tuş, silah değiştirmeden sol el. Savurma reload'u **iptal eder** (ilerleme sıfırlanır), 0,60 boyunca ateş/ADS kilitli | Artist yorumu onaylandı. Sol el aynı anda iki iş yapamaz; bedel reload'dur |
| 3 | Attendant lamba isabet küresi (§5.2) | yarıçap 18, Z ≈ 175 (kapsülü 3 cm aşıyor) | yarıçap 18, merkez Z 172 (Z 154–190) | Hedef boyutu ve %50 isabet varsayımı korunur; görsel lamba Z 175'te kalır |
| 3b | Alçak siperde lamba (§5.2) | Tanımsız | Görünür kalır (merkez Z ≥ 135); telgraf her zaman lamba açıkken başlar | Telgraf göstergesi gizlenemez; gövde saklı, zayıf nokta açık |
| 4 | Servis penceresinde lamba (§5.2, §10) | "Lamba söner" | Kararmaz; sakin renkte 1 Hz göz kırpar, zayıf nokta etkin | En çok vurulması gereken anda hedef kaybolmasın; kehribar Etkileşim sarısıyla karışır |
| 5 | Islaklık adı (§3.3) | `Head` | `Overhead` (kod: `ENFWetness`) | Tek ad; `Head` yalnız isabet bölgesi |
| 5b | Waist'te silah kuralı (§3.3, §4.4) | Spec ile kod yorumu çelişiyordu | Kilit derinliğe bağlı: d < 135 Purser hip-fire, d ≥ 135 (`ChestDepth`) ateşli silah yok | Purser'ın "bel suyunda çalışan tek silah" işi korunur |
| 6 | Shove toparlanma (§5.2) | Yok | 0,50 sn (toplam 0,90) | Artist varsayımı onaylandı; karşılık penceresi |
| 7 | Çömelmiş kapsül yarı yüksekliği (§2.1) | 60 (boy 120) | 55 (boy 110); göz 104 aynı | Kapsül üstü alçak siperle aynı kot; siper 110 kalır |
| 8 | Parched görseli (§5.1) | "Renk solar" | Değer sabit; doygunluk kaybı + mat + tuz çizgisi + sönük kese | Artist yorumu onaylandı; Drift koyu bandı bozulmaz |
| 9 | Scuttler bacakları (§5.1) | Tanımsız | İsabet almaz, çarpışmaz | Sürü maliyeti; yedek: gövde kutusunu genişlet |
| 9b | Sandık taşırken kadraj (§2.3) | Tanımsız | Strongbox sol alt köşede görünür (≤ ~%15), sol kol sandıkta | Yük HUD'sız okunur; tek el kuralının görsel nedeni |
| 10 | Kat ve su yükseklikleri (§3.1, §7.1) | — | **Teyit, değişiklik yok:** kat zeminleri Low 0→200 (eğimli) / Half 400 / High 800; su çizgileri Low Water 0 / Half Tide 300 / High Water 830 | Kit kat yüksekliği 400'dür; su çizgisi bantları kat zemininde **değil**, 300 ve 830'dadır |
| 10b | Tam siper eşiği (§7.6) | 220+ | 200+ | Stil rehberiyle tek eşik |
| 10c | Siper sınıfları (§7.6, §7.3) | Tanımsız; iskelede "korkuluk (alçak siper)" | Dolu parapet = alçak siper; açık korkuluk = siper değil, mermi geçirir; kiosk ve oyun makinesi = tam siper; iskele kenarı dolu parapet | Environment-artist sorusu |

**Bağlı tabloların denetimi:** bu revizyonda hasar, can, RPM, hareket hızı, su eğrisi ve harita koordinatı **değişmedi**. §4.5 TTK tablosu (Kedge 40 hasar / 0,60 aralık aynı), §3.2 hesaplanmış anlar, §6.1 ölüm süresi ve §7.4 rota süreleri yeniden gözden geçirildi; hiçbir satır değişmiyor. Savurma sırasındaki ateş kilidi sürekli hasar rakamlarını (Purser 125, Kedge 67 can/sn) değiştirmez; yalnız ikisinin toplanamayacağını kesinleştirir.
