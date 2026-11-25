# EBBSHIFT — Stil Rehberi (ortam ve görsel kimlik)

**Hazırlayan:** environment-artist · **Tarih:** 25 Kasım 2026 · **Durum:** Taslak v1 — designer ve developer onayı bekleyen maddeler §13'te
**Girdi:** [game-story.md](../design/game-story.md) §2, §3, §9.1, §9.4 · **Sözleşme:** `fps-art-pipeline`, `fps-level-design`

> Bu belgedeki sayılar başlangıç önerisidir. Hiçbiri motor içinde ölçülmedi: UE5 sahnesi, hedef donanım testi ve gri tonlama ekran görüntüsü henüz yok. L* değerleri hex kodlarından hesaplanmış albedo açıklığıdır, oyun içi ölçüm değildir.

---

## 1. Görsel hedef

Kapalı bir Atlantik gününde, sisin içinde duran **boyalı maket gibi** bir 1970'ler tatil kasabası: nane yeşili, şeker pembesi ve krem cepheler orta-açık tonda, düz ve temiz okunur; üstlerine sekiz yılın yosunu, midyesi, pası ve tuzu **yatay bantlar** halinde oturur. Ekran görüntüsünü tanıtan üç şey: (1) soluk pastel + koyu ıslak alt bant ikiliği, (2) her duvarda aynı yükseklikte duran üç su çizgisi, (3) bu soluk dünyada parlayan çok az sayıda doygun renk — hepsi oynanış bilgisi. Oranlar hafif abartılı (kalın pervaz, iri tabela, tombul korkuluk), yüzeyler sade, fotogerçekçilik yok. Oyuncu bir bakışta şunu okur: su nereye kadar çıkacak, yukarı çıkış nerede, düşman nerede.

**Okuma önceliği (değişmez):** düşman > etkileşimli nesne > yol/çıkış > su çizgisi > arka plan.

---

## 2. Palet

### 2.1 Ortam pastelleri (büyük yüzeyler)

| Ad | Hex | L* | Kullanım |
|---|---|---|---|
| Nane yeşili | `#A8CDB8` | 79 | Cephe sıvası, salon iç duvarı |
| Şeker pembesi | `#E3B1B4` | 77 | Cephe sıvası, otel |
| Kirli krem | `#CFC5AA` | 80 | Cephe, parapet, pervaz |
| Soluk gök mavisi | `#A9C3CF` | 77 | Üst kat cephe, kulübe |
| Tereyağı sarısı | `#DBCB94` | 82 | Tente şeridi, kiosk (az) |
| Kum | `#C9B98F` | 76 | Kumsal, kum birikintisi |
| Kaldırım betonu | `#9A9A92` | 63 | Promenade zemini |
| İskele ahşabı | `#7A6A58` | 46 | Döşeme, iskele ayağı |
| Pirinç | `#A8873F` | 58 | Korkuluk, tabela çerçevesi, armatür |
| 1970'ler turuncu-kahve | `#B5693A` | 52 | Plastik koltuk, tabela, makine gövdesi |
| Hardal | `#C29A3B` | 66 | Plastik/formika vurgu |
| Çikolata kahve | `#5E4332` | 31 | Ahşap kaplama, kapı, makine paneli |

Cephe pastelleri tek bir açık-gri sıva dokusunun **materyal instance renk çarpanıdır**; her renk için ayrı doku yapılmaz (§4).

### 2.2 Yıpranma tonları

| Ad | Hex | L* | Nerede |
|---|---|---|---|
| Tuz lekesi | `#DAD8CC` | 86 | Yalnız ince çizgi ve akıntı izi; yüzeyin %5'inden az |
| Pas | `#8A4B2A` | 39 | Metal bağlantılar, akıntı izleri |
| Yosun (kuru) | `#4E6B3E` | 42 | Half–High arası bant |
| Yosun (ıslak) | `#3A5241` | 33 | Low–Half arası bant |
| Midye/kabuk | `#6B6E73` / `#3F4347` | 46 / 28 | Low bandı, iskele ayakları |
| Zift / ıslak ahşap | `#4A4038` | 28 | İskele altı, kazık dipleri |

### 2.3 Su, sis, gök

| Ad | Hex | L* | Not |
|---|---|---|---|
| Su — sığ (0–50 cm) | `#9DB8A8` | 72 | Zemin görünür; hafif yeşil-gri |
| Su — orta (50–150 cm) | `#6E8F86` | 57 | Zemin silinir |
| Su — derin (150 cm+) | `#2C4A4A` | 29 | Opak; içi görünmez |
| Köpük / kıyı çizgisi | `#E8EFEA` | 94 | Su ile yüzey kesişiminde ince bant |
| Sis | `#AEB5B3` | 73 | Bilinçli olarak ortam bandının ortasında (§8) |
| Gök kubbe | `#C4CBCB` | 81 | Düz kapalı hava; bulut detayı çok az |

