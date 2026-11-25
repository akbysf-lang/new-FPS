# EBBSHIFT — Silah ve Karakter Spesifikasyonu (dikey dilim)

**Hazırlayan:** weapon-character-artist · **Tarih:** 25 Kasım 2026 · **Durum:** Taslak v1 — üretime hazır tarif; hiçbir varlık henüz üretilmedi
**Girdi:** [prototype-spec.md](../design/prototype-spec.md) §2–§5, §10 · [game-story.md](../design/game-story.md) §4, §5.1, §9.2–9.3 · [style-guide.md](style-guide.md) §2.4–2.5, §3, §8, §13 · [prototype-setup.md](../tech/prototype-setup.md) §6 · **Sözleşme:** `fps-art-pipeline`

> Belge dili Türkçe; varlık, kemik, soket ve notify adları İngilizce.
> **Bu belge kâğıt üzerindedir.** Blender MCP bu oturumda bağlı değildi (127.0.0.1:9876 bağlantıyı reddetti) ve konteynerde Blender/bpy yok. Blender'da hiçbir şey modellenmedi, `look` ile hiçbir şey kontrol edilmedi, FBX üretilmedi, UE'ye hiçbir şey alınmadı. Üçgen sayıları, doku boyutları ve süreler **bütçe/hedef**tir, ölçüm değildir.
> **Üretken yapay zekâ:** bu belgedeki hiçbir varlık için `generate_3d` ya da benzeri kullanılmadı. Kullanılırsa §8.4'teki kural geçerlidir (varlık listesinde işaret + GM'e bildirim, Steam beyanı için).
> Oynanış sayıları (hasar, süre, kapsül) designer'ındır; burada yalnız **tekrarlanır**. Değişirlerse animasyon süreleri §9'daki kurala göre yeniden ölçeklenir. Spec ile bulduğum tutarsızlıklar §10'da.

---

## 0. Ortak teknik sözleşme (bu belgedeki her varlık için)

| Konu | Kural |
|---|---|
| Birim / eksen | 1 BU = 1 cm; FBX `-Y Forward, Z Up`; UE'de +X ileri. Silah mesh'inde **namlu +X'e bakar** (developer'ın `Muzzle` soket beklentisi) |
| Pivot | Silah: kabzada, tetik elinin avuç merkezi (kolun `GripPoint` soketine sıfır ofsetle otursun; ince ayar `WeaponAttachOffset`). Düşman: ayak tabanı merkezi, Z = 0 |
| Adlandırma | `SK_` iskeletli, `SM_` statik, `SKEL_` iskelet, `PHYS_` fizik varlığı, `ABP_` AnimBP, `A_` animasyon, `AM_` montage, `T_..._D/_N/_ORM/_E`, `M_`/`MI_`, `VFX_` (Niagara), `CR_` Control Rig |
| Klasör | `Content/NewFPS/Weapons/<Ad>/`, `Content/NewFPS/Characters/Arms/`, `Content/NewFPS/Enemies/<Fraksiyon>/<Arketip>/`, `Content/NewFPS/VFX/` (öneri; developer'ın `Weapons/` Asset Manager taramasıyla çakışmaz: veri varlıkları kök klasörde kalır) |
| Materyal | İki master: `M_Weapon_Master` (FP; ORM + renk bölge maskesi + ıslaklık) ve `M_Enemy_Master` (ORM + emissive maske + durum parametreleri). Varlık başına yalnız `MI_`. Stil "boyalı maket": düz albedo, geniş roughness farkları, küçük ölçekli çizik gürültüsü yok |
| Kare hızı | Animasyonlar 60 fps'te yazılır (0,15 sn'lik isabet penceresi 30 fps'te 4,5 karedir; yetmez) |
| Stilde abartı | Stil rehberi §3: ikincil öğeler 1,3–1,5× kalın. Silahlarda tetik korkuluğu, horoz, arpacık, pompa eli ve kanca ucu bu kuralla **kalınlaştırılır**; toplam boy abartılmaz |
| Değer kuralı | FP silah/kol ekranın altında durur ve düşmanla karışmamalı: silah gövdesi L* 30–55 (ortamın ıslak bandı), eldiven L* 55–70. Silah ve kolda **Tehlike kırmızısı, Etkileşim sarısı, Ganimet moru, Drift turkuazı yok** |

**İlk varlıkta hat doğrulaması (zorunlu, seri üretimden önce):** `SK_Purser` vekil modeli → FBX → UE → `DA_Purser` → `Muzzle` soketinden iz çıkıyor mu, ölçek mankeniyle boy doğru mu, +X yönü doğru mu. Bu adım geçmeden başka varlığa başlanmaz.

---

## 1. Silahlar

Estetik: 1970'ler liman ve tersane ekipmanı; "yarısı silah, yarısı iş aleti". Üçünün ortak kimliği: **şirket malı** — Fathom & Lyle zimmet numarası basılmış pirinç plaka, mat gri-yeşil tersane boyası, bakalit/kauçuk tutamak, halat veya zincir askı. Gerçek marka, model, damga ve seri numarası formatı kopyalanmaz; mekanizma türleri (kırma top, pompa) geneldir, form özgündür.

### 1.1 Purser — toplu tabanca

**Siluet (1 saniyede):** kısa, kalın, **üstten kırılan** toplu; namlu üstünde boydan boya kalın düz şerit, iri halka horoz, kabza dibinde kordon halkası ve aşağı sarkan örgü kordon. Yandan bakınca "L" değil, kalın bir "çekiç" gibi okunur.

**Tasarım tarifi:** Geminin veznedarına (purser) verilen kasa tabancası fikri. Kısa (10 cm) ağır namlu, altıgen değil **yuvarlatılmış kutu** kesit (stil rehberinin "yumuşak kutu" dili). Top 6 yuvalı, yivsiz düz yüzeyli, üstünde tek kalın kanal. Kırma mekanizması: gövde üstünde iri bir başparmak mandalı; kırılınca yıldız çıkarıcı altı kovanı birden atar (reload okunaklı ve hızlı: 2,2 sn'ye sığar). Dolum **altılı klips** (tek parça yay halka) ile. Kabza: çikolata kahve bakalit, iri baklava değil yatay oluklu. Kabza dibinde pirinç zimmet plakası + kordon halkası. Gez: gövde üstünde U çentik; arpacık: kalın dikdörtgen, üstü açık krem boyalı (ADS'de koyu arka plan önünde okunur; oynanış rengi değildir).

| Ölçü | cm |
|---|---|
| Toplam boy (namlu ucu → horoz arkası) | 24 |
| Yükseklik (kabza dibi → gez üstü) | 15 |
| Kalınlık (top) / (gövde) | 4,4 / 2,2 |
| Namlu | 10 (dış çap 2,2 eşdeğeri kutu kesit) |
| Top | çap 4,4 · boy 4,0 |
| Kabza | boy 9,5 · kalınlık 3,4 |
| Gez–arpacık arası | 15,5 |
| Kordon | 18 (3 kemik, görsel) |

**Hareketli parçalar ve kemikler** (`SKEL_Purser`, 11 kemik):

| Kemik | Üst kemik | Hareket | Not |
|---|---|---|---|
| `root` | — | — | Pivot: kabza avuç merkezi |
| `frame` | `root` | — | Kabza + alt gövde |
| `barrel_assy` | `frame` | Menteşe pimi çevresinde 0° → −38° (kırma) | Namlu + top taşıyıcı |
| `cylinder` | `barrel_assy` | Kendi ekseninde atış başına 60° | |
| `extractor` | `cylinder` | Eksen boyunca 0 → 1,6 cm | Yıldız çıkarıcı |
| `latch` | `frame` | 0° → 20° | Başparmak mandalı |
| `hammer` | `frame` | 0° → −42° | |
| `trigger` | `frame` | 0° → 18° | |
| `lanyard_01..03` | `frame` | Fizik/AnimDynamics | Yalnız görsel; kamera önüne geçmeyecek sınır |

Ayrı mesh'ler: `SM_Purser_Round` (dolu fişek), `SM_Purser_Casing` (boş kovan, Niagara mesh parçacığı), `SM_Purser_Clip` (altılı klips; reload'da sol ele `Grip_L`/el soketinden bağlanır).

**Soketler** (`SK_Purser` üzerinde; hepsi +X ileri):

| Soket | Üst kemik | Konum | Durum |
|---|---|---|---|
| `Muzzle` | `barrel_assy` | Namlu ağzı merkezi, +X namlu ekseni | **Kodda var** (`MuzzleSocketName`, varsayılan `Muzzle`) |
| `ShellEject` | `cylinder` | Top arkası, +X = kovanların çıkış yönü (geriye-aşağı) | Kodda yok — öneri (`fps-art-pipeline` adı) |
| `Sight` | `frame` | Gez çentiğinin tabanı, +X arpacığa bakar | Kodda yok — ADS hizası için gerekli (§10) |
| `Grip_L` | `frame` | Sol elin destek noktası (kabza altı) | Kodda yok — sol el IK hedefi |

`GripPoint` silahta **değil**, kol iskelesindedir (§2.3).

**Bütçe:**

| Kalem | Hedef |
|---|---|
| Üçgen LOD0 (FP) | ≤ 18.000 (oyuncuya bakan arka/üst/sol yüz yoğun; sağ yüz ve namlu altı sade) |
| Dünya sürümü `SM_Purser_World` | ≤ 2.500, tek parça, 512² |
| Doku | 1 set 2048²: `T_Purser_D/_N/_ORM` |
| Materyal | 1 slot, `MI_Purser` |
| Fişek/kovan/klips | toplam ≤ 600 üçgen, silah atlasını paylaşır |

**Materyal bölgeleri** (tek doku seti, bölge maskesiyle):

| Bölge | Albedo (yaklaşık) | Rough / Metal | Not |
|---|---|---|---|
| Gövde, namlu: fosfatlı çelik üstü tersane boyası | `#56605A` (L* ~40) | 0,55 / 0,8 (boya soyulan kenarda çıplak çelik) | Kenar aşınması geniş ve az; çizik gürültüsü yok |
| Top: parlatılmış çelik | `#8A8E8C` | 0,30 / 1,0 | Ekrandaki en parlak silah parçası; dönüşü okutur |
| Kabza: bakalit | `#5E4332` (palet: çikolata kahve) | 0,45 / 0 | |
| Plaka, kordon halkası, klips: pirinç | `#A8873F` (palet) | 0,35 / 1,0 | Plakada kurgusal zimmet numarası: `F&L 0417` biçimi |
| Kordon: örgü pamuk | `#CFC5AA` | 0,9 / 0 | |
| Arpacık üstü | `#F4EEDD` boya | 0,6 / 0 | |

**Animasyon listesi.** Süreler `DA_Purser` başlangıç değerleriyle tutarlıdır: atış aralığı **0,40** (150 RPM), `ReloadTime` **2,2**, `AdsTime` 0,18, silah değiştirme 0,40, koşudan ateşe 0,20. Her animasyon iki parçadır: kol (`A_Arms_Purser_*`) ve silah (`A_Purser_*`), aynı uzunlukta.

| Animasyon | Süre (sn) | Tür | Not |
|---|---|---|---|
| `Idle` | 3,0 döngü | Taban | Çok hafif nefes; kordon AnimDynamics |
| `Fire` | 0,33 | Additive | 0,40'lık aralığa sığar; 0,07 boşluk. Tepme pozu 0,00–0,06, dönüş 0,33'te biter. Kamera tepmesi koddadır; mesh tepmesi küçük tutulur (çift tepme olmasın) |
| `Fire_Last` | 0,33 | Additive | Aynı; horoz inik kalır |
| `DryFire` | 0,20 | Additive | Yalnız tetik + horoz |
| `Reload` | **2,20** | Tam gövde (kol + silah) | Aşağıdaki zaman çizelgesi. Bel suyunda `ReloadTime × 1,25 = 2,75`: aynı animasyon **0,8 oynatma hızıyla** |
| `Equip` / `Unequip` | 0,25 / 0,15 | Tam | Toplam 0,40 = silah değiştirme |
| `Sprint` | 0,6 döngü | Taban | Silah göğse çapraz; `SprintOut` 0,20 |
| `ADS_Pose` | tek kare | Poz | Geçişi kod `GetAdsAlpha` ile yapar (0,18); gez-arpacık kamera merkezinde |
| `Carry_Idle`, `Carry_Fire` | 3,0 döngü / 0,33 | Taban / additive | Sandık taşırken: tek el, bel hizası, eğik tutuş; sol kol kadraj dışı. ADS ve reload yok (spec §2.3) |
| `Wade_Idle` | 3,0 döngü | Taban | Bel suyunda: silah göğüs hizasına kalkık, yalnız hip-fire |
| `Inspect` | ~4,0 | Tam | P2; dikey dilimde zorunlu değil |