### 2.4 Oynanış renk dili (oyun boyunca sabit)

Bu renkler ortam yüzeyinde **kullanılmaz**. Ortamda doygunluk üst sınırı yaklaşık %55; bu tablodakiler bu sınırın üstündedir ve çoğu emissive'dir.

| Anlam | Hex | L* | İkinci işaret (renk körü güvencesi) | Örnek |
|---|---|---|---|---|
| **Tehlike** | `#E63B2E` | 52 | Çapraz şerit deseni, yanıp sönme | Warden lambası (düşmanca), elektrikli su uyarısı, asit birikintisi kenarı |
| **Etkileşim** | `#FFC62B` | 83 | Dolu boyalı tutamak + ikon | Vinç kancası, savak çarkı, kapı kolu, tırmanma halatı |
| **Ganimet** | `#B45CFF` | 57 | Mühür bandı + Fathom & Lyle damgası | Sandık bandı, taşınabilir değerli eşya |
| **Can / iyileşme** | `#4FD66A` | 77 | Artı/halat düğümü ikonu | Patch Kit, HUD can çubuğu |
| **Elektrik** | `#CFF4FF` | 94 | Zikzak hareket | Storm Lamp arkı, kısa devre |
| **Su saati (HUD)** | `#6E8F86` → `#E63B2E` | — | Dolan çubuk + çan sesi | Tide table; flood'a 60 sn kala tehlike rengine döner |

### 2.5 Fraksiyon imzaları

| Fraksiyon | Gövde | Koyu/vurgu | Işık | Değer bandı |
|---|---|---|---|---|
| **Wardens** | Krem emaye `#F4EEDD` (L* 94) | Koyu şişe yeşili bakalit `#23302B` (L* 19), pirinç `#B08D3C` | Lamba: sakin `#FFF4D6`, düşmanca = Tehlike kırmızısı | Sahnedeki **en açık** büyük şekil + en koyu eklem: iç kontrast yüksek |
| **The Drift** | Islak yeşil-siyah `#16241F` (L* 13), kabuk `#2A3A33` (L* 23) | — | İçten turkuaz `#3FE0D0` (emissive) | Sahnedeki **en koyu** büyük şekil |
| **The Kept** (1.0) | Sonra tanımlanacak | — | — | Öneri: orta-koyu bakır/kanvas + başlık lambası; değer bandı 1.0 öncesi ayrıca test edilecek |

Wardens gövde dağılımı (krem üst gövde, koyu uzuv/iskelet) bu rehberin `weapon-character-artist`'e **önerisidir**; karar onda (§13).

---

## 3. Şekil dili

| Katman | Dil | Örnek |
|---|---|---|
| Kasaba (insan yapısı) | Yatay ve dikdörtgen; yuvarlatılmış köşe, kalın pervaz, tekrar eden akslar; 1970'ler "yumuşak kutu" | Cephe aksı 400 cm, kemerli giriş, iri harfli tabela |
| Wardens | Dikey, simetrik, daire + silindir; ince bel, belirgin baş lambası | Dik duran figür, jeton yuvası göğüste |
| The Drift | Alçak, geniş, asimetrik, sivri ve kümeli; yere yakın | Kabuk dikenleri, düzensiz bacak ritmi |
| Su ve yıpranma | **Kesin yatay** çizgiler (su çizgisi) + aşağı akan dikey izler | Yosun bandı, pas akıntısı |
| Oynanış nesneleri | Tek büyük basit form + tek doygun renk lekesi | Sarı çark, mor bantlı sandık |

Kurallar:
- Mimari **hiç sivri değildir**; sivri bir siluet her zaman canlıdır (Drift). Dikey ve simetrik hareketli siluet her zaman makinedir (Wardens).
- Abartı oranı: pervaz, korkuluk, tabela ve kapı kolu gibi ikincil öğeler gerçek ölçünün ~1,3–1,5 katı kalınlıkta. Oynanış ölçüleri (kapı, siper, geçit) **abartılmaz**; `fps-level-design` ölçüleri geçerlidir.
- Küçük ölçekli gürültü yok. Detay üç ölçekte: büyük form (aks), orta (pervaz, tabela, bant), küçük (yalnız yakın temas noktalarında). 10 m'den seçilmeyen detay modellenmez.

---

## 4. Materyal karakteri ve üretim yöntemi

**Stilizasyon düzeyi: "boyalı maket" (PBR-sade).** Düz, az gürültülü albedo; geniş ve yumuşak roughness farkları; normal map yalnız trim sheet ve tiling dokularda. Ortam prop'larında yüksek poligon bake **yok**: pah (bevel) + weighted normal ile orta poligon modelleme. Islak yüzeyler parlak ve koyu, kuru boya mat ve açık — materyal okuması esas olarak bu ikilikten gelir.