`Reload` zaman çizelgesi (2,20 sn; notify adları §9'daki ortak sözlükte):

| Zaman | Olay | Notify |
|---|---|---|
| 0,00 | Sol el kabzadan ayrılır, başparmak mandala | — |
| 0,22 | Mandal açılır | `AN_Reload_Open` (ses) |
| 0,30–0,50 | Namlu aşağı kırılır | — |
| 0,52 | Çıkarıcı altı kovanı atar | `AN_Reload_Eject` (VFX: 6 kovan, `ShellEject`; ses) |
| 0,60–1,15 | Sol el kemere iner, klipsle döner | `AN_Reload_ClipShow` 0,95 (klips mesh'i görünür) |
| 1,25 | Klips topa oturur | `AN_Reload_Insert` (ses) |
| 1,45 | Sol el çekilir, klips mesh'i gizlenir, top dolu | `AN_Reload_ClipHide` |
| 1,60–1,75 | Bilek hareketiyle namlu kapanır | `AN_Reload_Close` 1,72 (ses) |
| 1,75–2,20 | Hazır poza dönüş; sol el destek noktasına | — |
| **2,20** | Kodun reload zamanlayıcısı biter, mermi verilir | `AN_Reload_Commit` (yalnız işaret; mermiyi **kod** verir) |

**VFX / ses ihtiyaç listesi:**

| Kalem | Ad | Not |
|---|---|---|
| Namlu alevi | `VFX_Purser_Muzzle` → `MuzzleFlashFX` | 2 kare (60 fps'te ~0,033 sn), en çok 12 cm; ADS'de arpacığı kapatmaz. Topla namlu arasından küçük yan alev (toplu imzası). Duman: tek küçük, 0,3 sn, yarı saydamlık az |
| İz | `VFX_Tracer_Bullet` → `TracerFX` | Niagara kullanıcı parametresi **`User.BeamEnd`** (Vector, dünya uzayı; kodda `TracerEndParameter = BeamEnd`). İnce, soluk sıcak beyaz; 0,05 sn |
| İsabet | `VFX_Impact_Default`, `_Water`, `_Warden`, `_Drift` → `ImpactFX` | Kodda tek `ImpactFX` alanı var; yüzeye göre seçim developer'dan istek (§10). Warden: krem emaye pulu + kıvılcım. Drift: koyu kabuk kırığı + turkuaz sıvı. Su: dikey sıçrama, köpük renginde |
| Zayıf nokta isabeti | `VFX_Impact_WeakPoint_Lamp`, `_Sac` | Lamba: cam kırığı + parlama. Kese: turkuaz patlama. Normal isabetten **belirgin biçimde büyük** |
| Kovan | `VFX_Purser_Casings` | 6 mesh parçacık, yalnız reload'da; 3 sn ömür, suya değince küçük sıçrama |
| Sesler | `FireSound`, `DryFireSound`, `ReloadSound` (tek dosya ya da notify başına 4 parça: open / eject / insert / close), tutuş/çekme | Atış: kısa, sert, orta-tiz; Gaffer'dan frekansla ayrılır |

### 1.2 Gaffer — pompalı tüfek

**Siluet:** kısa, kalın namlu + altında aynı boyda şarjör borusu; iri, **oluklu silindir** pompa eli (kauçuk; halat makarası gibi); dipçik yerine iskeletleşmiş çelik boru + kauçuk omuz yastığı; namlu ucunda halat kılavuzuna benzeyen kalın halka. Purser'dan iki kat uzun, belirgin biçimde "yatay boru" okunur.

**Tasarım tarifi:** Tersanede halat atma/işaret aleti gövdesinden bozma pompalı fikri. Gövde (receiver) dikdörtgen, köşeleri yuvarlatılmış preslenmiş sac kutu; sağ yanında iri atım penceresi; üstünde boydan boya alçak düz ray (nişan yerine: arkada sığ V oluk, önde iri pirinç boncuk arpacık). Pompa eli çift çubukla gövdeye bağlı, hareket mesafesi 9 cm. Altta dolum kapağı (kaşık). Tetik korkuluğu eldivenle kullanım için büyük (stil abartısı 1,4×). Gövdenin sol yanında (oyuncuya bakan yüz) pirinç zimmet plakası ve elle boyanmış krem şablon yazı (`GAFFER / No. 2 HOLD` gibi kurgusal). Askı: kısa zincir + karabina.

| Ölçü | cm |
|---|---|
| Toplam boy | 86 |
| Namlu | 46 · dış çap 2,6 |
| Şarjör borusu | 40 · dış çap 2,8 (6 fişek; spec `MagazineSize` 6) |
| Gövde | 22 × 4,2 (kalınlık) × 7,5 (yükseklik) |
| Pompa eli | boy 16 · çap 5,2 · hareket 9 |
| Dipçik (boru iskelet) | 24; çekiş boyu (tetik → yastık) 33 |
| Atım penceresi | 7,5 × 2,6 |
| Gez–arpacık | 58 |
| Fişek | 7,0 × çap 2,0 |

**Kemikler** (`SKEL_Gaffer`, 9 kemik):

| Kemik | Üst kemik | Hareket |
|---|---|---|
| `root` | — | Pivot: tabanca kabzası avuç merkezi |
| `receiver` | `root` | — |
| `pump` | `receiver` | X ekseninde 0 → −9 cm |
| `bolt` | `receiver` | X ekseninde 0 → −8 cm (pompa ile; atım penceresinden görünür) |
| `lifter` | `receiver` | 0° → 25° (dolum kaşığı) |
| `trigger` | `receiver` | 0° → 15° |
| `safety` | `receiver` | 0 → 0,4 cm (yalnız inspect) |
| `sling_01..02` | `receiver` | AnimDynamics, görsel |

Ayrı mesh'ler: `SM_Gaffer_Shell` (dolu fişek; mumlu karton gövde `#B5693A` turuncu-kahve + pirinç dip — palette var, Tehlike kırmızısı **değil**), `SM_Gaffer_ShellSpent`.

**Soketler:**

| Soket | Üst kemik | Konum | Durum |
|---|---|---|---|
| `Muzzle` | `receiver` | Namlu ağzı merkezi, +X | **Kodda var** |
| `ShellEject` | `receiver` | Atım penceresi merkezi, +X = sağa-yukarı-hafif geri | Öneri |
| `Sight` | `receiver` | Arka V oluk tabanı, +X arpacığa | Öneri (ADS) |
| `Grip_L` | `pump` | Pompa elinin alt orta noktası | Öneri (sol el IK; pompa ile birlikte hareket eder) |
| `ShellInsert` | `receiver` | Dolum kapağı ağzı | Öneri (reload'da fişeğin hizalanacağı nokta) |

**Bütçe:** LOD0 ≤ 28.000 üçgen · dünya sürümü ≤ 3.500 / 512² · doku 1 set 2048² (`T_Gaffer_D/_N/_ORM`; UV'de oyuncuya bakan sol/üst/arka yüz 1,5× texel) · 1 materyal slotu · fişekler ≤ 400 üçgen, atlası paylaşır.

**Materyal bölgeleri:**

| Bölge | Albedo | Rough / Metal |
|---|---|---|
| Gövde, namlu, boru: tersane gri-yeşil boyası | `#4F5A52` (L* ~37) | 0,6 / 0,7 |
| Sürgü, atım penceresi içi: çıplak çelik | `#8A8E8C` | 0,35 / 1,0 |
| Pompa eli, omuz yastığı, kabza: siyah kauçuk | `#2E2C2A` (L* ~18; küçük yüzey) | 0,8 / 0 |
| Arpacık, plaka, karabina: pirinç | `#A8873F` | 0,35 / 1,0 |
| Şablon yazı | `#CFC5AA` | 0,7 / 0 |

**Animasyon listesi.** `DA_Gaffer`: atış aralığı **0,857** (70 RPM), `ReloadTime` **2,8**, `AdsTime` 0,22.

| Animasyon | Süre (sn) | Not |
|---|---|---|
| `Idle` | 3,0 döngü | |
| `Fire` (pompa dahil) | **0,80** | Tam gövde değil: tepme additive + pompa katmanı. 0,857'lik aralığa sığar (0,057 boşluk) |
| `Fire_Last` | 0,45 | Pompa yok; sürgü kapalı kalır (boş olduğu, pompalanmamasından okunur) |
| `DryFire` | 0,20 | |
| `Reload` (prototip: tek parça) | **2,80** | Aşağıda. Bel suyunda Gaffer zaten kullanılamaz; ×1,25 varyantı gerekmez |
| `Reload_Start` / `Reload_Loop` / `Reload_End` (dikey dilim: fişek fişek) | 0,40 / 0,35 / 0,30 | 0,40 + 6 × 0,35 + 0,30 = **2,80**: boştan tam dolum bugünkü `ReloadTime` ile aynı kalır. Kısmi dolum: 0,70 + n × 0,35 |
| `Equip` / `Unequip` | 0,25 / 0,15 | |
| `Sprint`, `SprintOut` | 0,6 döngü / 0,20 | |
| `ADS_Pose` | tek kare | Geçiş kodda (0,22) |
| `ToPurser_Wade` | 0,40 | Bel suyuna girince otomatik Purser'a geçiş: Gaffer sırta, Purser çekilir |

`Fire` zaman çizelgesi (0,80):

| Zaman | Olay | Notify |
|---|---|---|
| 0,00–0,10 | Tepme pozu (omuza geri + namlu yukarı) | — |
| 0,10–0,26 | Dönüş başlar; sol el pompayı kavrar | — |
| 0,26–0,42 | Pompa geri | `AN_Pump_Back` 0,28 (ses) |
| 0,40 | Boş fişek atılır | `AN_ShellEject` (VFX, `ShellEject`) |
| 0,44–0,60 | Pompa ileri | `AN_Pump_Forward` 0,58 (ses) |
| 0,60–0,80 | Nişan pozuna yerleşme | — |

Kod recoil'i 0,15 sn bekleyip 10°/sn toparlar (spec §4.2); mesh'in namlu yukarı pozu bu toparlanmayla yarışmamalı: mesh tepmesi ≤ 3° ve 0,26'da biter.

`Reload` (prototip, 2,80) zaman çizelgesi:

| Zaman | Olay | Notify |
|---|---|---|
| 0,00–0,40 | Silah sola yatar, dolum kapağı yukarı | `AN_Reload_Open` 0,30 |
| 0,40–2,50 | 6 fişek, 0,35 arayla | `AN_Reload_Insert` × 6: 0,65 · 1,00 · 1,35 · 1,70 · 2,05 · 2,40 |
| 2,50–2,80 | Silah doğrulur (boşsa: tek pompa, 2,55–2,75) | `AN_Pump_Back` 2,58 · `AN_Pump_Forward` 2,72 · `AN_Reload_Commit` 2,80 |

Prototip ve dikey dilim animasyonu aynı kareleri kullanır; fark yalnız kesilip döngüye bölünmesidir. Yani prototip için yapılan iş atılmaz.

**VFX / ses:**

| Kalem | Ad | Not |
|---|---|---|
| Namlu alevi | `VFX_Gaffer_Muzzle` | 3 kare, geniş ve kısa (en çok 20 cm); duman Purser'dan fazla ama 0,4 sn'de dağılır, opaklık ≤ 0,3 (0–7 m'deki Scuttler dumanın arkasında kaybolmamalı) |
| Saçma izleri | `VFX_Tracer_Pellet` | 8 iz; her biri `User.BeamEnd`. Kodun saçma başına iz spawn edip etmeyeceği developer kararı (§10); etmezse tek sistem + 8 uç noktası dizisi gerekir |
| İsabet | Purser ile ortak set | 8 isabet aynı karede: parçacık sayısı saçma başına Purser'ın 1/3'ü |
| Boş fişek | `VFX_Gaffer_ShellEject` | Tek mesh parçacık, atış başına; 3 sn |
| Sendeletme geri bildirimi | `VFX_Stagger_Burst` | Attendant ≥ 80 hasarda sendeler: göğüs plakasından emaye pulu patlaması; oyuncuya "eşiği geçtin" der |
| Sesler | `FireSound` (bas, geniş), pompa geri/ileri (2 parça), fişek sürme, `DryFireSound`, çekme | Pompa sesi atış sesinden ayrı dosya: notify ile tetiklenir |

### 1.3 Kedge — kanca (yakın dövüş)

**Siluet:** kısa saplı, iki dişli küçük çapa: bir uzun kıvrık kanca + ters yönde kısa küt mahmuz. Ekranda "soru işareti" gibi okunur.

**Tasarım tarifi:** Tekne kancası ile küçük demirin (kedge) kırması; liman işçisinin elinin altındaki her işe yarayan alet. Dövme çelik baş, dişbudak sap, sap dibinde halat sargı tutamak ve bilek ilmeği. Başın sapla birleştiği yerde pirinç bilezik ve zimmet numarası. Kanca ucu stil gereği kalın (1,4×), sivri ama iğne değil.

| Ölçü | cm |
|---|---|
| Toplam boy | 62 |
| Sap | 46 · çap 3,2 |
| Baş genişliği (kanca ucu → mahmuz ucu) | 19 |
| Kanca açıklığı | 9 |
| Bilek ilmeği | 22 çevre |

Erişim 180 cm (spec §4.3) kamera noktasından ölçülür: kol + alet + öne uzanma ile görsel olarak ~120 cm'ye ulaşır; kalan fark oyun payıdır, animasyon öne adım hissi verir.

**Rig:** iskelet gerekmez; `SM_Kedge` statik mesh, kolun sol el soketine (`KedgePoint`, §2.3) bağlanır. Bilek ilmeği sapla tek parça, hareketsiz. Soket: `Tip` (kanca ucu; isabet VFX'i ve iz için), `Trail_Start` / `Trail_End` (savurma izi şeridi).

**Bütçe:** ≤ 4.000 üçgen · 1024² tek set (`T_Kedge_D/_N/_ORM`) · 1 materyal · dünya sürümü aynı mesh'in LOD1'i (≤ 1.200).

**Materyal bölgeleri:** dövme çelik `#4A4E4D` (rough 0,5, metal 1; kanca ucunda parlak aşınma `#9A9E9C`) · dişbudak sap `#9C8660` (rough 0,6) · halat sargı `#CFC5AA` · pirinç bilezik `#A8873F`.

**Tutuş varsayımı (açık karar, §10):** Kedge **silah değiştirmeden**, sol elle savrulur; sağ el ateşli silahı tutmaya devam eder. Gerekçe: spec "tuştan isabete 0,15" ve "Gaffer dolarken boşluğu doldurur" diyor; 0,40'lık silah değiştirmeyle bu ikisi de olmaz. Göğüs suyunda (yalnız Kedge) ateşli silah aşağı iner, Kedge sol elde hazır bekler.

**Animasyon listesi.** Vuruş aralığı **0,60**, tuştan isabete **0,15**.

| Animasyon | Süre (sn) | Not |
|---|---|---|
| `Kedge_Swing_A` / `_B` | **0,60** | Sol-alttan sağ-üste / sağ-üstten sol-alta; art arda vuruşta sırayla. İsabet karesi 0,15 |
| `Kedge_Swing_Hit` (additive) | 0,12 | İsabet onayında 2 karelik takılma (hit-stop hissi; oyun zamanı durmaz) |
| `Kedge_Ready_Idle` | 3,0 döngü | Göğüs suyu: Kedge önde, silah yok |
| `Kedge_Ready_In` / `_Out` | 0,25 / 0,20 | Göğüs eşiğine giriş/çıkış |
| Gaffer reload sırasında savurma | — | Karar gerekir: savurma reload'u keser mi (§10). Kesmezse sol el fişek sürerken savuramaz; animasyon bunu çözemez |

`Kedge_Swing` zaman çizelgesi (0,60):

| Zaman | Olay | Notify |
|---|---|---|
| 0,00–0,10 | Hazırlık yok denecek kadar kısa: sol el kadraja girer (tepki hissi için ilk karede hareket) | `AN_Melee_Whoosh` 0,05 (ses) |
| 0,10–0,20 | Savurma; iz şeridi açık | `ANS_Melee_Trail` 0,08–0,22 (notify state) |
| **0,15** | İsabet karesi | `AN_Melee_Hit` (kodun isabet anıyla aynı; kod zamanlayıcıyla çalışıyorsa yalnız işaret) |
| 0,20–0,60 | Toparlanma; 0,45'ten sonra kol kadrajdan çıkar | — |

**VFX / ses:** `VFX_Kedge_Trail` (ince, yarı saydam şerit; 0,15 sn) · isabet: `VFX_Impact_Melee_Warden` (emaye pulu + çınlama), `_Drift` (kabuk çatırtısı) · Parched Scuttler'ı tek vuruşta öldürme: **ayrı, büyük** kuru parçalanma efekti (`VFX_Scuttler_Death_Parched`; ödül anı) · sesler: savurma, metal-emaye isabet, kabuk isabet, ıska.

---

## 2. First-person kollar

### 2.1 Tasarım

İş eldiveni + su geçirmez kol. Dirsekten yukarısı çoğu pozda kadraj dışıdır; detay bilek–parmak ve ön kolun üst/iç yüzündedir.

| Parça | Tarif |
|---|---|
| Eldiven | Kalın PVC kaplı balıkçı/liman eldiveni; avuç içi pütürlü, parmak uçları yuvarlak, bilekte kalın kıvrık manşet. Parmaklar gerçekten ~1,2× kalın (stil + eldivenle iri tetik korkuluğu tutarlı) |
| Kol | Yağlı muşamba (oilskin) ceket kolu; bilekte çıtçıtlı sıkma bandı, ön kolda tek kalın dikiş hattı. Kumaş kırışığı büyük ve az (normal map; küçük kırışık gürültüsü yok) |
| Sol bilek | Fathom & Lyle zimmetli dalış saati kayışı yeri **boş bırakılır**: ileride HUD'un gelgit saatini bileğe taşıma seçeneği için (karar değil, rezerv) |
| Islaklık | `M_Weapon_Master` içindeki ıslaklık parametresi kolda da çalışır: bel suyundan çıkınca 4 sn roughness düşük + damla normal'i (developer'dan `Wetness` parametresi, §10) |

Ölçü: Epic Manny kol oranları (omuz–bilek ~56 cm, el ~19 cm) korunur; eldiven el çevresini ~1 cm kalınlaştırır. Viewmodel FOV 70 (spec §2.1): kol bu FOV'da modellenip kontrol edilir, dünya FOV'unda değil.

### 2.2 Dört varyasyon yaklaşımı

Hikâye §5.1'deki dört karakter (Marnie, Dez, Ines, Sprat) **aynı mesh, aynı iskelet, aynı animasyon**. Fark üç katmanda:

| Katman | Yöntem | Maliyet |
|---|---|---|
| Renk | `MI_Arms_<Ad>`: eldiven ve kol için iki renk parametresi + bölge maskesi (`T_Arms_Mask`: R = eldiven, G = kol, B = manşet/bant, A = yama) | 4 instance, sıfır ek doku |
| Yama/işaret | Aynı maskede A kanalı + 4'lü küçük dekal atlası (512²): onarım yaması, bant, tebeşir işareti, kol numarası | 1 küçük doku |
| Küçük parça | Manşet bölgesinde değişen tek statik mesh (≤ 500 üçgen), `hand_l`/`lowerarm_l` soketine bağlı: Marnie örgü yün bilek, Dez dalgıç manşet halkası, Ines lastik yalıtkan eldiven manşeti, Sprat renkli çıtçıt + bileklik | 4 küçük mesh, ortak atlas |

Önerilen renkler (hepsi doygunluğu düşük; oynanış renkleriyle çakışmaz, L* 45–70):

| Karakter | Kol | Eldiven |
|---|---|---|
| Marnie Quill | Soluk hardal muşamba `#B79A52` | Koyu lacivert-gri `#4A5560` |
| Dez Abara | Donanma grisi `#6F7B80` | Kahve deri takviyeli `#7A5A3C` |
| Ines Carvalho | Kirli krem iş önlüğü kolu `#C7BFA6` | Tuğla kauçuk `#9A5B45` (Tehlike kırmızısından uzak: doygunluk < %45) |
| Sprat Lowe | Soluk petrol mavisi `#6F97A3` | Açık bej PVC `#C9B98F` |

Cilt görünmez (eldiven + kol tam kapalı): ten rengi varyasyonu, tırnak, kıl üretimi yok. Co-op'ta başkasının gördüğü tam gövde bu belgenin kapsamı dışındadır (dikey dilimde yer tutucu; §5).

### 2.3 Rig notu (Epic iskeletiyle uyum)

- `SK_Arms_FP`, **UE5 Manny/Quinn iskelet hiyerarşisinin tamamını** (`root → pelvis → spine_01..05 → clavicle_l/r → upperarm → lowerarm → hand → parmaklar`, `ik_hand_root / ik_hand_gun / ik_hand_l / ik_hand_r` dahil) aynı ad ve aynı referans pozla taşır. Mesh yalnız üst kol ortasından aşağıyı içerir; gövde kemikleri ağırlıksız kalır. Böylece Manny için yazılmış FP animasyon paketleri **yeniden hedefleme olmadan** ya da uyumlu iskelet (Compatible Skeletons) ile çalışır.
- Ek kemik yok; yalnız soket eklenir. `lowerarm_twist_01/02`, `upperarm_twist_01/02` ve metakarpal kemikler korunur (eldiven kalın olduğu için parmak ağırlıkları 2 kemik etkisiyle sınırlı tutulur; LOD yok, tek LOD).
- Soketler (kol iskeletinde):

| Soket | Kemik | Not |
|---|---|---|
| `GripPoint` | `hand_r` | **Kodda var** (`WeaponAttachSocketName`, varsayılan `GripPoint`). Silahın pivotu buraya sıfır ofsetle oturur; +X namlu yönü |
| `KedgePoint` | `hand_l` | Öneri; Kedge buraya bağlanır |
| `ClipPoint` | `hand_l` | Öneri; reload'da klips/fişek mesh'i |
| `CuffPoint_L` / `_R` | `lowerarm_l` / `_r` | Öneri; varyasyon manşet parçası |

- Sol el silaha **IK ile** oturur: hedef, silahın `Grip_L` soketi (`ik_hand_l`). Pompa hareketinde el pompayı izler; animasyonda el ile pompa ayrı anahtarlanmaz (kayma olmaz).
- `ABP_Arms_FP` katmanları (öneri; uygulama gameplay-developer ile): taban poz (silaha göre) → hareket/sprint → ADS karışımı (`GetAdsAlpha`) → additive ateş → tam gövde montage (reload, equip) → sol el IK → prosedürel sway/bob (developer; kapatılabilir, spec §2.1).
- Bütçe: iki kol toplam ≤ 20.000 üçgen · 1 set 2048² (`T_Arms_D/_N/_ORM`) + `T_Arms_Mask` 1024² · 1 materyal slotu.
- Kamera kesmesi: FP kol ve silah ayrı render yolunda/FOV'da (spec: 70). Bunu kodun nasıl yaptığı (ayrı FOV materyal fonksiyonu mu, 5.x first-person primitive özelliği mi) developer kararıdır; kol mesh'i üst koldan **kesik** olduğu için dünya gölgesi atmamalı (§10).

---

## 3. Düşmanlar

### 3.1 Scuttler (The Drift)

**Siluet:** köpek boyu, alçak, geniş yengeç. Şekil dili stil rehberi §3: **alçak, geniş, asimetrik, sivri ve kümeli.** Üç okuma: (1) yere yakın geniş elmas kabuk, (2) bir yanı belirgin büyük iki kıskaç (sol kıskaç 1,5× — asimetri), (3) sırtın arkasında parlayan turkuaz kese. Kabuk kenarında midye kümeleri ve düzensiz dikenler; bacaklar ince, eklemleri sivri, ritmi düzensiz.

**Ölçüler** (spec kapsülü: yarıçap 45, yükseklik 70 — bkz. §10 tutarsızlık 1):

| Ölçü | cm | Not |
|---|---|---|
| Duruş yüksekliği (kabuk tepesi) | 52 | |
| En yüksek nokta (kese tepesi) | **68** | ≤ 70: 110'luk alçak siperin arkasında görünmez (spec gerekçesi) |
| Kabuk eni × boyu | 78 × 70 | Kapsül çapı 90 içinde |
| Bacak açıklığı (uçtan uca) | 125 | Kapsülden taşar; **bacaklar çarpışmaya girmez**, isabet almaz/alır kararı §10 |
| Karın yerden yüksekliği | 22 (Wet eşiği d ≥ 20 ile aynı: su karnına değdiği an ıslaktır — okunaklı) | |
| Kıskaç (büyük / küçük) boyu | 42 / 28 | Pinch telgrafında kalkınca tepe noktası ~95 |
| Kese | **Ø 25** (spec ~25 cm küre) | Sırtın arka üçte birinde, merkezi Z ≈ 55; önden bakınca kabuğun ön sırtı keseyi kapatır |

**Zayıf nokta görsel işareti:** kese, modeldeki **tek emissive büyük yüzey**: turkuaz `#3FE0D0`, yarı saydam görünümlü (opak + subsurface taklidi; gerçek yarı saydamlık yok: sürüde overdraw). İkinci işaret (renk körü güvencesi): kese **nabız gibi atar** (ölçek ±%6, 1,2 Hz; Wet'te 2 Hz) ve gövdedeki tek yuvarlak, pürüzsüz formdur — geri kalan her şey sivri. Kabuğun ön sırtı keseyi önden ve aynı kottan kapatır; ≥ ~20° yukarıdan ya da arkadan görünür (yüksek zemindeki oyuncuya ödül; spec §5.1). Kod tarafı: `sac` kemiğine bağlı küre çarpışma gövdesi, `HeadBones` listesinde `sac` (ya da `NF.Head` etiketli küre bileşeni).

**İskelet** (`SKEL_Drift_Crawler` — Dredger bunun büyük türevi olacak; 27 kemik):

| Grup | Kemikler | Sayı |
|---|---|---|
| Kök | `root`, `body` | 2 |
| Kese | `sac` | 1 |
| Kıskaçlar | `claw_l_01` (kol), `claw_l_02` (ön kol), `claw_l_pincer`; `claw_r_*` | 6 |
| Bacaklar | `leg_fl_01/02`, `leg_ml_*`, `leg_rl_*`, `leg_fr_*`, `leg_mr_*`, `leg_rr_*` (ön/orta/arka × sol/sağ; 2 kemik) | 12 |
| Bacak IK hedefleri | `ik_leg_fl` … `ik_leg_rr` (ağırlıksız) | 6 (yalnız LOD0'da değerlendirilir) |

Skinning **rijit** (parça başına tek kemik; yalnız kese ve karın zarında 2 etki): ağırlık boyama işi ve LOD'da skin maliyeti düşer.

**Prosedürel adım yaklaşımı (üç kademe):**

| Kademe | Koşul (öneri) | Yöntem |
|---|---|---|
| Yakın | Oyuncuya ≤ 1000 cm **ve** en yakın 6 | `CR_Scuttler_Legs` (Control Rig): 6 ayak için aşağı iz + adım hedefi; alternatif üçlü yürüyüş (fl-mr-rl / fr-ml-rr), adım tetikleme = gövde hedeften 28 cm uzaklaşınca, adım süresi 0,12 sn. Gövde eğimi zemine uyar (rampalar 13–27°) |
| Orta | ≤ 2500 cm | Pişirilmiş yürüyüş döngüsü (`A_Scuttler_Walk`), oynatma hızı gerçek hıza bağlı; ayak kayması kabul edilir |
| Uzak | > 2500 cm ya da ekran dışı | Aynı döngü, güncelleme 15 Hz (URO) ya da Animation Budget Allocator |

Pişirilmiş döngü **her kademede yedektir**: Control Rig maliyeti hedef donanımda tutmazsa yalnız döngüyle gidilir. Su içinde ayaklar görünmez (opak orta/derin su); d ≥ 50'de yakın kademe de döngüye düşer.

**Animasyon listesi.** Hızlar spec §5.1: Wet 504, Dry 360, Parched 300. Tek yürüyüş döngüsü 360 cm/sn için yazılır; oynatma hızı Wet 1,4 · Dry 1,0 · Parched 0,83.

| Animasyon | Süre (sn) | Not |
|---|---|---|
| `Idle` | 2,0 döngü | Kıskaçlar hafif açılır-kapanır; kese nabzı materyalde |
| `Walk` | 0,50 döngü (360 cm/sn; adım 30 cm × 6 bacak düzeni) | Yukarıdaki oynatma hızlarıyla |
| `Circle_L` / `_R` | 0,50 döngü | Token'sız olanlar 250–400'de döner: yan yürüyüş (yengeç!) — dönerken **yan**, saldırırken **ön** gelir; oyuncu niyeti yönden okur |
| `Alert` | 0,40 | Fark etti: kıskaçlar bir kez şaklar (ses: tıkırtı hızlanır) |
| `Pinch` | **1,30** = telgraf **0,50** + toparlanma **0,80** | 0,00–0,50 iki kıskaç baş üstüne kalkar ve **durur** (son 0,15 sn hareketsiz poz: okunaklı an); 0,50 kapanır |
| `Lunge` | **1,00** = telgraf **0,60** + sıçrayış **0,40** | 0,00–0,60 gövde çöker, arka bacaklar gerilir, kese şişer; 0,60–1,00 havada, kıskaçlar önde |
| `Lunge_Miss` | **1,00** | Karın üstü kayma + doğrulma; kese açıkta ve hareketsiz (kolay hedef) |
| `Lunge_Hit` | 0,40 | Geri sekme |
| `Flinch_F` / `_L` / `_R` | **0,30** (additive) | Ölmeyen her isabette; spec "0,3 sn duraklar" |
| `Knockback` | 0,45 | Gaffer/Kedge itmesi (200 cm): bacaklar toplanır, kayar, açılır |
| `Spawn_Water` | **1,50** | Doğuş telgrafı 1,5 sn: sudan/ızgaradan çıkış |
| `Death_A` / `_B` | 0,6 + fizik | Bacaklar içe kıvrılır → ragdoll devri |
| `Death_Parched` | 0,35 | Ragdoll yok: kuru parçalanma (4–6 önceden kırılmış parça + toz) |
| `Dry_Shake` | 0,6 (additive, bir kez) | Wet → Dry geçişinde silkinme: durum değişimi görülür |

Notify'lar:

| Animasyon | Notify | Zaman |
|---|---|---|
| `Pinch` | `AN_Telegraph_Start` (ses "klak") · `AN_Attack_Hit` · `AN_Attack_End` | 0,00 · **0,50** · 1,30 |
| `Lunge` | `AN_Telegraph_Start` (tıslama + su sıçraması VFX) · `AN_Lunge_Launch` · `AN_Attack_Hit` | 0,00 · **0,60** · **1,00** |
| `Lunge_Miss` | `ANS_Vulnerable` | 0,00–1,00 |
| `Walk` | `AN_Footstep` × 2 (üçlü grup başına; 6 ayrı ses değil) | 0,00 · 0,25 |
| `Spawn_Water` | `AN_Spawn_Emerge` | 1,50 |

**LOD ve sürü bütçesi.** Spec tavanı: aynı anda en çok **17 Scuttler + 8 Attendant** (4 oyuncu). EA'da diğer arketipler gelince hedef 30+ aktör; bütçe buna göre.

| LOD | Üçgen | Ekran boyutu (öneri) | Kemik | Not |
|---|---|---|---|---|
| LOD0 | ≤ 6.000 | > 0,35 | 27 | Control Rig adayı |
| LOD1 | ≤ 2.800 | 0,35–0,12 | 21 (IK kemikleri atılır) | Dikenler ve midyeler siluete erir |
| LOD2 | ≤ 1.000 | < 0,12 | 15 (bacaklar tek kemik) | Kese emissive **korunur** (uzaktan zayıf nokta + sayım) |

Sürü üst sınırı (17 × LOD0): ~102.000 üçgen — ihmal edilebilir; gerçek maliyet animasyon ve çizim çağrısıdır. Kurallar: tek materyal slotu · durum değişimi **Custom Primitive Data** ile (aktör başına dinamik materyal instance yok) · doku 1 set 1024² (`T_Scuttler_D/_N/_ORM`) + emissive maske ORM'nin boş kanalında değil ayrı `T_Scuttler_E` 512² (tek kanal) · gölge: LOD2'de dinamik gölge kapalı · 3 boyut/oran varyantı ölçekle (0,9 / 1,0 / 1,1) ve kıskaç tarafı aynalamayla (sürüde tekdüzelik kırılır, ek varlık yok).

**Hit-react / ölüm / fizik:** `PHYS_Scuttler`: `body` (kutu), `sac` (küre Ø 25), iki kıskaç (kapsül), bacaklar için **gövde yok** (6 bacak × fizik gövdesi sürüde pahalı; ragdoll'da bacaklar animasyon pozunda donar). Ölümde 0,6 sn animasyon → gövde ragdoll → 6 sn sonra suya batarak ya da kuruda çökerek kaybolur (ceset birikimi 20+ düşmanda okunabilirliği ve performansı bozar). Kese vurularak öldürülürse: kese patlar (VFX), gövde anında söner.

**Durum görselleri (Wet / Dry / Parched):** `M_Enemy_Master` parametreleri, Custom Primitive Data ile; geçiş 0,4 sn yumuşak (spec'teki 0,2 sn durum gecikmesinin üstüne).

| Durum | Albedo / değer | Roughness | Işıltı (eklem + kese) | Ek |
|---|---|---|---|---|
| **Wet** (d ≥ 20) | Gövde `#16241F`, kabuk `#2A3A33` (L* 13–23) | 0,15 (ıslak parlaklık: kapalı göğü yansıtır, koyu ıslak zeminden ayıran şey budur) | %100; nabız 2 Hz | Damla + bacaklardan su akıntısı VFX; can yenilerken kese ışıltısı içe doğru "dolar" |
| **Dry** | Aynı albedo | 0,55 | %60; nabız 1,2 Hz | Geçişte `Dry_Shake` + kısa su serpintisi |
| **Parched** (20 sn Dry) | Gövde değeri **korunur** (L* ≤ 26); kabuk sırtlarında ve eklem kenarlarında **tebeşir beyazı tuz çizgileri** (`#DAD8CC`, yüzeyin < %15'i), doygunluk sıfıra yakın | 0,9 (tam mat) | %25, titrek; kese süt rengi turkuaz (`#9FCFC8`), nabız düzensiz | Kabuktan pul pul dökülme parçacığı; yürürken toz. Spec "renk solar"ı böyle yorumladım (§4.2, §10) |
| Suya geri giriş | Anında Wet (spec) | 0,15 | %100'e 0,2 sn'de sıçrar (parlama) | "Şlup" sıçrama VFX |

**VFX / ses ihtiyaçları:** `VFX_Scuttler_Wake` (su içinde ilerlerken yüzeyde köpük renginde V izi — **zorunlu**: d ≥ 70'te gövde tamamen su altında ve su opak; iz olmadan görünmez düşman olur) · `VFX_Scuttler_SpawnFoam` (1,5 sn doğuş telgrafı) · `VFX_Scuttler_LungeSplash` · `VFX_Scuttler_Drip` · `VFX_Scuttler_ParchedFlakes` · `VFX_Scuttler_Death`, `_Death_Parched`, `_SacBurst`. Sesler spec §5.1 tablosundaki altı işaret + ayak tıkırtısı (ıslak/kuru iki set) + ölüm.

### 3.2 Attendant (The Wardens)

**Siluet:** kasketli, ince, dik insansı otomat. Şekil dili: **dikey, simetrik, daire + silindir; ince bel, belirgin baş lambası.** Üç okuma: (1) baş = iri yuvarlak sinyal lambası, üstünde kısa siperlikli kasket; (2) krem emaye, omuzdan bele daralan "önlük" gövde, göğüste jeton yuvası; (3) koyu, boru gibi uzuvlar ve belirgin küre eklemler. Yüz yok. Sağ ön kol bilet zımbası (silah): kol ucunda iki çeneli pirinç zımba; ateş ederken çeneler "klik-klak" kapanır.

**Ölçüler** (spec kapsülü: yarıçap 40, yükseklik 190):

| Ölçü | cm | Not |
|---|---|---|
| Toplam boy (kasket tepesi) | **190** | Kapsül tepesiyle aynı; oyuncudan (180) 10 cm uzun: "resmî" durur |
| Lamba merkezi | **Z 175** | Spec |
| Lamba görünür mercek çapı | 26 (gövde/yuva çapı 30) | İsabet küresi yarıçapı 18 (Ø 36) spec'ten: görselden cömert (§10 tutarsızlık 3) |
| Kasket | 4 kalınlık, siperlik 9 öne | Lambanın üst yayını örter; merceği kapatmaz |
| Omuz genişliği | 46 | Kapsül çapı 80 içinde |
| Bel | 22 | İnce bel: Wardens imzası |
| Göğüs plakası | 34 × 40 | Jeton yuvası 6 × 1,2, pirinç çerçeve |
| Uzuv çapı (üst kol / ön kol / uyluk / baldır) | 7 / 8 / 10 / 11 | Bilek ve ayak bileğine doğru **kalınlaşır** (alt ağırlık; koyu kütleyi büyütür, §4.1) |
| Diz yüksekliği | 50 | Diz suyu eşiği (50–99) tam dizde başlar: kıvılcım dizden çıkar |
| Bel (kalça bloğu üstü) | 100 | Kısa devre eşiği d ≥ 100: su **koyu kalça bloğunu geçip krem gövdeye değdiği an** — eşik modelden okunur |

**Zayıf nokta görsel işareti:** lamba hem zayıf nokta hem telgraf göstergesi (spec). Durumlar:

| Durum | Mercek | İkinci işaret |
|---|---|---|
| Sakin / devriye | `#FFF4D6`, sabit, düşük şiddet | — |
| Telgraf (Punch Burst, 0,7 sn) | Tehlike kırmızısı `#E63B2E`, 0,7 sn'de şiddeti artar | Mercekte dönen çapraz şerit perdesi (stil §2.4: çapraz şerit) + "klik-klak" |
| Ateş | Kırmızı, mermi başına parlama | — |
| Shove telgrafı (0,4 sn) | Kırmızı, tek sert parlama | Kol geri çekilir |
| Servis (3,0 sn) | **Sönük** — ama pirinç çerçeve + merceğin açık cam yüzeyi şekil olarak okunur; 1 Hz zayıf kehribar "servis" göz kırpması **öneri** (§10 açık karar) | Baş öne eğik, göğüs kapağı açık |
| Sendeleme | 0,6 sn rastgele titreme | — |
| Kısa devre | Aşağıda | — |

Lamba, oyuncuya hep **yuvarlak, pirinç çerçeveli, koyu yuvaya oturmuş açık disk** olarak görünür: ışık sönse de şekil kalır. Kod tarafı: Epic `head` kemiği `HeadBones` içinde; fizik gövdesi küre r = 18, merkez Z 175.

**İskelet / rig (ortak Warden iskeleti):** `SKEL_Warden` = **UE5 Manny uyumlu** hiyerarşi ve kemik adları (yeniden hedefleme ve pazar yeri insansı animasyonları için) + ek kemikler:

| Ek kemik | Üst kemik | İş |
|---|---|---|
| `lamp` | `head` | Mercek/şerit perdesi dönüşü (materyalde de yapılabilir; kemik yedektir) |
| `cap` | `head` | Sendelemede kasket sekmesi |
| `coin_door` | `spine_04` | Servis: göğüs kapağı |
| `punch_jaw_u` / `punch_jaw_l` | `hand_r` | Zımba çeneleri |
| `weapon_muzzle` (soket: `Muzzle`) | `hand_r` | Mermi çıkışı |

- **Rijit skinning:** her parça tek kemiğe bağlı (otomat; deformasyon yok). Ağırlık boyama yok, LOD'da skin maliyeti düşük, parça değişimiyle diğer Warden'lar (hikâye §9.2) üretilir. Eklemlerde küre + körük (bakalit) boşluğu kapatır.
- Oranlar Manny'den farklı (ince, 190): **IK Retargeter** ile Manny animasyonları aktarılır; iskelet ayrı varlıktır, uyumlu iskelet olarak işaretlenir. Lifeguard (uzun bacak) ve Beadle (iri) aynı `SKEL_Warden` üzerinde kemik ölçeği/retarget profiliyle türetilir — bu belge yalnız Attendant'ı üretir, ama iskelet bu ikisini taşıyacak biçimde kurulur.
- Parmak kemikleri: sol el 3 parmaklı basit pençe (Manny'nin `index/middle/pinky` zincirleri; `thumb` ve `ring` ağırlıksız); sağ el zımba.
- Hareket karakteri: animasyonlar retarget sonrası **kare düşürme + poz tutma** ile mekanikleştirilir (yumuşak insan hareketi değil: 12 fps'lik adımlı baş/kol, gövde sabit dik). Bu, hazır animasyonu stile çeken ucuz işlemdir.

**Animasyon listesi** (spec §5.2 süreleri):

| Animasyon | Süre (sn) | Not |
|---|---|---|
| `Idle` | 3,0 döngü | Dik; baş yavaş tarar |
| `Patrol_Walk` (300 cm/sn) | 1,0 döngü | Kısa, eşit adım; kollar gövdeye yakın |
| `Reposition_Run` (420 cm/sn) | 0,7 döngü | Gövde hâlâ dik; "koşan memur" |
| `Cover_Idle_Low` / `_High`, `Cover_Peek` | 2,0 döngü / 0,35 | Alçak siperde diz kırar: lamba siperin (110) üstünde **kalır** mı kararı §10 |
| `Alert` | 0,6 | "May I see your ticket?": baş oyuncuya döner, lamba parlar |
| `Punch_Burst` | **1,35** = telgraf **0,70** + 3 mermi (0,70 · 0,85 · 1,00) + 0,35 dönüş | 0,00–0,70 sağ kol düz ileri kalkar ve kilitlenir; çeneler açılır. Her mermide çene kapanır + kol 2 cm geri teper |
| `Shove` | **0,90** = telgraf **0,40** + itme + 0,50 dönüş | 0,00–0,40 sol kol geri (dirsek omuz üstünde: iri, yandan da okunur) |
| `Service` | **3,00** | 0,00–0,40 baş eğilir, lamba söner, göğüs kapağı açılır; 0,40–2,60 sol el göğüste jeton/şerit değiştirir; 2,60–3,00 kapak kapanır, lamba yanar |
| `Stagger` | **0,60** | Geri iki adım (kod 150 cm iter); lamba titrer; telgrafı keser |
| `Flinch` (additive) | 0,20 | Sendeletmeyen isabet: omuz/gövde tık |
| `Retreat_Turn` | 0,45 | Geri çekilme: sırtını döner — sırtta sigorta kapağı yok (o Beadle'ın), yalnız krem sırt + koyu omurga |
| `Knee_Wade` | 1,4 döngü (hız ×0,7) | Dizde su: yüksek diz kaldırma, her adımda kıvılcım |
| `ShortCircuit` | **2,00** + `ShortCircuit_Collapse` 0,8 | Aşağıda |
| `Spawn_Shutter` | 1,50 | Kepenk telgrafı 1,5 sn sonrası kapıdan çıkış |
| `Death_Disabled` | 0,9 | Lamba patlar, gövde **kaskatı** öne/arkaya devrilir (ragdoll değil: kilitlenmiş otomat) |

Notify'lar:

| Animasyon | Notify | Zaman |
|---|---|---|
| `Punch_Burst` | `AN_Telegraph_Start` (lamba kırmızı + ses) · `AN_Attack_Fire` × 3 · `AN_Attack_End` | 0,00 · **0,70 / 0,85 / 1,00** · 1,35 |
| `Shove` | `AN_Telegraph_Start` · `AN_Attack_Hit` · `AN_Attack_End` | 0,00 · **0,40** · 0,90 |
| `Service` | `ANS_Vulnerable` · `AN_Lamp_Off` · `AN_Lamp_On` | 0,00–3,00 · 0,30 · 2,75 |
| `Stagger` | `ANS_Staggered` | 0,00–0,60 |
| Yürüyüşler | `AN_Footstep` | Adım başına |
| `Spawn_Shutter` | `AN_Spawn_Emerge` | 1,50 |

Saldırı zamanlamasını **kod** yönetiyorsa (Behavior Tree görevleri), notify'lar yalnız ses/VFX içindir ve animasyon uzunlukları kodun sürelerine kilitlenir (§9).

**LOD ve bütçe:**

| LOD | Üçgen | Ekran boyutu | Not |
|---|---|---|---|
| LOD0 | ≤ 14.000 | > 0,5 | |
| LOD1 | ≤ 6.500 | 0,5–0,2 | Parmaklar ve körükler sadeleşir |
| LOD2 | ≤ 2.200 | < 0,2 | Tercih menzili 12–25 m'de çoğunlukla LOD1/LOD2 görülür: **siluet ve iki ton LOD2'de eksiksiz** olmalı |

Doku: 1 set 2048² (`T_Warden_Common_D/_N/_ORM` — tüm Warden'ların paylaşacağı emaye/bakalit/pirinç atlası; Attendant'a özel parça dokusu yok) + `T_Warden_E` 512². Materyal: 2 slot (gövde; lamba merceği — mercek ayrı ki şerit perdesi ve durum rengi ucuz değişsin). Azami 8 Attendant × LOD0 = 112.000 üçgen.

**Hit-react / ölüm / fizik:** `PHYS_Warden`: baş (küre r 18), göğüs, kalça, üst/alt kol × 2, uyluk/baldır × 2 (11 gövde; Manny fizik varlığından uyarlanır). Tam ragdoll **kullanılmaz**: ölüm rijit devrilmedir (tek gövde gibi düşer, yerde 1 kez seker); bu hem fraksiyon karakteri hem ucuz. Devrilen gövde 8 sn sonra söner (lamba camı ve kasket ayrı küçük parça olarak 3 sn kalır). Lamba vurularak öldürülürse: mercek patlar, baş yuvası boş ve koyu kalır.

**Islakken / kısa devrede görsel durum:**

| Durum | Görsel | Ses/VFX |
|---|---|---|
| Kuru | Mat krem emaye (rough 0,35), koyu bakalit (0,5) | — |
| Ayak ıslak (d < 30; gönüllü girdiği sınır) | Ayak/baldırda ıslak koyulaşma (dünya Z maskesi: su yüksekliğinin altı) | Adımda sıçrama |
| **Diz** (50–99) | Diz eklemlerinden ve su çizgisinden aralıklı ark; lamba 4–6 Hz hafif titrer; yürüyüş `Knee_Wade` | `VFX_Warden_KneeSpark` (Elektrik rengi `#CFF4FF`, zikzak; küçük, opak ışık şeridi). Bark: "No bathing after s— s— six." |
| **Kısa devre sayacı** (d ≥ 100, 0 → 2,0 sn) | 0,0–0,7: su çizgisinde sürekli ark halkası, lamba hızlı kırmızı-beyaz titrer, omuzlar seğirir · 0,7–1,4: göğüs plakası dikişlerinden ark, gövde adım adım kilitlenir, hareket kesik kesik · 1,4–2,0: tüm eklemler ark, lamba aşırı parlar, baş geri | `VFX_Warden_ShortCircuit` 3 evreli; ses perdesi yükselir. **2,0 sn ilerleme çubuğu modelin kendisidir**: oyuncu ne kadar kaldığını görür |
| Sayacın kesilmesi (sudan çıktı) | Arklar 0,3 sn'de söner; 1 sn boyunca kıvılcım damlaları | Bu durumda hasar ya da sendeleme var mı: spec'te yok (§10) |
| **Devre dışı** (2,0) | Tek büyük parlama, lamba patlar, `ShortCircuit_Collapse`: kaskatı devrilip suya düşer; su yüzeyinde 1 sn köpük halkası + duman | Bark: "Thank you for visiting Carrow Sands." Prototipte çevreye elektrik hasarı yok: efekt **alan tehdidi gibi görünmemeli** (yarıçap ≤ 80 cm, yere yayılan halka yok) |

**Mermi (Punch Burst):** 3500 cm/sn yavaş mermi; oyuncu görüp kaçacak. `VFX_Warden_Punch_Projectile`: Tehlike kırmızısı, 14 cm uzunlukta parlak çubuk + kısa iz; ikinci işaret: dönerek gelen dikdörtgen "bilet" silueti. İsabet/ıska: `VFX_Warden_Punch_Impact`. Namlu: zımba ucunda küçük kırmızı parlama (`Muzzle` soketi).

---

## 4. Stil rehberindeki iki soruya yanıt

### 4.1 Wardens iki tonlu gövde (krem + koyu uzuv) — **kabul, bir düzeltmeyle**

**Kabul gerekçesi.** Rehberin §8.3 hesabı doğru: krem emaye (L* 94) açık pastel cephe (77–82) önünde yalnız 12–17 L* fark verir; kural 25 istiyor. Koyu bakalit (L* 19) aynı arka planda ~58 fark verir. Tersi de geçerli: Attendant arcade içinde (çikolata kahve L* 31, ıslak bant) ve su çizgisi altı koyu yüzeyler önünde de durur; orada koyu parça kaybolur ve krem gövde taşır (fark 60+). İki ton, tek bir arka plana değil **iki arka plana birden** sigortadır. Ayrıca dikey-simetrik makine dilini güçlendirir ve bel eşiğini modelden okutur (koyu kalça / krem gövde sınırı = Z 100 = kısa devre eşiği).

**Düzeltme (itiraz ettiğim tek nokta): "koyu uzuv" tek başına yetmez; koyu kütle uzuvlardan büyük şekillere taşınmalı.** Attendant "ince"dir ve tercih menzili 12–25 m. Kaba hesap (FOV 95, 1920 px, **motor içinde ölçülmedi**): 25 m'de ekran ~0,35 px/cm. 8 cm'lik ön kol ~3 px, 46 cm'lik omuz ~16 px. Okumayı 3 piksellik çizgiler taşıyamaz. Bu yüzden:

| Bölge | Ton | Neden |
|---|---|---|
| Göğüs + sırt plakası, kasket üstü | Krem `#F4EEDD` | Fraksiyon kimliği; koyu arka planda taşıyıcı |
| **Baş yuvası (lambanın arkası ve yanları), omuz boyunduruğu, kalça bloğu** | Koyu `#23302B` | Büyük koyu kütleler: lambayı çerçeveler (açık disk + koyu halka = hedef tahtası), gövdeyi üstten ve alttan keser |
| Uzuvlar | Koyu; **bileğe doğru kalınlaşan** (7 → 8, 10 → 11 cm), ön kol ve baldırda manşon | Koyu alanı artırır, alt ağırlık verir |
| Eklemler, jeton yuvası çerçevesi, lamba çerçevesi, zımba | Pirinç `#B08D3C` | Orta değer; ayırıcı çizgi |

Hedef: ön siluet alanının **en az %40'ı koyu, en az %35'i krem**. Bu oran ve 25 m okuması ilk UE gri kutusunda gri tonlama testiyle (stil §8.4) doğrulanır; tutmazsa ilk ayar omuz boyunduruğunu büyütmektir, gövdeyi koyultmak değil.

Ek not: lamba sakin rengi `#FFF4D6` ile krem emaye çok yakın (ikisi de L* ~94–96). Lamba ancak koyu baş yuvası sayesinde ayrışır; bu da yukarıdaki düzeltmenin ikinci gerekçesi.

### 4.2 Drift koyu değer bandı (L* 13–23, sahnedeki en koyu büyük şekil) — **kabul, üç koşulla**

**Kabul gerekçesi.** Kuru ortam (60–82), sis (73) ve açık su yüzeyi (57–72) önünde 35–60 L* fark verir; Drift'in savaştığı yerin çoğu budur (kuru kumsal, sahil yolu, terasta 30 cm su). Wardens'la karışmaz (zıt uç), sivri-alçak siluetle birleşince arketip 1 saniyede ayrılır.

**Koşul 1 — ıslak koyu zeminde bant tek başına yetmiyor; ikinci güvence modele yazılmalı.** Rehber bunu kural 2'de kendisi söylüyor; rakamı netleştiriyorum: kabuk L* 23, iskele altı/zift L* 28, midye L* 28, ıslak yosun L* 33 → fark **5–10**, kuralın çok altında. Tam bu bölgede D-Low doğuş noktası ve Ganimet B var (iskele altı). Bu yüzden Scuttler'da üç şey **isteğe bağlı değil**: (a) ıslakken düşük roughness (0,15) — kapalı gök kabuk sırtında açık bir yansıma şeridi çizer; (b) eklem aralarında ve karında turkuaz iç ışıltı — alttan zemine hafif yansır; (c) kese LOD2'de de emissive. Karşılığında environment-artist'ten istek: iskele altı ve benzeri "Drift koridorlarında" zemin/duvar değeri **L* ≥ 33** tutulsun (zift `#4A4038` büyük yüzey olarak kullanılmasın) ve "hâlâ çalışıyor" ışıklarından biri oraya konsun.

**Koşul 2 — tamamen su altındaki Drift için bant anlamsız; yüzey izi şart.** Su 50 cm'den sonra zemini siler, 150'den sonra opaktır (stil §5.1). Scuttler 68 cm: d ≥ 70'te hiç görünmez. Rehber Undertow için V izinden söz ediyor; aynı köpük renkli iz Scuttler için de zorunludur (`VFX_Scuttler_Wake`). Bu sanat işi ama oynanış bilgisidir: designer'ın "okunabilir ölüm" ölçütü (spec §1.4, ölçüt 5) buna bağlı.

**Koşul 3 — Parched durumunda değer yükseltmiyorum.** Spec "renk solar" diyor. Gövdeyi açarsam (L* 35–45) Parched Scuttler iskele ahşabı (46), kaldırım (63) ve ıslak bantla karışır — tam da oyuncunun "tek atışlık" hedefi seçmesi gereken anda. Çözüm: gövde koyu kalır (L* ≤ 26), "solma" **doygunluk kaybı + tam mat yüzey + ince tebeşir beyazı tuz çizgileri + sönük ışıltı** ile verilir (§3.1). Tuz çizgisi rehberin "açık uç yalnız ince çizgi" kuralına uyar. Designer bunu "yeterince farklı görünmüyor" bulursa ikinci aday: Parched'da kabuk kenarlarının pul pul kalkması (siluet değişimi), değer değişimi değil.

**Yan not (renk çakışması):** Drift turkuazı `#3FE0D0` oynanış renk tablosunda (§2.4) yok ama fiilen "Drift / zayıf nokta" anlamı taşıyor; Elektrik `#CFF4FF` ve Can yeşili `#4FD66A` ile komşu. Öneri: turkuaz tabloya "Drift (canlı tehdit)" olarak eklensin, ikinci işaret = nabız. Kısa devre arkı (Elektrik) her zaman **zikzak ve beyaza yakın**, Drift ışıltısı her zaman **yumuşak ve nabızlı** kalsın; renk körü ön ayarlarında bu ikisi birlikte test edilsin.

---

## 5. Fab / Marketplace hazır paket kullanımı

Kullanıcının Unreal hesabındaki paketlerin listesi bu oturumda görülemedi; aşağıda **paket adı yok**, yalnız tür ve ölçüt var. Paket seçimi, kullanıcı kütüphanesini editörde açtığında bu tabloyla yapılır.

### 5.1 Prototipte yer tutucu olarak işe yarayacak türler

| Tür | Prototipte ne işe yarar | Bu projeye özel dikkat |
|---|---|---|
| FP kol mesh'i + FP animasyon seti (tabanca, pompalı) | En büyük kazanç: idle/ateş/reload/çekme/sprint hazır gelir; gunplay hissi ilk günden sınanır | Reload süreleri bizimkilerle (2,2 / 2,8) tutmaz: oynatma hızıyla ölçeklenir (§9). Toplu tabanca animasyonu nadirdir; şarjörlü tabanca animasyonu Purser'da yanlış okunur ama prototip sorusunu etkilemez. Pompa atışı 0,857'ye sığmalı |
| Tabanca ve pompalı modeli (tercihen iskeletli) | Vekil kutunun yerine silah silueti; `Muzzle` soketi eklenir | Modern taktik görünüm (ray, optik, polimer) stile aykırı: yalnız prototipte. İskeletliyse kemik adları animasyon setiyle eşleşmeli |
| İnsansı düşman animasyonları (yürüme, koşu, siper, tek el nişan/ateş, itme, sendeleme, ölüm) | Attendant için doğrudan: `SKEL_Warden` Manny uyumlu olduğu için retarget edilir | "Tüfekli asker" setleri iki el tutuş içerir; Attendant tek kolla ateş eder: tabanca ya da tek el setleri daha yakın. Mekanikleştirme (§3.2) sonradan uygulanır |
| Örümcek / yengeç benzeri çok bacaklı yaratık (iskelet + yürüme/saldırı/ölüm) | Scuttler yer tutucusu; sürü davranışı ve sayım okunabilirliği sınanır | Kapsüle (r 45, yük. 70) ölçeklenir. Sırtına parlak küre eklenir (zayıf nokta). 17 adet aynı anda: üçgen ve kemik sayısına bak |
| Niagara silah VFX seti (namlu alevi, iz, isabet, kovan) | `MuzzleFlashFX` / `TracerFX` / `ImpactFX` alanlarına bağlanır | İz sistemi `User.BeamEnd` okumalı (yoksa parametre eklenir). Cascade paketleri doğrudan bağlanamaz (tech §6) |
| Geçici ses kütüphanesi | Spec §10 bunu benden istiyor: çan, tıkırtı, klik-klak, tıslama, kepenk, su, hitmarker | Lisans: ticari kullanım + yeniden dağıtım koşulu |

### 5.2 Bir paketi değerlendirme ölçütleri

| Ölçüt | Bakılacak şey | Eleme eşiği |
|---|---|---|
| **İskelet uyumu** | UE5 Manny/Quinn mi, UE4 Mannequin mi, özel mi? FP kol seti tam gövde iskeleti mi, yalnız kol iskeleti mi? Silah iskeletinde kemik adları | Özel iskelet + IK Retargeter profili yoksa ve animasyon sayısı > 20 ise yer tutucu olarak bile pahalı. UE4 Mannequin → retarget gerekir, kabul |
| **Motor sürümü** | Paket UE 5.x destekliyor mu (proje 5.8 hedefli); Cascade mi Niagara mı; eski AnimBP düğümleri | Yalnız UE4 + Cascade ise VFX için ele |
| **Lisans** | Fab Standart Lisansı ticari oyunda kullanıma izin veriyor mu; "yalnız UE ile kullanım" kısıtı (eski Epic içerikleri); editoryal/kişisel kısıt; kaynak dosya dahil mi | Lisans metni okunmadan nihai ürüne girmez. **Paket sayfasında üretken yapay zekâ ile üretildiği belirtiliyorsa** varlık listesinde işaretlenir ve GM'e bildirilir (Steam beyanı). Lisans koşulları satın alma tarihine göre değişebilir: her paket için lisans türü ve tarih `docs/studio/art/` altındaki varlık listesine yazılır |
| **Stil uyumu** | Fotogerçekçi mi; modern askerî mi; gerçek marka/model birebir mi | Prototipte stil elenme nedeni değil. Nihai üründe: gerçek silah modelinin birebir kopyası ve modern taktik aksesuar **kullanılmaz** |
| **Üçgen / doku bütçesi** | LOD var mı; düşmanda LOD0 üçgen sayısı; 4K doku setlerinin sayısı; materyal slot sayısı | Sürü düşmanı LOD0 > 15.000 üçgen ya da LOD'suz ise 17 adet için ele ya da LOD üret. Silahta 4+ materyal slotu: prototipte kabul, nihai üründe hayır |
| **Animasyon kalitesi ve kapsam** | Root motion mu in-place mi (kodumuz hareketi kendisi yürütür: in-place gerekir); additive ateş var mı; reload'da notify var mı; 60 fps mi | Yalnız root motion ise her klip dönüştürülmeli: maliyeti say |
| **Ölçek / eksen** | 1 uu = 1 cm; silah +X ileri mi | Düzeltilebilir (`WeaponAttachOffset`), not al |
| **Ayrıştırılabilirlik** | Animasyonlar pakette kendi Blueprint sistemine gömülü mü; saf varlık olarak çıkarılabiliyor mu | Kendi karakter/silah sistemi olan "FPS kit"lerin **kodu alınmaz**: yalnız mesh, animasyon, VFX, ses. Kod `Source/NewFPS` olarak kalır |

### 5.3 Nihai üründe özgün üretilmesi gerekenler

| Varlık | Neden özgün |
|---|---|
| **Purser, Gaffer, Kedge** (ve sonraki tüm silahlar) | Ekranın en çok bakılan varlığı ve oyunun kimliği; 1970'ler liman ekipmanı dili hazır pakette yok; gerçek model kopyası riski |
| **FP kollar** (eldiven + muşamba kol) | Mağaza görsellerinin her karesinde; pazar yeri kolları tanınır. İskelet Epic uyumlu kalır |
| **Scuttler ve Attendant** (tüm düşmanlar) | Fraksiyon siluetleri, değer bandı ve zayıf nokta dili oynanışın parçası; hazır yaratık bunları taşımaz |
| **Silaha özgü reload ve ateş animasyonları** | Kırma toplu, fişek fişek pompalı, sol el kanca: mekanizmaya özel |
| **Düşman telgraf animasyonları** (Pinch, Lunge, Punch Burst, Shove, Service, ShortCircuit) | Süreleri oynanış verisi; hazır klip 0,5/0,6/0,7/0,4 sn'ye oturmaz |
| **Oynanış bilgisini taşıyan VFX** | Zayıf nokta isabeti, kısa devre, Scuttler su izi, Warden mermisi, durum geçişleri: renk dili (stil §2.4) bize özel |
| **İmza sesler** | Spec'e göre oynanış telgrafı; ses tasarımcısı/dış kaynak (hikâye §9.5) |

**Nihai üründe kalabilecek hazır içerik:** insansı hareket animasyonları (yürüme, koşu, dönme, siper geçişleri — mekanikleştirme işleminden sonra), genel Niagara yapı taşları (kıvılcım, toz, su sıçraması modülleri; renk ve biçim bize göre ayarlanmış), ses kütüphanesi katmanları (imza sesler hariç). Her biri lisans kaydıyla.

---

## 6. Üretim sırası ve kaba süre tahmini

> **Tahmin.** Tek sanatçı, Blender + UE erişimi var, kesintisiz çalışma varsayımı. Hiçbir kalem ölçülmedi; ilk varlıktaki hat doğrulaması süreleri değiştirebilir. "Gün" = iş günü. Bağlantı yokken (bugünkü durum) yalnız betik/belge üretilebilir; aşağıdaki süreler bağlantı kurulduktan sonrası içindir.

### 6.1 Aşama A — prototip (vekil modeller; spec §10'daki işim)

| # | İş | Tahmin (gün) | Bağımlılık |
|---|---|---|---|
| A1 | Hat doğrulaması: `SK_Purser` vekil (kutu + silindir, 5 kemik, `Muzzle`) → FBX → UE → iz doğru yerden çıkıyor | 0,5 | Blender + UE bağlantısı |
| A2 | Vekil Gaffer + Kedge + Strongbox (60 × 40 × 40), dolap, vinç direği | 0,5 | A1 |
| A3 | Vekil Scuttler (ilkel; sırtta parlak küre; 2 poz: kıskaç kalkık / çökük) ve Attendant (dik-ince; lamba 3 durum) | 1 | Kapsül kararı (§10-1) |
| A4 | Hazır paket taraması (§5) ve bağlama: FP kol + animasyon, yaratık, insansı animasyon | 1–2 | Kullanıcının kütüphane listesi |
| A5 | Geçici ses seti + geçici VFX (namlu, iz, isabet, lamba rengi) | 1 | — |
| | **Aşama A toplamı (tahmin)** | **4–5** | |

### 6.2 Aşama B — dikey dilim (nihai kalite)

Sıra gerekçesi: önce ekranın en çok bakılan şeyi (Purser + kol), sonra sayıca en çok görülen (Scuttler), sonra en pahalı rig (Attendant).

| # | İş | Tahmin (gün) | Not |
|---|---|---|---|
| B1 | FP kollar: model, UV, doku, Manny iskeletine bağlama, 4 varyasyon instance | 5–6 | Tüm FP animasyonların ön koşulu |
| B2 | Purser: model, doku, rig, soketler | 4–5 | |
| B3 | Purser FP animasyon seti (§1.1) + `ABP_Arms_FP` katmanları (developer ile) | 4–5 | ADS hizası koddan |
| B4 | Purser VFX (namlu, iz, isabet × 4 yüzey, zayıf nokta × 2, kovan) | 2–3 | |
| B5 | Scuttler: model, 3 LOD, doku, rijit rig, fizik varlığı | 4–5 | |
| B6 | Scuttler animasyonları + `CR_Scuttler_Legs` + durum materyali + VFX | 6–8 | Control Rig en belirsiz kalem |
| B7 | Gaffer: model, doku, rig | 5–6 | |
| B8 | Gaffer FP animasyonları (fişek fişek reload dahil) + VFX | 4–5 | |
| B9 | Kedge: model, doku, sol el animasyonları, VFX | 2–3 | Tutuş kararı (§10) |
| B10 | Attendant: model, 3 LOD, ortak Warden atlası, `SKEL_Warden`, fizik | 6–8 | İskelet diğer 3 Warden'ı taşıyacak |
| B11 | Attendant animasyonları (retarget + mekanikleştirme + 6 özgün telgraf klibi) + lamba/kısa devre VFX | 6–8 | |
| B12 | Dünya sürümleri (silah LOD'ları), gri tonlama testi, bütçe ölçümü, düzeltme turu | 3–4 | systems-developer ölçümü |
| | **Aşama B toplamı (tahmin)** | **51–66 gün (~10–13 hafta)** | Tek kişi; inceleme/yeniden iş payı dahil değil (+%20–30 beklenir) |

Riskler: (1) Control Rig bacak adımının sürüde maliyeti — yedek plan pişirilmiş döngü; (2) ADS hizası ve FP render yolu koda bağlı; (3) Blender/UE bağlantısı kurulamazsa bu takvim başlamaz.

### 6.3 Diğerlerinden beklenenler

| Kimden | Ne | Ne zaman |
|---|---|---|
| **game-designer** | Scuttler kapsül kararı (§10-1) · Kedge tutuşu: silah değiştirmeden sol el mi (§10) · Kedge savurması reload'u keser mi · Attendant servis sırasında lamba tam sönük mü, göz kırpar mı · alçak siperde Attendant'ın lambası siper üstünde kalır mı · Shove toparlanma süresi · Parched yorumu (§4.2 koşul 3) onayı · Scuttler bacakları isabet alır mı | A3'ten önce (kapsül); B5/B9'dan önce (diğerleri) |
| **gameplay-developer** | Ek soket adlarının onayı (`ShellEject`, `Sight`, `Grip_L`, `KedgePoint`, `ClipPoint`) · `UNFWeaponData`'ya animasyon alanları (kol ve silah için ateş/reload/equip montage ya da AnimBP sınıfı) · reload mermisi zamanlayıcıda mı notify'da mı (§9) · ADS'de `Sight` soketini kamera merkezine oturtan prosedürel katman · yüzeye göre `ImpactFX` seçimi ve saçma başına iz · Kedge veri varlığında animasyon/isabet zamanı · düşman durumlarını materyale veren parametreler (Custom Primitive Data: `Wetness`, `Parched`, `Glow`, `LampState`, `ShortCircuitAlpha`) · saldırı zamanlamasının sahibi (BT görevi mi, notify mı) · `HeadBones` için `sac` ve `head` · FP kol/silah FOV 70 render yolu ve gölge | B3'ten önce |
| **systems-developer** | Hedef donanımda ölçüm: 17 Scuttler + 8 Attendant ile animasyon ve çizim çağrısı maliyeti; Control Rig × 6 maliyeti; URO/Animation Budget Allocator ayarı; düşman başına gölge maliyeti | B6 sonunda |
| **environment-artist** | "Drift koridorlarında" (iskele altı vb.) büyük yüzey değeri L* ≥ 33 ve bir pratik ışık (§4.2 koşul 1) · Wardens doğuş kapıları ve durdukları parapet arkası için arka plan değeri (krem gövde + koyu kütle testi) · turkuazın renk dili tablosuna eklenmesi · `MPC_Tide` su yüksekliği: düşman ıslak koyulaşma maskesi aynı parametreyi okur · ölçek mankeni ve siper yükseklikleri (110/200+) ile ortak gri tonlama test sahnesi | B5/B10'dan önce |
| **studio-gm** | Hazır paket lisans kayıt düzeni (varlık listesi) · üretken yapay zekâ kullanılacaksa önceden karar · kullanıcıdan Fab kütüphane listesi · Blender/Unreal MCP bağlantısının kurulması | Aşama A başlamadan |

---

## 7. Teslim kontrol listesi (her varlık için)

Ölçek mankeniyle boy doğru · pivot ve +X yönü doğru · adlandırma §0'a uygun · soket adları kodla birebir · fizik varlığı/çarpışma var, kapsülde `Weapon` kanalı Ignore (iskeletli düşmanlar) · LOD zinciri ve ekran boyutları ayarlı · doku boyutu bütçede · yalnız `MI_` kullanıyor · animasyon süreleri veri varlığındaki sayılarla eşleşiyor (§9) · notify adları sözlükte · hedef ışıkta ve **gri tonlamada** oyun içinde kontrol edildi (kuru / half / high su) · üçgen ve çizim çağrısı bütçede · hazır/yapay zekâ içerik varlık listesinde işaretli.

---

## 8. Varlık listesi (özet)

### 8.1 Silah ve kol

| Varlık | Üçgen (LOD0) | Doku | Kemik |
|---|---|---|---|
| `SK_Purser` | ≤ 18.000 | 2048² | 11 |
| `SK_Gaffer` | ≤ 28.000 | 2048² | 9 |
| `SM_Kedge` | ≤ 4.000 | 1024² | — |
| `SK_Arms_FP` (+ 4 `MI_`, 4 manşet parçası) | ≤ 20.000 | 2048² + maske 1024² + dekal 512² | Manny tam hiyerarşi |
| Fişek/kovan/klips mesh'leri (5 adet) | ≤ 1.000 toplam | Silah atlasları | — |
| Dünya sürümleri (`SM_*_World` × 3) | ≤ 7.200 toplam | 512² × 2 | — |

FP ekranda aynı anda: kol + bir silah + Kedge ≤ ~52.000 üçgen (skill başlangıç bandı içinde, alt yarıda — stil yoğun geometri istemiyor).

### 8.2 Düşman

| Varlık | LOD0 / 1 / 2 | Doku | Kemik | Aynı anda azami |
|---|---|---|---|---|
| `SK_Scuttler` | 6.000 / 2.800 / 1.000 | 1024² + E 512² | 27 | 17 |
| `SK_Attendant` | 14.000 / 6.500 / 2.200 | 2048² ortak Warden + E 512² | Manny + 5 | 8 |

### 8.3 VFX (Niagara) — 27 sistem

Silah: `VFX_Purser_Muzzle`, `VFX_Gaffer_Muzzle`, `VFX_Tracer_Bullet`, `VFX_Tracer_Pellet`, `VFX_Purser_Casings`, `VFX_Gaffer_ShellEject`, `VFX_Kedge_Trail` · İsabet: `VFX_Impact_Default`, `_Water`, `_Warden`, `_Drift`, `_WeakPoint_Lamp`, `_WeakPoint_Sac`, `_Melee_Warden`, `_Melee_Drift`, `VFX_Stagger_Burst` · Scuttler: `_Wake`, `_SpawnFoam`, `_LungeSplash`, `_Drip`, `_ParchedFlakes`, `_Death`, `_Death_Parched`, `_SacBurst` · Warden: `_KneeSpark`, `_ShortCircuit`, `_Punch_Projectile` (+ `_Punch_Impact` aynı sistemin ikinci yayıcısı).

Ortak kural: yarı saydam büyük duman yok; kıvılcım/sıçrama/pul opak ya da additive küçük parçacık. Hiçbir efekt düşmanın siluetini ya da zayıf noktasını 0,2 sn'den uzun kapatmaz.

### 8.4 Hazır ve yapay zekâ içerik kaydı

Şu an: **hazır paket yok, yapay zekâ üretimi yok** (hiçbir varlık üretilmedi). Kullanılan her hazır paket için: ad, satıcı, lisans türü, edinme tarihi, hangi varlıkta kullanıldığı, nihai üründe kalıp kalmayacağı; yapay zekâ üretimi için ek olarak araç ve yapılan temizlik. Kayıt `docs/studio/art/asset-register.md` olarak açılacak (ilk hazır paket bağlandığında).

---

## 9. Animasyon–veri tutarlılık kuralı ve notify sözlüğü

**Kural:** oynanış süresinin sahibi veri varlığıdır (`ReloadTime`, RPM, telgraf süreleri). Animasyon o sayıya göre yazılır; sayı playtest'te değişirse animasyon yeniden yazılmaz, **oynatma hızı = animasyon süresi / veri süresi** ile ölçeklenir (0,8–1,25 aralığında göze batmaz; dışına çıkarsa yeniden zamanlanır). Atış animasyonu atış aralığından **kısa** olmak zorunda: Purser 0,33 < 0,40, Gaffer 0,80 < 0,857, Kedge 0,60 = 0,60 (son 0,15 sn toparlanma, kesilebilir).

Koddaki mevcut durum (okuduğum kadarıyla): reload bir zamanlayıcıyla bitiyor (`ReloadTimerHandle`), AnimBP `IsReloading` / `OnReloadStateChanged` / `OnFired` / `OnDryFire` dinliyor; `UNFWeaponData` sunum bölümünde animasyon alanı yok. Bu yüzden aşağıdaki notify'lar **şimdilik yalnız ses/VFX/görsel içindir**; hiçbiri mermi vermez, hasar uygulamaz. Bunu değiştirmek developer kararıdır.

| Notify | Nerede | Anlamı |
|---|---|---|
| `AN_Reload_Open` | Purser, Gaffer | Mekanizma açıldı (ses) |
| `AN_Reload_Eject` | Purser | Kovanlar atıldı (VFX + ses) |
| `AN_Reload_ClipShow` / `AN_Reload_ClipHide` | Purser | El mesh'ini göster/gizle |
| `AN_Reload_Insert` | Purser (1×), Gaffer (fişek başına) | Mermi girdi (ses). Fişek fişek dolumda developer isterse mermi artışını buna bağlar |
| `AN_Reload_Close` | Purser | Kapandı (ses) |
| `AN_Reload_Commit` | Hepsi | Animasyonun "dolu" sayıldığı kare = `ReloadTime` |
| `AN_Pump_Back` / `AN_Pump_Forward` | Gaffer | Pompa sesi |
| `AN_ShellEject` | Gaffer | Boş fişek VFX |
| `AN_Melee_Whoosh`, `AN_Melee_Hit`, `ANS_Melee_Trail` | Kedge | Ses / isabet karesi (0,15) / iz |
| `AN_Telegraph_Start` | Düşman saldırıları | Telgraf sesi + görsel başlangıcı |
| `AN_Attack_Hit` | Pinch, Lunge, Shove | Hasar karesi |
| `AN_Attack_Fire` | Punch Burst | Mermi çıkışı (3×) |
| `AN_Lunge_Launch` | Lunge | Sıçrayış başlar |
| `AN_Attack_End` | Düşman saldırıları | Toparlanma bitti |
| `ANS_Vulnerable`, `ANS_Staggered` | Lunge_Miss, Service, Stagger | Durum penceresi (yalnız işaret) |
| `AN_Lamp_Off` / `AN_Lamp_On` | Service | Lamba durumu |
| `AN_Footstep` | Yürüyüşler | Ses + ıslak zeminde sıçrama |
| `AN_Spawn_Emerge` | Doğuş | Telgraf bitti, aktör etkin |

---

## 10. Spec ile tutarsızlıklar ve açık kararlar

### 10.1 Tutarsızlıklar

| # | Nerede | Sorun | Önerim |
|---|---|---|---|
| 1 | Spec §5.1 Scuttler kapsülü "yarıçap 45, yükseklik 70" | UE kapsülünde yarı yükseklik yarıçaptan küçük olamaz: yükseklik 70 → yarı yükseklik 35 < 45. Motor bunu yarıçap 45'lik küreye (yükseklik 90) yuvarlar ya da reddeder | (a) yarıçap 35, yarı yükseklik 35 (Ø 70 küre) + bacaklar çarpışma dışı — geçitlerde daha rahat sürü; ya da (b) yarıçap 45, yükseklik 90. İkisinde de model 68 cm kalır ve 110'luk siperin arkasında görünmez. Model §3.1 ikisine de uyar; karar designer + developer |
| 2 | Spec §4.2 Gaffer `ReloadTime` 2,8 "tek parça tam dolum" | Pompalıda 6 fişeği 2,8 sn'de tek hareketle sürmenin inandırıcı animasyonu yok (şarjör/klips tasarlamadıkça) | Prototipte 6 hızlı sürme (0,35 aralık) tek animasyon olarak oynar; dikey dilimde aynı kareler başla/döngü/bitir olarak bölünür: 0,40 + 6 × 0,35 + 0,30 = 2,80. Designer'ın sayısı değişmez |
| 3 | Spec §5.2 Attendant lamba "~18 cm yarıçap, Z ≈ 175", kapsül yüksekliği 190 | Küre 157–193 arasını kaplar: kapsül tepesini 3 cm aşar; ayrıca Ø 36 cm, başın tamamından büyük | Görsel mercek Ø 26, isabet küresi r 18 kalsın (cömert zayıf nokta) ama merkez Z 172'ye insin ya da yarıçap 15 olsun. Küçük; designer bilgisi için |
| 4 | Spec §4.3 Kedge "tuştan isabete 0,15", "pompa dolarken boşluk" ↔ §2.2 "silah değiştirme 0,40" | Kedge ayrı silah yuvasıysa 0,15'te vuramaz ve reload boşluğunu dolduramaz | Kedge'i silah değiştirmeden sol elle savrulan hızlı yakın dövüş olarak modelledim (§1.3). Onay gerekir |
| 5 | Spec §5.2 servis: "3,0 sn açıkta, lamba söner" + zayıf nokta = lamba | Oyuncunun en çok vurması gereken pencerede hedef göstergesi kapanıyor | Lamba şekil olarak okunur kalır (pirinç çerçeve + koyu yuva); isteğe bağlı 1 Hz kehribar göz kırpma. Karar designer |
| 6 | Spec §5.1 Parched "renk solar" ↔ stil rehberi §2.5/§8 Drift "sahnedeki en koyu şekil" | Açılan gövde kuru zeminde okunabilirliği düşürür | Değer sabit; doygunluk, parlaklık, tuz çizgisi ve ışıltı değişir (§4.2 koşul 3) |
| 7 | Stil rehberi §8 kural 2 ↔ iskele altı/ıslak bant | Drift kabuğu (L* 23) ile zift/midye (L* 28) arası fark 5: kural 25 istiyor; D-Low doğuşu ve Ganimet B tam orada | §4.2 koşul 1: modelde ıslak yansıma + ışıltı zorunlu; ortamda o koridorlarda L* ≥ 33 |
| 8 | Tech §6: kodda yalnız `Muzzle` ve `GripPoint` soketi tanımlı | Kovan çıkışı, ADS hizası, sol el IK ve Kedge için soket adı yok; `UNFWeaponData` sunum bölümünde animasyon alanı yok; tek `ImpactFX` (yüzey ayrımı yok); tech §8 "ADS'te silahın nişangâha hizalanması yok" | §6.3'teki developer istek listesi. Adları bu belgede **öneri** olarak işaretledim |
| 9 | Spec §3.3 ıslaklık adları Dry/Knee/Waist/Head ↔ `NFTypes.h` `ENFWetness` Dry/Knee/Waist/**Overhead**; koddaki yorum "Waist: silah kullanılamaz" ↔ spec "Waist: Purser hip-fire çalışır" | Benim alanım değil ama materyal/animasyon durum adlarım bu enum'a bağlanacak | Developer + designer arasında netleşsin; ben kod adını (`Overhead`) kullanırım |
| 10 | Spec §5.2 Shove | Telgraf (0,4) ve hasar var, toparlanma süresi yok | Animasyonu 0,50 toparlanmayla yazdım (toplam 0,90); sayı designer'dan |

### 10.2 Açık kararlar

| # | Karar | Kimde | Beni nasıl etkiler |
|---|---|---|---|
| 1 | Scuttler kapsülü (yukarıda #1) | designer + gameplay-developer | Vekil model ve fizik varlığı |
| 2 | Kedge tutuşu ve reload'u kesip kesmediği | designer | Sol el animasyon seti, Gaffer reload'unun kesilebilirliği |
| 3 | Reload'da mermi zamanlayıcıyla mı notify ile mi verilecek; reload iptal edilebilir mi | gameplay-developer | Notify'ların bağlayıcılığı; iptal varsa "yarıda bırakma" pozları |
| 4 | Ek soket adları ve `UNFWeaponData` animasyon alanları | gameplay-developer | Rig'e yazılacak adlar |
| 5 | FP kol/silah render yolu (FOV 70, gölge, dünya ile kesişme) | gameplay-developer | Kolun üst koldan kesilmesi, materyal fonksiyonu |
| 6 | Saldırı zamanlamasının sahibi: Behavior Tree mi notify mı | gameplay-developer | Animasyon uzunlukları kilitli mi |
| 7 | Scuttler bacakları isabet alır mı (kapsül dışına taşıyor) | designer | Fizik varlığına bacak gövdesi eklenir mi (sürüde maliyet) |
| 8 | Attendant alçak siperde lamba görünür kalır mı | designer | `Cover_Idle_Low` pozu; görünürse "siperden çıkabilir misin?" sorusu yumuşar |
| 9 | Servis sırasında lamba göstergesi | designer | Lamba materyal durumları |
| 10 | Wardens koyu kütle dağılımı (§4.1) ve Drift koşulları (§4.2) | environment-artist ile ortak, gri tonlama testinde | Model oranları |
| 11 | Drift turkuazının oynanış renk tablosuna girmesi | environment-artist + GM | VFX renk ayrımı |
| 12 | Sandık taşırken ekranda ne görünür (Strongbox kadrajda mı, sol kol nerede) | designer | `Carry_*` pozları |
| 13 | Sol bilekte gelgit saati (HUD'un diegetik sürümü) — yalnız rezerv bıraktım | designer + GM | Kol UV'sinde yer |
| 14 | Kullanıcının Fab kütüphanesi listesi; prototipte hangi hazır paketler | GM + kullanıcı | Aşama A4 |
| 15 | Blender ve Unreal MCP bağlantısı | kullanıcı | Bu belgedeki hiçbir varlık bağlantı olmadan üretilemez; §0 hat doğrulaması bekliyor |