### 4.1 Karar: tiling + trim sheet + vertex paint, tek master materyal

| Konu | Karar |
|---|---|
| Ana yöntem | Tiling doku (büyük yüzey) + kit başına 1 trim sheet (pervaz, korniş, korkuluk, çerçeve) + vertex paint ile yıpranma. Prop'a özel doku yalnız hero varlıklarda |
| Renk çeşitliliği | Nötr açık sıva dokusu × `BodyColor` instance parametresi. Bir doku seti tüm pastelleri verir |
| Master materyal | `M_Env_Master` (tiling/trim, renk çarpanı, vertex paint katmanları, gelgit ıslaklığı). Her varlık `MI_` instance kullanır; varlığa özel grafik yok |
| Vertex paint kanalları | R = kir/tuz akıntısı · G = boya soyulması (alt kat/pas görünür) · B = ek yosun/midye · A = boş (ayrılmış) |
| Gelgit ıslaklığı | `MF_TideWetness`: dünya Z'si güncel su yüksekliğinin altında veya az üstünde olan pikseller koyulaşır ve roughness düşer. Su yüksekliği `MPC_Tide` üzerinden gelir (developer ile, §13) |
| Su çizgisi bantları | `MF_Tideline`: üç sabit dünya Z'sinde yosun/tuz bandı, gürültü maskesiyle kırılmış. Elle dekal yerleştirme yalnız hero noktalarda |
| Dekal | `M_Decal_Master` + tek atlas: gelgit cetveli işaretleri, tebeşir yazıları (Kept), pas akıntısı, tabela yazıları |
| Texel density | **512 px/m** ortam standardı · **1024 px/m** elle tutulan/etkileşimli hero (vinç, çark, sandık) · 256 px/m yalnız uzak landmark |
| Doku boyutları | Tiling 2048² (4 × 4 m kaplar) · trim sheet 2048² (4 m uzunluk) · hero prop 1024²–2048² · maske/dekal atlası 2048² · 4096² ortamda yok |
| Doku seti | `_D` (sRGB), `_N`, `_ORM` (R=AO, G=Roughness, B=Metallic; lineer) |
| Çarpışma | Her kit parçasında `UCX_` kutuları; oynanış yüzeyinde karmaşık çarpışma yok |

### 4.2 Nanite ve Lumen (öneri; ölçüm `systems-developer` ile)

Hedef: orta sınıf GPU + Steam Deck. Aşağıdakiler **ölçülmemiş** başlangıç kararlarıdır.

| Konu | Öneri | Gerekçe |
|---|---|---|
| Nanite | Dikey dilimde **kapalı** varsay; klasik LOD (LOD0–2) + instancing. Deck'te ölçüldükten sonra yalnız yoğun hero varlıklar için yeniden değerlendir | Kit parçaları zaten düşük poligon; Nanite'ın kazancı yoğun geometride. Stil yoğun geometri istemiyor |
| Lumen | Işık kurgusu **Lumen kapalıyken de doğru** okunacak. Yüksek ayarda Lumen GI açık; düşük ayar/Deck'te kapalı (gök ışığı + ortam kapatma) | Kapalı hava = gök ışığı baskın, yön ışığı zayıf: GI'sız görüntü stile yakın kalır |
| Baked ışık | Varsayılan değil. Run'lar arası değişen yerleşim ve 2 kişilik ekip için lightmap bakımı pahalı. Dinamik çözüm bütçeyi tutmazsa yeniden açılır | — |
| Su | Tek düzlem mesh + tek materyal (`M_Water_Tide`); Water eklentisi yok. Kırılma/yansıma kalitesi ayara bağlı ölçeklenir | Tasarım "tek düzlem" diyor; en ucuz doğru çözüm |
| Hacimsel sis | Yüksek ayarda; Deck'te kapalı, yerine exponential height fog | Büyük yarı saydam hacim en pahalı kalem |
| Yarı saydamlık | Cam önceden parçalanmış ve çoğunlukla opak/maskeli; büyük saydam yüzeyler üst üste bindirilmez | Overdraw |

---

## 5. Su düzlemi ve su çizgisi dili

Su tek düzlemdir ve bir **saattir**; güzel olmasından önce okunaklı olması gerekir.

### 5.1 Su düzlemi

- **Derinlik = renk.** Sığ `#9DB8A8` → orta `#6E8F86` → derin `#2C4A4A`. Geçiş eşikleri oynanış durumlarına bağlanır (designer'ın diz / bel / göğüs / baş üstü eşikleri kesinleşince, §13): diz altı sığ, bel–göğüs orta, baş üstü derin ve **opak**.
- **Kıyı çizgisi:** suyun duvar, zemin ve nesneyle kesiştiği yerde 5–10 cm açık köpük bandı. Bu çizgi suyun o anki yüksekliğini her yüzeyde gösterir; kapatılamaz bir oynanış göstergesidir.
- **Yön:** ebb'de köpük ve yüzey deseni denize, flood'da karaya doğru akar. Hareketin yönü evreyi söyler.
- **Yüzey:** kapalı gök yansıdığı için su **açık** değerdedir; suyun içindeki Drift koyu siluet olarak okunur (§9). Dalga yüksekliği düşük; yüzeyde gürültü az.
- Suyun altı sis gibi çabuk kapanır (2–3 m görüş). Undertow'un V izi ve Leadfoot'un kabarcıkları yüzeyde, köpük renginde çizilir.

### 5.2 Üç su çizgisi (Low / Half / High)

Her çizgi iki katmandan oluşur: **doğal** (yosun, midye, tuz — `MF_Tideline`) ve **kurumsal** (L&C'nin emaye gelgit cetveli: duvarlara, direklere ve merdiven başlarına basılmış işaret — dekal).

| Çizgi | Doğal katman | Cetvel işareti (renkten bağımsız) | Altındaki yüzey |
|---|---|---|---|
| **Low Water** | Yoğun midye + koyu ıslak yosun `#3A5241` | Tek kalın çizgi + `LW` | Sürekli ıslak: en koyu, en parlak; kum birikintisi |
| **Half Tide** | Yeşil yosun bandı `#4E6B3E`, üstünde ince tuz çizgisi | Çift çizgi + `HT` | Islak-kuru arası: lekeli, pas akıntılı |
| **High Water** | Yalnız tuz çizgisi `#DAD8CC` + soluk boya | Üç çizgi + `HW`, üstünde dolu bant | Bandın **üstü** temiz pastel: "burası kuru kalır" |

- Çizgiler **mutlak dünya yüksekliğindedir**: her duvarda, her direkte, iç mekânda aynı Z'de. Eğimli sokakta çizginin zemine değdiği yer suyun varacağı noktadır.
- Sonuç olarak kasaba üç değer katmanına ayrılır: altta koyu ve parlak, ortada lekeli, üstte açık ve mat. **Yukarı = açık = güvenli** tüm oyunun yön kuralıdır.
- Çıkış (vinç) noktası her zaman High çizgisinin üstünde, temiz pastel bölgede ve Etkileşim sarısıyla işaretli.
- Kit ızgarasına göre önerilen yükseklikler (designer onayı gerekir): Low kat zemini Z=0, Half kat zemini Z=400, High kat zemini Z=800; çizgiler bu kat zeminlerine göre konumlanır.
- Final (M9) sonrası için `MF_Tideline` yoğunluğu tek parametreyle azaltılabilir ("su çizgileri kuruyor").

---

## 6. Modüler kit ızgarası ve adlandırma

| Konu | Kural |
|---|---|
| Izgara | 50 / 100 / 200 / **400 cm**. Cephe aksı 400, kat yüksekliği 400 (döşeme üstünden döşeme üstüne), duvar ve döşeme kalınlığı 25 |
| Kapı boşluğu | 150 × 250 cm |
| Alçak siper | 110 cm (parapet). Tam siper 200 cm ve üstü. Arası (120–190) yapılmaz: okunaksız siper |
| Açık korkuluk | 110 cm; **siper değildir**, mermi geçirir. Siper olan parapet her zaman doludur |
| Pivot | Taban, min-X köşe; duvarın dış yüzü Y=0, kalınlık +Y (bina içi). Döşemenin üst yüzü Z=0. Kapı kanadı ayrı mesh, pivot menteşede |
| Birim/eksen | Blender Unit Scale 0.01 (1 BU = 1 cm), FBX `-Y Forward, Z Up`, Smoothing Face, tangent space açık |
| Statik mesh | `SM_<KIT>_<Parça>_<Genişlik>x<Yükseklik>_<Varyant>` — ör. `SM_PRM_WallDoor_400x400_A` |
| Kit kodları | `PRM` Promenade cephe · `PIER` iskele · `COM` ortak sistem varlıkları · `HUB` Patient Margaret |
| Doku | `T_<KIT>_<Ad>_<D\|N\|ORM>` — ör. `T_PRM_Render_D` |
| Materyal | `M_Env_Master`, `MI_PRM_Render_Mint`, `M_Water_Tide`, `M_Decal_Master` |
| Çarpışma | `UCX_<MeshAdı>_##` |

Siper yükseklikleri ve geçiş genişlikleri designer'ın onayladığı blockout'tan gelir; sanat aşamasında değişmez.

---

## 7. Işık ve hava

| Öğe | Kural |
|---|---|
| Gün | Kapalı gündüz, öğleye yakın; saat ve hava görev boyunca **sabit**. Değişen tek şey sudur |
| Ana ışık | Gök ışığı baskın. Yön ışığı zayıf ve yumuşak (yaklaşık 1:3 yön/gök); sert gölge yok |
| Biçim | Gölge zayıf olduğu için hacmi ortam kapatma ve değer katmanları (§5.2) verir |
| Sis | Yükseklik sisi; suya yakın yoğun, yukarıda seyrek. Çatışma mesafesinde (0–40 m) kontrast kaybı az; 120 m ötesi yalnız siluet. Uzak landmark'lar (dönme dolap, fener) sis içinde koyu siluet olarak her zaman görünür |
| Sis değeri | L* ~73: ortam bandının ortası. Krem Warden sisten açık, Drift sisten koyu kalır; arka plan sise karışırken düşman karışmaz |
| İç mekân | Tam karanlık yok. Her iç mekânda en az bir "hâlâ çalışıyor" ışığı (hikâye belgesi §9.4) yolu gösterir; çıkış yönü en aydınlık açıklıktır |
| Yönlendirme ışığı | Sıcak pratik ışık (tabela, salon lambası) = yol. Soğuk turkuaz ışıltı = Drift. Kırmızı = tehlike. Başka renkli ışık yok |
| Pozlama | Otomatik pozlama dar aralıkta; iç-dış geçişte körlük yok |
| Post-process | Hafif bloom; hafif "kartpostal" renk düzeltmesi (siyahlar tam siyaha inmez, ama değer bantları korunur). Motion blur, chromatic aberration, film grain varsayılan **kapalı** |
| Flood evresi | Işık değişmez; sis biraz yoğunlaşır ve su sesi yükselir. Kararma yok (okunabilirlik) |

---

## 8. Değer (gri tonlama) kuralı

Sahne üç değer bandına bölünür; düşmanlar ortamın kullanmadığı iki uçta durur.

| Bant | L* | Kim kullanır |
|---|---|---|
| Açık uç | 88–96 | Warden emayesi, köpük çizgisi, elektrik, lamba ışıkları. Ortamda yalnız ince çizgi (tuz, köpük) |
| Ortam — kuru | 60–82 | Pastel cepheler, zemin, kum, sis, gök |
| Ortam — ıslak | 28–50 | Su çizgisi altı, ıslak ahşap, yosun, derin su |
| Koyu uç | 8–24 | Drift gövdesi, Warden eklemleri. Ortamda büyük yüzey olarak **yok** |

Kurallar:
1. Ortamda 2 m²'den büyük hiçbir yüzey L* 88'in üstünde veya 26'nın altında olamaz.
2. Düşman ile arkasındaki büyük yüzey arasında en az **25 L*** fark. Drift ıslak bölgede (28–50) savaşır: fark alt sınırda kaldığı için Drift'in turkuaz iç ışıltısı ve suyun açık yüzeyi ikinci güvencedir.
3. Wardens yüksek zeminde, açık pastel önünde durur: krem gövde tek başına yetmez (pastel 77–82, emaye 94). Okumayı **koyu uzuvlar ve baş lambası** taşır; bu yüzden Warden iki tonludur (§2.5).
4. Her mekân teslim edilmeden önce gri tonlama testinden geçer: oynanış kamerasından 3 ekran görüntüsü (kuru / half / high su), doygunluk sıfır, 25% boyuta küçültülmüş. Her düşman arketipi ve çıkış noktası 1 saniyede bulunamıyorsa mekân geçmez.
5. Erişilebilirlik: oynanış renklerinin her biri ikinci bir işaret taşır (§2.4). Ayarlarda renk körü ön ayarları (tehlike/ganimet/can renkleri değiştirilebilir), isteğe bağlı düşman vurgusu, HUD ölçeği. Kırmızı–yeşil çiftine (tehlike–can) yalnız renkle güvenilmez.

Bu kural şu an kâğıt üzerindedir; ilk UE5 gri kutusunda test edilecektir.

---

## 9. Çevresel anlatım (her alan için üç katman)

| Katman | Görsel karşılık | Değer/renk kuralı |
|---|---|---|
| **1971 anı** | Yarım kalmış tek bir sahne: kurulu masa, açık kasa, kalkmamış otobüs | High çizgisinin üstünde, temiz ve soluk; oyuncunun durup baktığı sakin noktada |
| **Sekiz yıl** | Su çizgisi, midye, kum birikintisi, çökmüş tavan | Materyal fonksiyonu + vertex paint; elle modelleme en az |
| **Hâlâ çalışıyor** | Yanan tabela, dönen turnike, tannoy | Sahnedeki tek sıcak emissive; yol göstergesi olarak yerleştirilir |
| Kept izleri | Tebeşirle yazılmış gelgit saatleri, eşiklerde kurşun ağırlık | Dekal atlası; açıklanmaz, tekrar eder |

Gerçek marka, bayrak ve dinî simge yok; tabela yazıları kurgusal ve İngilizce.

---

## 10. HUD ve menü görsel dili (özet)

L&C broşürü ve liman cetveli estetiği: krem zemin, koyu şişe yeşili yazı, tek kalın sans-serif, emaye tabela köşeleri. HUD köşelerde ve küçük: can, cephane, nişangâh, hasar yönü. **Tide table** ekranın üst ortasında ince yatay cetvel: üç eşik işareti (tek/çift/üç çizgi — dünyadaki cetvelle aynı), dolan çubuk, flood'a 60 sn kala tehlike rengi. Nişangâh özelleştirilebilir; HUD ölçeği, altyazı, renk körü ön ayarları EA çıkışında var.

---

## 11. Yapılacak / yapılmayacak

| Yap | Yapma |
|---|---|
| Büyük, düz, soluk renk alanları; detayı pervaz ve bantta topla | Her yüzeye gürültülü kir ve çizik dokusu |
| Yıpranmayı yatay bant ve dikey akıntı olarak yerleştir | Rastgele dağılmış leke; su çizgisini kıran dekor |
| Doygun rengi yalnız oynanış bilgisi için kullan | Ortamda Tehlike kırmızısı, Etkileşim sarısı veya Ganimet moru (tabela, tente, oyuncak dahil) |
| Yukarıyı açık ve temiz, aşağıyı koyu ve ıslak tut | Üst katta koyu, alt katta açık büyük yüzey |
| Siperi 110 veya 200+ cm yap; dolu ve kutu gibi okunur olsun | 120–190 cm arası "belirsiz" siper; açık korkuluğu siper gibi göstermek |
| Trim sheet ve tiling'den üret; rengi instance'tan ver | Prop başına özel doku seti; 4K ortam dokusu |
| Mimariyi yuvarlak köşeli ve yatay tut | Sivri, organik mimari (Drift siluetiyle karışır) |
| İç mekâna en az bir çalışan ışık koy | "Atmosfer" için tam karanlık koridor |
| Hazır varlık kullanırsan lisansı varlık listesine yaz | Lisansı belirsiz model/doku; gerçek marka ve logo |
| Fotoğrafı referans olarak kullan | Fotoğraf dokusu (photoscan) yüzeye basmak; fotogerçekçi su |

**Referans panosu (toplanacak; bu belgeye görsel eklenmedi):** 1960–70'ler İngiliz sahil kartpostalları (doygun baskı renkleri, soluklaşmış hâlleri) · sezon dışı sahil kasabası fotoğrafları (kapalı hava, boş promenade) · gelgit cetvelleri ve liman duvarlarındaki su izleri · 1970'ler formika, bakalit ve emaye ürün tasarımı · jetonlu oyun salonu makineleri. Referanslar ilham içindir; birebir bina veya tabela kopyalanmaz.

---

## 12. Promenade dikey dilimi — öncelikli varlık listesi

Üçgen sayıları LOD0 tahminidir. **P0** = prototip gri kutusu için gerekli (ilk geçiş kalitesi yeter) · **P1** = dikey dilim · **P2** = dikey dilimden sonra.

### 12.1 Modüler kit ve sistem varlıkları

| Parça | Ad | Üçgen (tahmin) | Doku | Öncelik |
|---|---|---|---|---|
| Dolu duvar 400 | `SM_PRM_Wall_400x400_A` | 200–600 | Tiling + trim | P0 |
| Dolu duvar 200 | `SM_PRM_Wall_200x400_A` | 150–400 | Tiling + trim | P0 |
| Kapılı duvar | `SM_PRM_WallDoor_400x400_A` | 600–1.200 | Tiling + trim | P0 |
| Dış / iç köşe | `SM_PRM_WallCornerOut_25x400_A`, `SM_PRM_WallCornerIn_25x400_A` | 100–300 | Trim | P0 |
| Zemin plağı | `SM_PRM_Floor_400x400_A`, `SM_PRM_Floor_200x200_A` | 12–100 | Tiling | P0 |
| Kat merdiveni (400 cm çıkış) | `SM_PRM_Stairs_200x400_A` | 500–1.000 | Tiling + trim | P0 |
| Parapet (alçak siper) | `SM_PRM_Parapet_400_A`, `SM_PRM_Parapet_200_A` | 100–300 | Trim | P0 |
| Su düzlemi | `SM_COM_WaterPlane_A` + `M_Water_Tide` | 2–2.000 | Normal 1024², gürültü 512² | P0 |
| Ganimet sandığı (orta) | `SM_COM_Crate_M_A` | 600–1.200 | 2048² ortak sandık atlası | P0 |
| Vinç noktası | `SM_COM_WinchPoint_A` | 4.000–6.000 | 2048² (1024 px/m) | P0 gri kutu · P1 son hâl |
| Su çizgisi sistemi | `MF_Tideline`, `MF_TideWetness`, `T_COM_TideMasks_ORM` | — | Maske 1024² | P0 basit · P1 son hâl |
| Pencereli duvar | `SM_PRM_WallWindow_400x400_A` | 800–1.500 | Tiling + trim | P1 |
| Dükkân/salon vitrini | `SM_PRM_WallShopfront_400x400_A` | 1.500–2.500 | Tiling + trim | P1 |
| Açık korkuluk | `SM_PRM_Railing_400_A` | 300–800 | Trim | P1 |
| Balkon / teras modülü | `SM_PRM_Balcony_400x200_A` | 1.000–2.000 | Trim | P1 |
| Korniş / çatı kenarı | `SM_PRM_Cornice_400_A` | 200–400 | Trim | P1 |
| Tente | `SM_PRM_Awning_400_A` | 300–600 | Trim + renk parametresi | P1 |
| İskele döşemesi | `SM_PIER_Deck_400x400_A` | 200–600 | Tiling ahşap | P1 |
| İskele ayağı | `SM_PIER_Pile_100x800_A` | 300–600 | Tiling ahşap + midye maskesi | P1 |
| İskele merdiveni | `SM_PIER_Stairs_200x400_A` | 600–1.200 | Tiling ahşap + trim | P1 |
| Gelgit cetveli | `SM_COM_TideGauge_A` + dekal atlası `T_COM_Decals_D` | 200–400 | Atlas 2048² | P1 |
| Sandık küçük / büyük | `SM_COM_Crate_S_A`, `SM_COM_Crate_L_A` | 400–1.500 | Ortak sandık atlası | P1 |
| Tannoy direği | `SM_COM_TannoyPole_A` | 1.500–2.500 | 1024² | P1 |
| Çatı | `SM_PRM_Roof_400x400_A` | 200–600 | Tiling | P2 |
| Savak kapısı + çark | `SM_COM_SluiceGate_A`, `SM_COM_SluiceWheel_A` | 3.000–5.000 | 2048² | P2 (M2'de gerekli) |

### 12.2 Hero ve dekor

| Parça | Ad | Üçgen (tahmin) | Doku | Öncelik |
|---|---|---|---|---|
| Jetonlu salon cephesi | `SM_PRM_ArcadeFront_800x400_A` | 6.000–10.000 | Trim + 2048² tabela | P1 |
| Kiosk (tam siper) | `SM_PRM_Kiosk_A` | 2.000–3.000 | Trim + renk parametresi | P1 |
| Oyun makinesi (2 varyant) | `SM_PRM_ArcadeCabinet_A`, `_B` | 800–1.500 | 2048² ortak atlas | P1 |
| Bank (alçak siper) | `SM_PRM_Bench_200_A` | 400–800 | Trim | P1 |
| Dönme dolap — uzak siluet | `SM_PRM_FerrisWheelFar_A` | 2.000–4.000 | 512² | P1 |
| Dönme dolap — oynanabilir | `SM_PRM_FerrisWheel_A` | 15.000–25.000 | 2048² + trim | P2 (M1 set-piece) |
| Kurulu masa seti ("1971 anı") | `SM_PRM_TableSet_A` | 1.200–2.000 | 1024² | P2 |
| Midye / yosun kümesi | `SM_COM_BarnacleCluster_A`–`C` | 200–500 | Maske atlası | P2 |
| Kum birikintisi | `SM_COM_SandDrift_A`–`C` | 100–300 | Tiling kum | P2 |

### 12.3 Doku setleri (dikey dilim)

| Set | Boyut | Not |
|---|---|---|
| `T_PRM_Render_D/N/ORM` | 2048² | Nötr sıva; tüm pasteller bundan |
| `T_PRM_Trim_D/N/ORM` | 2048² | Pervaz, korniş, korkuluk, çerçeve, tente şeridi |
| `T_PRM_Paving_D/N/ORM` | 2048² | Promenade zemini |
| `T_PIER_Wood_D/N/ORM` | 2048² | İskele ve ahşap |
| `T_COM_Sand_D/N/ORM` | 1024² | Kumsal, birikinti |
| `T_COM_TideMasks_ORM` | 1024² | Yosun / midye / tuz maskeleri (paketli) |
| `T_COM_Decals_D/N/ORM` | 2048² | Cetvel işaretleri, tebeşir, pas akıntısı, tabela |
| `T_COM_Crates_D/N/ORM` | 2048² | Üç sandık boyu tek atlas |
| `T_COM_Water_N` | 1024² | Su normali + gürültü |

Dikey dilim için toplam: 9 doku seti, 2 kişilik ekipte ortam sanatçısının üretim yükünü belirleyen asıl kalem budur.

---

## 13. Açık noktalar ve beklenenler

| Kimden | Ne | Neden |
|---|---|---|
| game-designer | Low / Half / High su çizgilerinin mutlak yükseklikleri ve kat zeminleri (öneri: 0 / 400 / 800) | `MF_Tideline` ve kit kat yüksekliği buna bağlı |
| game-designer | Islaklık eşikleri (diz / bel / göğüs / baş üstü) cm olarak | Su renk geçişi bu eşiklere bağlanacak |
| game-designer + gameplay-developer | Alçak siper yüksekliği teyidi. `NFCharacter.cpp` taslağında çömelmiş kapsülün üstü 120 cm, çömelmiş göz 100 cm; 110 cm parapet kapsül üstünün 10 cm altında kalıyor | Siper yüksekliğini ben değiştirmiyorum; ya siper ya çömelme yüksekliği kararı gerekli |
| game-designer | Açık korkuluk mermi geçirir / dolu parapet siperdir ayrımının onayı; kiosk ve oyun makinesinin siper sınıfı (110 mu 200+ mı) | Varlık ölçüleri |
| gameplay-developer | Güncel su yüksekliğini materyale veren `MPC_Tide` (ör. `WaterHeight`, `TidePhase`); savakla geciken bölgeler için yerel su yüksekliğinin materyale nasıl ulaşacağı | Islaklık ve su çizgisi materyal tarafında çözülüyor; bölgesel gecikme tek küresel parametreyle gösterilemez |
| gameplay-developer | Açık korkuluk için "oyuncuyu durdurur, mermiyi geçirir" çarpışma kanalı | `UCX_` tek kutu şu an ikisini de durdurur |
| systems-developer | Hedef donanımda ölçüm: Nanite açık/kapalı, Lumen açık/kapalı, tek katman su maliyeti; sahne başına üçgen ve çizim çağrısı bütçesi | §4.2'deki öneriler ölçülmedi |
| weapon-character-artist | Wardens iki tonlu gövde (krem + koyu uzuv) ve Drift değer bandı önerisinin kabulü | §8 değer kuralı düşman tarafında da tutmalı |
| studio-gm | Ganimet rengi olarak mor (`#B45CFF`) dönem estetiğinin dışında bilinçli bir seçim; itiraz varsa alternatif gerekir | Renk dili oyun boyunca sabit kalacak |

---

## 14. Blender denemesi notu (25 Kasım 2026)

**Blender MCP bu oturumda canlı bir Blender'a bağlanmadı.** İki deneme yapıldı (`get_addon_status`, `get_scene_info`); ikisi de aynı hatayı döndü: `Could not connect to Blender at 127.0.0.1:9876`. Blender'da hiçbir şey kurulmadı, `look` ile kontrol yapılmadı, `generate_3d` kullanılmadı.

Yerine aynı seti kuran betik hazırlandı: [`tools/blender/promenade_kit_blockout.py`](../../../tools/blender/promenade_kit_blockout.py)

| Parça | Boyut (cm) | Üçgen | Not |
|---|---|---|---|
| `SM_PRM_Wall_400x400_A` | 400 × 25 × 400 | 12 | Tam siper |
| `SM_PRM_WallDoor_400x400_A` | 400 × 25 × 400 | 44 | Kapı boşluğu 150 × 250, 3 `UCX_` kutusu |
| `SM_PRM_Floor_400x400_A` | 400 × 400 × 25 | 12 | Üst yüz Z=0 |
| `SM_PRM_Parapet_400_A` | 400 × 25 × 110 | 12 | Alçak siper |
| `SM_PRM_Railing_400_A` | 400 × 10 × 110 | 168 | Açık korkuluk; siper değil |

Betik ayrıca: yeni bir sahne açar (mevcut sahneye dokunmaz), birimi 1 BU = 1 cm yapar, §2.1 palet renklerinden yer tutucu materyaller üretir, 3 akslı 2 katlı örnek bir cephe ve oyuncu kapsülü ölçüsünde (80 cm çap, 192 cm boy; göz 160, çömelmiş üst 120) bir ölçek mankeni kurar, her parçayı `UCX_` çarpışmasıyla birlikte ayrı FBX olarak yazar (`-Y Forward, Z Up`, ölçek 1.0, `FBX_SCALE_NONE`, Smoothing Face, tangent space).

**Doğrulama durumu:**
- Geometri üretimi Blender'sız, saf Python ile test edildi (`python3 tools/blender/promenade_kit_blockout.py --selftest`): beş parça da kapalı (manifold), ölçüler ve hacimler beklenen değerde.
- Betiğin Blender'a bağlı kısmı (sahne, materyal, UV, FBX export) **burada çalıştırılmadı**; yalnızca sözdizimi derlemesinden geçti. Blender 4.x/5.x API'sine göre yazıldı.
- FBX'in UE5'e doğru ölçek ve yönde girdiği doğrulanmadı. İlk çalıştırmada sözleşmedeki uçtan uca kontrol (Blender → FBX → UE → ölçek mankeni) yapılmalı.

Çalıştırma: `blender -b -P tools/blender/promenade_kit_blockout.py -- --out <klasör>` veya Blender MCP bağlandığında betiğin içeriği `execute_blender_code` ile.
