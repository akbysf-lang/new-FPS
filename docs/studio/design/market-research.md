# FPS Pazar Araştırması ve Konsept Önerisi

**Hazırlayan:** game-designer · **Tarih:** 25 Kasım 2026 · **Durum:** Kullanıcı kararı bekliyor (konsept seçimi)

> **Yöntem ve sınırlar — önce bunu okuyun**
>
> - Web araması (WebSearch) **çalıştı**; bu rapordaki rakamlar 25 Kasım 2026'da yapılan aramalardan geliyor.
> - Sayfa okuma (WebFetch) **çalışmadı**: `newsletter.gamediscover.co`, `howtomarketagame.com` ve `www.pcgamer.com` bu oturumun izinli ağ alanlarının dışında. Bu yüzden rakamlar kaynak sayfaların tamamı okunarak değil, **arama sonuç özetlerinden** alındı. Yatırım kararı öncesi kritik rakamların (aşağıda ★ ile işaretli) kaynak sayfadan bir kez daha teyit edilmesi gerekir. Tam okuma için bu üç alanın oturum ağ ayarlarında açılması yeterli.
> - SteamDB / Gamalytic / VG Insights panellerine doğrudan erişilemedi. Üçüncü taraf "tahmini satış" rakamları ikincil sitelerden geldi ve tabloda **tahmin** diye etiketli.
> - "Bulunamadı" yazan hücreler aranmış ama doğrulanabilir veri çıkmamış hücrelerdir; boşluk doldurulmadı.
> - Köşeli parantezli numaralar ([K1] vb.) bölüm 10'daki kaynakçaya gider.

---

## 1. Yönetici özeti

1. **Rekabetçi/canlı servis çok oyunculu nişancı 2026'da küçük ekip için kapalı kapı.** Highguard çıkıştan 45 gün sonra, The Cube, Save Us üç haftada, PUBG: Blindspot iki ayda kapandı; Last Flag, Spectre Divide, Seekers of Skyveil, Level Zero: Extraction aynı yolu izledi [K14–K21]. Bunların çoğu bizden kat kat büyük bütçeli ekiplerdi.
2. **Güçlü kancası olan premium co-op PvE FPS, bugün küçük ekibin en iyi kanıtlanmış yolu.** Far Far West (yaklaşık 7 kişilik ekip, 19,99 $, Erken Erişim) iki haftadan kısa sürede 500 binin üstünde sattı; yayıncıya göre 1 milyonu geçti [K3–K7]. Jump Space (19,99 $) 900 binin üstünde [K11]; Deadzone: Rogue ilk iki haftada 150 bin [K9]; SULFUR 200 bin [K10].
3. **Ama co-op etiketi tek başına satmıyor.** FBC: Firebreak (Remedy) Steam'de 2.000 eşzamanlı oyuncunun altında kaldı, stüdyo satıştan "memnun olmadığını" açıkladı [K22]; Killing Floor 3 %55 olumlu ile açıldı ve oyuncusunu hızla kaybetti [K23]. Ortak şikâyet: kimliksizlik, zayıf ilk saat, tekrar eden görev.
4. **Retro/boomer shooter arzı çok hızlı büyüyor.** Steam'de "Boomer Shooter" etiketli çıkış 2024'te 169, 2025'te 232, 2026'da Eylül sonuna kadar 204; ayrıca 548 yayınlanmamış sayfa bekliyor [K24]. Tek oyunculu kampanya hâlâ satabiliyor (MOUSE: P.I. For Hire 1 milyon [K8]) ama bunu olağanüstü bir görsel kanca ve çok platformlu çıkışla yaptı.
5. **Taban oranlar acımasız; karar wishlist verisine bağlanmalı.** 2025'te Steam'de çıkan 20.282 oyundan yalnızca 608'i (%3) 1.000 incelemeye ulaştı [K1]. Şubat 2026 Next Fest anketinde medyan kazanım 806 wishlist [K27]. Fiyatı 10 $ üstü oyunlarda ilk hafta satışı medyanda çıkış wishlist'inin ~0,10 katı [K29].

**Tek tavsiye:** **EBBSHIFT** (çalışma adı) — 1–4 kişilik, tek başına da oynanabilen, premium co-op PvE FPS. Kanca: **gelgit görevin saatidir**; deniz çekilince en değerli ganimet açığa çıkar, geri gelince alt katlar boğulur ve ekip yukarı doğru savaşarak çıkar. Steam Erken Erişim, 19,99 $, mikro ödeme yok, P2P (listen server) co-op. Ayrıntı bölüm 7'de; hikâye belgesi: [game-story.md](game-story.md).

---

## 2. Pazarın durumu (2025–2026)

### 2.1 Steam genel tablo

| Gösterge | Değer | Kaynak | Güven |
|---|---|---|---|
| 2025'te Steam'de çıkan oyun | 20.282 (VG Insights sayımı; SteamDB ~20.000) | [K1] | Orta (izleyiciler arası küçük fark var) |
| 1.000+ incelemeye ulaşan 2025 oyunu | 608 (%2,99; 2024'te %2,44) | [K1][K2] | Orta |
| 2025'te 100.000 $+ kazanan oyun (Valve, GDC) | 5.863 (2020'de 3.000'in biraz üstü) | [K30] | Yüksek (Valve açıklaması) |
| 2025 çıkışlarında medyan gelir | 249 $ (Gamalytic tahmini) | [K31] | Düşük–orta (tahmin yöntemi) |
| 2025 çıkışlarından 1 milyon $+ brüt | ~300 / ~20.000 (Alinea tahmini) | [K31] | Düşük–orta |
| İlk 30 günde 500 bin+ satan oyun (Eyl 2025–Ağu 2026) | 76 (önceki yıl 55); bağımsız yapım 33 (önceki yıl 13) | [K32] | Orta (Sensor Tower tahmini; PS+Steam+Xbox) |
| Ekim 2026 Next Fest'te demo sayısı | 7.240–8.000+ (kaynaklar farklı) | [K27][K28] | Orta |

**Okuma:** Pazarın tepesi bağımsızlara hiç olmadığı kadar açık (500 bin+ satan bağımsız oyun sayısı bir yılda 13'ten 33'e çıktı), ama taban aynı ölçüde kalabalık. "İyi oyun yaparsak satar" varsayımı veriyle desteklenmiyor; **ayırt edici kanca + erken wishlist ölçümü** şart.

### 2.2 FPS'e özgü gözlemler

- **Canlı servis nişancılarda "iki aylık ömür" dönemi.** 2026'nın ilk yarısında Highguard (zirvede ~100 bin eşzamanlı, 45 günde kapanış), The Cube, Save Us ve PUBG: Blindspot peş peşe kapandı [K14–K16]. Wildgate çıkışta 7.799 eşzamanlı zirveden günlük 25–150'ye düştü ve geliştirmesi sonlandırıldı [K19].
- **Premium co-op PvE'de küçük ekip başarıları.** Far Far West, Jump Space, Deadzone: Rogue, SULFUR, Abyssus, Mycopunk (ayrıntı bölüm 4). Ortak nokta: 14,99–24,99 $ fiyat, Erken Erişim, demo/playtest ile uzun ön ısınma, tek cümlelik kanca.
- **Büyük ölçekli taktik PvP'de tek istisna büyük bütçe ister.** WARDOGS (Bulkhead/Team17, 39,99 $, 100 oyuncu) Erken Erişim'de 1 milyonun üstünde satış bildirdi [K13]; bu bizim ölçeğimizde tekrarlanabilir bir örnek değil.
- **"Friendslop" (ucuz, arkadaşla oynanan co-op) dalgası sürüyor:** PEAK 10 milyon, R.E.P.O. 16,8–20 milyon [K33]. Bu oyunların çoğu FPS değil ama "arkadaş tavsiyesiyle yayılan ucuz co-op" talebini kanıtlıyor. Sensor Tower bu kümedeki 500 bin+ çıkışları 5'ten 9'a çıkmış sayıyor [K32].
- **İnceleme çarpanı:** güncel ikincil kaynaklar ~30× (aralık 20–60×; 40 $+ oyunlarda 15–25×) diyor [K34]. GameDiscoverCo/Gamalytic'in birincil 2026 rakamı **bulunamadı**. Bu rapordaki "inceleme × 30" hesapları **tahmindir**.

---

## 3. Alt tür haritası ve fırsat değerlendirmesi

Puanlar 1–5 (5 = bizim için en iyi). "Arz" sütununda 5 = az rekabet.

| Alt tür | Talep | Arz (5=az) | Giriş maliyeti (5=ucuz) | 6 kişiye uygunluk | Kanıt | Karar |
|---|---|---|---|---|---|---|
| **Co-op PvE (görev/horde)** | 5 | 3 | 3 | 4 | Far Far West, Jump Space başarı; FBC Firebreak, KF3 başarısız [K3–K7][K11][K22][K23] | **Birincil hedef** |
| **Roguelite FPS** | 4 | 3 | 4 | 5 | Deadzone: Rogue 150 bin/2 hafta, SULFUR 200 bin, Witchfire 500 bin+ [K9][K10][K12] | Co-op ile birleştir |
| Boomer/retro shooter (tek oyunculu) | 3 | 1 | 5 | 5 | 2025'te 232 çıkış, 548 sayfa sırada [K24]; Selaco 51 bin/2 hafta [K25]; Phantom Fury %62 karışık [K26] | Yedek seçenek |
| Stilize tek oyunculu kampanya | 4 | 3 | 2 | 2 | MOUSE: P.I. For Hire 1 milyon; çok platform, uzun geliştirme [K8] | Ekip için pahalı |
| Movement shooter | 3 | 3 | 4 | 4 | ULTRAKILL çok büyük ama üçüncü taraf tahminleri 3,5–9,5 milyon arasında dağınık [K35]; yeni çıkışlar için satış verisi **bulunamadı** | Tek başına riskli; hareket özelliği olarak al |
| Korku FPS / co-op korku | 3 | 2 | 4 | 4 | Trepang2 ~0,8–0,9 milyon (tahmin) [K36]; korku 2025'te 1.000+ incelemeli 39 oyunla üçüncü tür [K1] | Ton olarak ödünç alınabilir |
| Bodycam / fotogerçekçi | 3 | 3 | 1 | 1 | Bodycam Temmuz 2024'e kadar ~800 bin (ikincil kaynak); stüdyo 2'den 30 kişiye büyüdü [K37] | Görsel çıta 2 artist için çok yüksek |
| Taktik / milsim | 4 | 3 | 1 | 1 | Ready or Not 13 milyon [K38]; WARDOGS [K13] | İçerik ve AI yükü ağır |
| Extraction (PvPvE) | 3 | 1 | 1 | 1 | Level Zero, Seekers of Skyveil, The Cube kapanış [K16–K18] | **Girme** |
| Hero shooter / arena PvP / battle royale | 3 | 1 | 1 | 1 | Highguard, Spectre Divide, Last Flag, Blindspot [K14][K15][K20][K21] | **Girme** |
| Immersive sim | 2 | 4 | 1 | 1 | Güncel küçük ekip satış verisi **bulunamadı** | Kapsam dışı |

**Boşluk analizi (oyuncular ne istiyor, kim karşılamıyor):**

- Co-op PvE'de en sık şikâyet **tekrar eden görev ve az düşman çeşidi** (Far Far West'in %96 olumlu incelemelerinde bile en çok anılan eleştiri) [K7]. Çözüm içerik miktarı değil, **her görevi farklı oynatan sistemik bir kural**.
- Roguelite co-op'ta **zamanlayıcı** tartışmalı: Deep Rock Galactic: Rogue Core'un ilk gün incelemeleri (%70 olumlu) zamanlayıcıyı "seven ve nefret eden" diye ikiye bölündü [K39]. Soyut sayaç itici; **dünyada görünen, öngörülebilen, oyuncunun etkileyebildiği** bir baskı kaynağı boşluk.
- Başarısız büyük co-op'larda ortak teşhis: **kimlik yok** (KF3) ve **ilk saat zayıf** (Firebreak'te Remedy'nin kendi açıklaması: onboarding ve görev yapısı) [K22][K23].
- "Sular yükselir" fikri Steam'de var ama co-op FPS'te sahipsiz: TideFall (7,99 $, Temmuz 2026, "The tide is the timer" sloganlı küçük bir aksiyon oyunu), Flood Escape (ücretsiz), TIDEBREAK (hayatta kalma) [K40]. Hiçbiri görev tabanlı premium co-op nişancı değil. **Bu sloganı kullanmıyoruz.**

---

## 4. Rakip tablosu (başarılı ve başarısız)

Satış rakamları geliştirici/yayıncı açıklamasıysa "açıklama", üçüncü taraf modeliyse "tahmin" diye belirtildi. İnceleme sayıları anlık görüntüdür.

### 4.1 Başarılı

| Oyun | Alt tür | Çıkış | Fiyat | Ekip | Steam inceleme | Satış | Kanca / ders | Kaynak |
|---|---|---|---|---|---|---|---|---|
| Far Far West ★ | Co-op PvE FPS (1–4) | 28 Nis 2026 (EA) | 19,99 $ | ~7 kişi (PC Gamer); başka kaynak 8 diyor | 19.500+, %96 | 48 saatte 250 bin+; <2 haftada 500 bin+ (GameDiscoverCo başlığı, EG7); yayıncıya göre 2 haftada 1 milyon | "Büyücü robot kovboylar"; Şubat 2026 Next Fest'te ilk 5 demo, fest haftasında ~101 bin wishlist (Alinea tahmini) | [K3–K7] |
| Jump Space | Co-op PvE (1–4), uzay gemisi mürettebatı | 19 Eyl 2025 (EA) | 19,99 $ | Bulunamadı | Bulunamadı | İlk 3 haftada 430 bin+; toplam 900 bin+ (açıklama) | "Gemiyi birlikte kullan"; açık playtest'lerle uzun ısınma | [K11] |
| Deadzone: Rogue | Roguelite FPS, co-op | EA 2025 (ay doğrulanmadı); 1.0 Ağu 2025 | 24,99 $ | Bulunamadı (Hi-Rez kurucusu destekli) | "Çok Olumlu" | EA'nın ilk 2 haftasında 150 bin+ (açıklama) | Hızlı EA→1.0 döngüsü (~4 ay) | [K9] |
| SULFUR | Roguelite FPS (tek oyunculu) | EA 28 Eki 2024; 1.0 Eki 2026 (planlanan) | 19,99 $ | Bulunamadı | Bulunamadı | 200 bin+ (açıklama) | Stilize görsel + silah modifikasyonu; co-op'u satış eşiğine bağladı | [K10] |
| Witchfire | Roguelite/extraction-lite FPS (tek oyunculu) | EA 2023–2024 (hafızadan; doğrulanmadı) | Bulunamadı | Bulunamadı (The Astronauts) | Bulunamadı | 500 bin+ (açıklama, Ara 2025) | Çok yüksek görsel kalite; uzun EA | [K12] |
| Abyssus | Co-op roguelite FPS (1–4) | 12 Ağu 2025 | 24,99 $ | Bulunamadı | ~4.900–5.300, "Çok Olumlu" | Resmî rakam bulunamadı; inceleme × 30 ≈ 150 bin (**tahmin**) | "Brinepunk" denizaltı uygarlığı; tema olarak bize komşu | [K41] |
| Mycopunk | Co-op PvE FPS (1–4) | 10 Tem 2025 (EA) | 14,99 $ | Bulunamadı (ilk oyun; Devolver yayıncı) | 2.700–4.400, ~%89 | Resmî rakam yok; üçüncü taraf "270 bine kadar" (**tahmin**) | Düşük fiyat + mizah; zirve eşzamanlı 2.832 | [K42] |
| MOUSE: P.I. For Hire | Tek oyunculu stilize FPS | 16 Nis 2026 | 29,99 $ | Bulunamadı | ~14.400, %94 | 1 ayda ~730 bin (yarısı konsol), Tem 2026'da 1 milyon (açıklama) | 1930'lar çizgi film görseli; tek karede tanınır | [K8] |
| Selaco | Retro FPS | Haz 2024 (EA) | Bulunamadı | Bulunamadı | Bulunamadı | İlk 2 haftada ~51 bin (açıklama) | Nişte "iyi" sonucun ölçeği: on binler | [K25] |
| Bodycam | Fotogerçekçi PvP FPS | 7 Haz 2024 (EA) | Bulunamadı | 2 → 30 kişi | Bulunamadı | Tem 2024'e kadar ~800 bin (ikincil kaynak) | Görsel kanca tek başına satış getirir ama içerik borcu büyütür | [K37] |
| WARDOGS | 100 oyunculu taktik PvP | 10 Eyl 2026 (EA) | 39,99 $ | Bulunamadı (Bulkhead) | 36 bin+, %78–85 | 1–2 milyon (açıklama; kaynaklar farklı) | Kıyas dışı; bütçe ve sunucu ister | [K13] |
| Ready or Not | Taktik co-op FPS | PC Ara 2023; konsol Tem 2025 | Bulunamadı | Bulunamadı | Bulunamadı | 13 milyon (açıklama, Eyl 2025) | Uzun kuyruk; AI ve içerik yükü ağır | [K38] |

### 4.2 Başarısız veya beklentinin altında

| Oyun | Alt tür | Çıkış | Model | Sonuç | Tekrarlanan şikâyet / neden | Kaynak |
|---|---|---|---|---|---|---|
| Highguard | Çevrim içi PvP nişancı | 26 Oca 2026 | F2P | 12 Mar 2026'da kapandı (45 gün); zirve ~100 bin, ilk haftada ~%90 düşüş | Sürdürülebilir oyuncu tabanı kurulamadı | [K14] |
| The Cube, Save Us | Extraction | 18 Mar 2026 | F2P | 3 hafta sonra kapanış duyurusu; birkaç yüz oyuncu | Ücretsiz olmasına rağmen nüfus yok | [K16] |
| PUBG: Blindspot | Taktik PvP | 5 Şub 2026 (EA) | F2P | 30 Mar 2026'da kapandı | Nüfus | [K15] |
| Last Flag | 5v5 PvP | Bulunamadı | Bulunamadı | Destek sonlandırıldı | "Oyuncu sayımız olması gereken yerde değil" | [K20] |
| Spectre Divide | Taktik PvP | 2024 (hafızadan; doğrulanmadı) | F2P (hafızadan; doğrulanmadı) | Oyun ve stüdyo 6 ayda kapandı | Nüfus + gelir | [K21] |
| Wildgate | PvPvE | 2025 | Bulunamadı | Zirve 7.799 → günlük 25–150; geliştirme bitti | Eşleşme nüfusu eridi | [K19] |
| Level Zero: Extraction | Extraction korku | 1.0 2025 başı | Premium | 1.0'dan 2 hafta sonra geliştirme durdu | "Küçük ekibimiz için sürdürülemez" | [K17] |
| Seekers of Skyveil | MOBA-extraction | 2025 | F2P | 18 günde kapandı, stüdyo kapandı | Karışık inceleme, tutunma yok | [K18] |
| FBC: Firebreak | Co-op PvE FPS (3) | Haz 2025 | Premium + abonelik servisleri | Steam zirvesi 1.992; Remedy satıştan "memnun değil"; Steam satışı ~55 bin (**üçüncü taraf tahmini**) | Onboarding, görev yapısı, kimlik | [K22] |
| Killing Floor 3 | Co-op horde FPS | Tem 2025 | Premium | %55 olumlu açılış, sonradan %46; haftalar içinde KF2'nin altına düştü. Yine de ~730 bin kopya (**üçüncü taraf tahmini**) | "Kendi kimliğini kaybetmiş"; teknik sorunlar | [K23] |
| DRG: Rogue Core | Co-op roguelite FPS | 20 May 2026 (EA) | Premium, 29,99 $ | İlk gün ~2.000 inceleme, %70 olumlu; satış rakamı **bulunamadı** | Zamanlayıcı ve paylaşılan ilerleme tartışması | [K39] |
| Phantom Fury | Retro FPS | Nis 2024 | Premium | %62 olumlu (814 inceleme); satış **bulunamadı** | Hatalar, selefinin gerisinde | [K26] |
| Starlight Re:Volver | Co-op roguelite (FPS değil) | 2025 | — | 17 milyon $ yatırım; 3 ayda yarı ekip işten çıkarıldı, Şub 2026'da kapandı | Yatırım ≠ kitle | [K20] |

**Tablodan çıkan dersler**

1. Kapanan oyunların neredeyse hepsi **oyuncu nüfusuna bağımlı** (PvP, PvPvE, eşleşme zorunlu). Premium co-op PvE'de oyuncu sayısı düşse de oyun oynanabilir kalıyor; ürün "ölmüyor".
2. Başarılı co-op PvE oyunlarının fiyatı **14,99–24,99 $** bandında; hepsi Erken Erişim'le çıktı.
3. Başarının öncü göstergesi demo/playtest dönemi: Far Far West'in toplam wishlist'i için ikincil bir kaynak 536 bin diyor (**doğrulanamadı**) [K6].
4. Büyük marka + büyük bütçe co-op'u kurtarmıyor (Firebreak, KF3); **ilk saat ve kimlik** kurtarıyor.

---

## 5. Hedef kitle profili

**Birincil:** 18–35 yaş, KA + Batı/Kuzey Avrupa, PC (Steam), Discord'da 2–4 kişilik sabit arkadaş grubu; Deep Rock Galactic, Helldivers 2, Left 4 Dead geleneğinden gelen, "akşam iki görev atalım" oyuncusu. Satın alma kararı: arkadaş tavsiyesi + kısa klip + demo.

**İkincil:** Tek başına oynayan roguelite FPS oyuncusu (Deadzone: Rogue, SULFUR, Witchfire kitlesi). Bu yüzden oyun **tek kişiyle tam oynanabilir** olmalı; bot arkadaş şart değil, ölçekleme şart.

**Doğrulanan / doğrulanamayan varsayımlar**

| Varsayım (skill'deki başlangıç listesi) | Durum | Not |
|---|---|---|
| PC'de premium model kabul görür | **Doğrulandı** | Başarılı küçük ekip örneklerinin tamamı premium; kapanan F2P'ler çoğunlukta [K14–K18] |
| Demo ve Next Fest ana keşif kanalı | **Doğrulandı, uyarıyla** | Fest öncesi wishlist ile fest kazanımı arasındaki korelasyon r = 0,825 (ikincil kaynak) [K27]; demoyu fest'ten aylar önce yayınlayanlar ~2,5× medyan kazanmış. Yani fest bir **çarpan**, başlangıç değil |
| Co-op organik yayılır | **Doğrulandı** | Sensor Tower co-op/"friendslop" kümesi [K32]; Far Far West, Jump Space |
| Rekabetçi PvP nüfus riski | **Güçlü biçimde doğrulandı** | Bölüm 4.2 |
| Steam Deck / gamepad beklenir | **Kısmen** | Valve anketinde Deck payı ~%0,83; GameDiscoverCo anketinde aylık PC oyuncularının %17'si Deck kullanıyor (ikincil; farklı örneklem). Steam Machine 30 Haz 2026'da çıktı [K43]. Gamepad + Deck "Playable" hedeflenmeli, "Verified" 1.0'a bırakılabilir |
| Yerelleştirme sırası EN → DE, FR, ES, PT-BR | **Kısmen** | Steam dil payları: İngilizce %39,0 (Eki 2026), Basitleştirilmiş Çince %24,0 (Ağu 2026; aydan aya çok oynak), Rusça %9,9, İspanyolca %4,5 (Ağu 2026); Almanca %3,0, Fransızca %2,5 (Mar 2026, ikincil) [K44]. PT-BR payı **bulunamadı**. Öneri: EA'da yalnız İngilizce; 1.0'da DE/FR/ES/PT-BR + metin hacmi düşük olduğu için Basitleştirilmiş Çince **değerlendirilsin** |
| İlk 2 saat kritik (iade) | Doğrudan veri **bulunamadı** | Firebreak örneği dolaylı destek [K22] |

**Platform beklentileri (tasarıma girdi):** FOV 70–120, tuş atama, ultrawide, kilitsiz fps, gamepad + aim assist, altyazı, renk körü seçeneği, head-bob/shake kapatma, su efekti yoğunluğu ayarı (bu oyuna özgü: hareket hastalığı riski).

**Yaş derecelendirmesi hedefi:** ESRB T / PEGI 16. Düşmanların çoğu makine ve deniz canlısı; insan düşman sınırlı ve kansız/az kanlı sunulabilir. Almanya için özel kısıt beklenmiyor (**tahmin**; USK başvurusunda doğrulanır).

---

## 6. Aday konseptler ve puanlama

### Aday A — EBBSHIFT: gelgitli co-op PvE kurtarma nişancısı
1–4 oyuncu. Terk edilmiş bir sahil kasabasında enkaz kurtarma ekibisiniz. Gelgit her görevde çekilir ve geri gelir: en iyi ganimet en altta, çıkış en üstte. Su iki düşman fraksiyonunu ters yönde etkiler (deniz yaratıkları suda güçlenir, kasabanın otomat bekçileri suda kısa devre yapar), elektrikli silahlar suda zincirleme çarpar. Roguelite ilerleme hub'da.

### Aday B — Tek oyunculu, stilize retro FPS kampanyası
Aynı dünyada 6–8 saatlik el yapımı kampanya, hızlı hareket, gizli odalar, 10–12 silah. Çok oyunculu yok.

### Aday C — Tek oyunculu roguelite FPS (co-op sonradan)
Aynı gelgit kuralı, tek oyuncuya göre dengelenmiş kısa run'lar; co-op yalnızca satış eşiği aşılırsa eklenir (SULFUR modeli [K10]).

### Aday D — Küçük ölçekli PvP (ör. 3v3 gelgit arenası) — **elendi**
Bölüm 4.2'deki kanıt nedeniyle puanlamaya alınmadı: eşleşme nüfusu, sunucu, hile önleme ve canlı operasyon 2 developer'la sürdürülemez; aynı dönemde çok daha büyük ekipler aynı duvara çarptı.

| Ölçüt | A: Co-op gelgit | B: Retro kampanya | C: Solo roguelite | Gerekçe |
|---|---|---|---|---|
| Talep | 5 | 3 | 4 | Co-op PvE'de en yüksek satış kanıtı [K3–K11]; retro'da tek tek sonuçlar on binler [K25] |
| Rekabet (5=az) | 3 | 1 | 3 | Boomer shooter: 232 çıkış/yıl, 548 sırada [K24]; co-op'ta güçlü ama sayıca az rakip |
| Ayırt edicilik | 5 | 3 | 4 | "Deniz çekiliyor → ganimet → deniz geri geliyor" tek GIF'te okunur; co-op'ta kaos klibi üretir |
| Yapılabilirlik | 3 | 5 | 4 | A'da ağ kodu (replication, su durumu senkronu) 2 developer için en büyük risk; B en düşük teknik risk |
| Pazarlanabilirlik | 5 | 3 | 3 | Co-op yayıncı/arkadaş etkisi; playtest ile topluluk kurma |
| Ömür | 5 | 2 | 4 | Yeni kasaba/bölge DLC'si, gelgit modifikatörleri, EA içerik takvimi |
| **Toplam (30)** | **26** | **17** | **22** | |

**Tavsiye: Aday A**, ancak **C'ye geri çekilme kapısı** açık bırakılarak: prototip aşamasında (aşama 3) co-op ağ senkronu 6 hafta içinde 4 oyuncuyla kararlı çalışmıyorsa, aynı tasarım tek oyunculu roguelite olarak sürer ve co-op EA sonrası eklenir. Tasarım (gelgit kuralı, düşmanlar, silahlar, dünya) iki yolda da aynı kalır; israf olmaz.

---

## 7. Önerilen konsept: EBBSHIFT

| Başlık | Karar | Gerekçe |
|---|---|---|
| **Tek cümle** | *A 1–4 player co-op shooter where the tide is your clock: drop into a drowned seaside town at low water, haul out what the sea left behind, and fight your way uphill before it comes back.* | Tek GIF: sular sokaklardan yukarı tırmanırken merdivende geri geri ateş eden ekip |
| **Fantezi** | Usta, soğukkanlı, mavi yakalı kurtarma ekibi; "işini bilen hayatta kalır" | DRG/Helldivers kitlesinin sevdiği "çalışan insan" tonu; askerî kimlik yok |
| **Core loop** | 30 sn: gör → konum seç (ıslak/kuru) → ateş et → ganimeti al → yukarı çık. 5 dk: bir gelgit evresi. 1 saat: 2–3 görev + hub'da yükseltme | fps-game-design §1 |
| **Ayırt edici kural** | Gelgit çizelgesi görev başında gösterilir; su seviyesi tek küresel düzlemdir; savak (sluice) ile yerel olarak geciktirilebilir | Rogue Core'daki "soyut zamanlayıcı" şikâyetine yanıt: görünür, öngörülebilir, etkilenebilir [K39] |
| **Konumlandırma** | "Deep Rock Galactic'in ekip ruhu + her görevi yeniden şekillendiren gelgit" (referans; kopya değil) | Co-op PvE'nin "tekrar" şikâyetine sistemik yanıt [K7] |
| **Platform** | PC / Steam (Windows). Gamepad tam destek; Steam Deck "Playable" hedef. Konsol: 1.0 sonrası, yalnızca satış eşiği aşılırsa | Küçük ekip odağı; MOUSE satışlarının yarısı konsoldan geldi [K8], yani konsol ileride gerçek gelir ama başta dikkat dağıtır |
| **İş modeli** | Premium, tek seferlik satın alma. Mikro ödeme, loot box, battle pass **yok**. 1.0 sonrası ücretli içerik genişlemesi (yeni kasaba) + soundtrack + destekçi paketi (kozmetik) | fps-game-design §8; başarılı örneklerin tamamı premium |
| **Fiyat** | EA: **19,99 $** (çıkışta %10 indirim). 1.0'da 24,99 $'a yükseltme değerlendirilir | Far Far West, Jump Space, SULFUR 19,99 $; Abyssus, Deadzone 24,99 $; Mycopunk 14,99 $ [K3][K9–K11][K41][K42] |
| **Çıkış biçimi** | Steam sayfası erken → kapalı/açık playtest → demo → Next Fest → Erken Erişim (10–12 ay) → 1.0 | Tüm başarılı kıyasların izlediği yol |
| **Çok oyunculu altyapı** | Steam üzerinden P2P (listen server), özel sunucu yok, eşleşme isteğe bağlı (arkadaş daveti birincil) | Sunucu maliyeti ve operasyon yok; oyuncu sayısı düşse de oyun yaşar |
| **EA çıkış kapsamı** | 3 harita (bölge), 2 düşman fraksiyonu (10 arketip), 1 boss, 8 silah + 4 alet, 6 görev kartı (M0–M5), 1 hub | 2 artist + 2 developer için; bkz. hikâye belgesi §6 ve §9 |
| **EA güncellemeleri** | +1 harita, 2. boss, M6–M7 (2. perdenin sonu), +2 silah | EA yol haritası çıkış günü yayınlanır (R2) |
| **1.0 kapsamı** | +1 harita, +1 fraksiyon (4 arketip), final, +2 silah, 3. perde (M8–M9) | EA geliriyle finanse edilir; gelir gelmezse 1.0 küçültülür |
| **Süre (öneri)** | Prototip 2 ay → dikey dilim +3 ay → demo/Next Fest ~9.–10. ay → EA ~14.–15. ay → 1.0 ~26. ay | **Tahmin**; GM planında kesinleşir |
| **Adın durumu** | "EBBSHIFT" çalışma adı. Web aramasında aynı adlı oyun çıkmadı; resmî marka taraması yapılmadı | Ayrıntı: game-story.md §1 |

**Bilinçli olarak yapmadıklarımız:** PvP modu, açık dünya, prosedürel harita üretimi (yerine el yapımı harita + değişken rota/çizelge/modifikatör), karakter sınıfı kilitleri, yüz animasyonlu ara sahne, özel sunucu.

---

## 8. Gelir senaryoları ve başabaş

### 8.1 Varsayımlar (hepsi açık; değiştirilebilir)

| # | Varsayım | Değer | Dayanak |
|---|---|---|---|
| V1 | Liste fiyatı | 19,99 $ | Bölüm 7 |
| V2 | İlk yıl ortalama satış fiyatı (çıkış indirimi, bölgesel fiyat, mevsim indirimleri sonrası) | ~15,5 $ | **Tahmin** |
| V3 | Geliştiriciye kalan pay (platform, iade, vergi sonrası) | brütün %50'si → **~7,75 $/kopya** | Skill kuralı (%45–55); güncel birincil veri **bulunamadı** |
| V4 | İlk hafta satışı ≈ çıkış wishlist'i × 0,10–0,15 | medyan, 10 $+ oyunlarda 0,10 | GameDiscoverCo kıyası, Eyl 2024–Eyl 2025, 25 bin+ wishlist'li oyunlar [K29] |
| V5 | İlk yıl satışı ≈ ilk hafta × 4 | 4× | **Tahmin**; doğrulanamadı. Co-op'ta ağızdan ağıza etkisiyle daha yüksek, başarısız çıkışta daha düşük olur |
| V6 | Ekip maliyeti | **Bilinmiyor — kullanıcıdan alınmalı** | Aşağıda iki örnek bütçe ile gösterildi |
| V7 | Dış giderler (seslendirme, müzik, yerelleştirme, QA, pazarlama, yazılım, Steam ücreti) | 70–90 bin $ | **Tahmin** |
| V8 | Yayıncı | Yok (kendi yayınımız). Yayıncı varsa net pay ~%30 daha düşer (**tahmin**) | — |

### 8.2 Üç senaryo (EA çıkışından sonraki ilk 12 ay)

| Senaryo | Çıkıştaki wishlist | İlk hafta | İlk yıl kopya | Brüt (V2) | Geliştirici net (V3) | Kıyas |
|---|---|---|---|---|---|---|
| **Kötü** | ~20.000 | 2.000–3.000 | ~12.000 | ~186 bin $ | **~93 bin $** | 1.000 incelemenin altında kalan %97'lik dilim [K1] |
| **Beklenen** | ~100.000 | 10.000–15.000 | ~60.000 | ~930 bin $ | **~465 bin $** | Mycopunk/Abyssus bandının altı (onlar ~150–270 bin **tahmin**) [K41][K42] |
| **İyi** | ~300.000 | 30.000–45.000 | ~250.000 | ~3,9 milyon $ | **~1,94 milyon $** | Deadzone: Rogue / SULFUR bandı [K9][K10] |
| (Kıyas dışı) | 500.000+ | — | 1 milyon+ | — | — | Far Far West, Jump Space [K3][K11]; plana konmaz |

"Beklenen" senaryo bir **hedeftir, taban oranı değildir.** Taban oranı "kötü" senaryoya yakındır: Şubat 2026 Next Fest anketinde medyan oyun fest'te 806 wishlist kazandı [K27].

### 8.3 Başabaş

Formül: **başabaş kopya = toplam maliyet ÷ 7,75 $**

| Örnek bütçe | Hesap | Toplam | Başabaş kopya | Gereken çıkış wishlist'i (V4+V5) | Yayıncı varsa |
|---|---|---|---|---|---|
| **A (düşük maliyetli bölge)** | 6 kişi × 18 ay × 4.000 $ + 70 bin $ dış gider | ~500 bin $ | **~65.000** | ~110–160 bin | ~92.000 kopya |
| **B (Batı Avrupa ortalaması)** | 6 kişi × 18 ay × 8.000 $ + 86 bin $ dış gider | ~950 bin $ | **~123.000** | ~205–310 bin | ~175.000 kopya |

Kişi başı aylık maliyetler **örnektir, veri değildir**; gerçek rakamı kullanıcı vermeli.

**Sonuç:** A bütçesinde "beklenen" senaryo başabaşın hemen altında, B bütçesinde yalnızca "iyi" senaryo kârlı. Bu yüzden proje **wishlist kapılarıyla** yönetilmeli:

| Kapı | Zaman (öneri) | Eşik | Eşik tutmazsa |
|---|---|---|---|
| K1 — Prototip | 2. ay | "Su yükselirken yukarı savaşmak" 4 kişilik iç testte eğlenceli; ağ senkronu kararlı | Aday C'ye (tek oyunculu) dön |
| K2 — Steam sayfası + fragman | 4.–5. ay | İlk 4 haftada 5.000+ wishlist | Kancayı/görseli yeniden çerçevele, sayfayı test et |
| K3 — Açık playtest/demo | 9. ay | Demo medyan oynama süresi 30 dk+; fest öncesi 25.000+ wishlist | Kapsamı kıs, EA tarihini erteleme yerine küçült |
| K4 — Next Fest | 10. ay | Fest'te 10.000+ wishlist ("Diamond" kademesi [K27]) | Yayıncı arayışı veya fiyatı 14,99 $'a çekme |
| K5 — EA çıkışı | 14.–15. ay | Çıkışta 100.000+ wishlist | 1.0 kapsamını (3. perde) gelir gelene kadar dondur |

---

## 9. Riskler ve azaltma

| # | Risk | Olasılık | Etki | Azaltma |
|---|---|---|---|---|
| R1 | **Co-op ağ kodu 2 developer'ı aşar** (su durumu, AI, fizik senkronu) | Orta–yüksek | Çok yüksek | UE5 yerleşik replication + listen server; su tek deterministik düzlem (sunucu zamanı + çizelge → her istemci kendi hesaplar, senkron verisi neredeyse sıfır); prototipin ilk işi 4 oyunculu test; K1'de C'ye geri çekilme |
| R2 | **İçerik azlığı / tekrar** şikâyeti (türün 1 numaralı eleştirisi [K7]) | Yüksek | Yüksek | Çeşitliliği içerikten değil sistemden üret: gelgit çizelgesi türleri, iki fraksiyonun suya ters tepkisi, görev tipi × harita × modifikatör matrisi; EA yol haritası çıkış günü yayınlanır |
| R3 | **Keşfedilememe** (yılda 20 bin+ oyun [K1]) | Yüksek | Çok yüksek | Sayfa 4.–5. ayda; her sprint'te 1 klip; demo fest'ten aylar önce (2,5× etkisi [K27]); wishlist kapıları |
| R4 | **İlk saat zayıf** → iade ve kötü inceleme (Firebreak dersi [K22]) | Orta | Yüksek | Dikey dilim = ilk görev; 60 sn'de silah elde, 10 dk'da ilk "su geliyor" anı; tek başına oynanabilir öğretici |
| R5 | **Tek başına oynayan için dengesiz** | Orta | Orta | Düşman sayısı ve canlanma kuralı oyuncu sayısına göre ölçeklenir; solo "yedek halat" (bir kez kendini kaldırma) |
| R6 | **Tema komşuluğu:** Abyssus (denizaltı), "su yükselir" oyunları [K40][K41] | Orta | Orta | Su altı değil **gün ışığında sahil kasabası**; 1970'ler tatil beldesi estetiği; farklı slogan; "brine/abyss" sözcüklerinden kaçın |
| R7 | **Su görseli/performans** (UE5 su sistemi pahalı) | Orta | Orta | Stilize, tek düzlem su; kırılma/yansıma ucuz yaklaşımla; kapalı alanlarda post-process; ayarlanabilir kalite |
| R8 | **Zaman baskısı sevmeyen oyuncu** (Rogue Core dersi [K39]) | Orta | Orta | Çizelge baştan görünür; savakla su geciktirilebilir; "Neap" (sakin gelgit) zorluk seçeneği |
| R9 | **Büyük rakiple aynı pencere** (Far Far West 1.0 ~2027 baharı; EA "yaklaşık 12 ay" [K3]) | Orta | Orta | EA tarihini rakip takvimine göre seç; fest zamanlamasını buna göre ayarla |
| R10 | **Ad çakışması / marka** | Düşük–orta | Orta | Resmî marka taraması (USPTO/EUIPO) Steam sayfasından önce; yedek ad hazır |
| R11 | **Veri güveni:** bu rapordaki rakamlar arama özetlerinden | — | Orta | ★ işaretli rakamlar kaynak sayfadan teyit; SteamDB/Gamalytic erişimiyle tablo güncellenir |

---

## 10. Kaynakça

Tüm bağlantılara erişim tarihi: **25 Kasım 2026**. Erişim biçimi: web araması sonuç özeti (sayfanın tamamı okunamadı; bkz. en üstteki not).

| No | Kaynak | Kullanıldığı veri |
|---|---|---|
| K1 | GamesRadar — https://www.gamesradar.com/games/a-terrifying-20-282-games-were-released-on-steam-in-2025-and-just-608-managed-to-get-1-000-reviews-expert-finds-we-might-be-in-a-bit-of-an-indie-golden-age · How To Market A Game — https://howtomarketagame.com/2026/01/27/what-the-hell-happened-in-2025/ · Shazoo — https://shazoo.ru/2026/01/29/178897/za-2025-god-v-steam-vyslo-bolee-20-tysiac-igr-no-tolko-608-polucili-bolee-1000-otzyvov | 20.282 çıkış, 608 oyun 1.000+ inceleme, tür dağılımı |
| K2 | Steam Page Analyzer — https://www.steampageanalyzer.com/blog/how-many-games-release-on-steam | %2,99 / %2,44 oranları |
| K3 | Gematsu — https://www.gematsu.com/2026/04/far-far-west-launches-in-early-access-on-april-28 | Far Far West çıkış tarihi, fiyat, EA süresi |
| K4 | KitGuru — https://www.kitguru.net/gaming/joao-silva/evil-raptors-far-far-west-clears-half-a-million-copies-sold/ · GameDiscoverCo — https://newsletter.gamediscover.co/p/how-far-far-west-sold-500k-in-less · Fix Gaming Channel — https://www.fixgamingchannel.com/far-far-west-passes-1-million-sales-on-steam/ · Capsule Computers (basın bülteni) — https://www.capsulecomputers.com.au/?p=342795 | 500 bin+, 1 milyon, ilk 48 saat 250 bin / 26 bin eşzamanlı |
| K5 | PC Gamer — https://www.pcgamer.com/games/fps/developers-of-steam-next-fests-wildly-popular-robot-wizard-cowboy-shooter-say-its-cowboy-robots-are-also-wizards-because-magic-is-always-cool/ | Ekip ~7 kişi; demo 6.000+ inceleme, ilk 5 demo |
| K6 | Alinea Analytics — https://alineaanalytics.substack.com/p/steam-next-fests-wishlist-winners · Recognizing Patterns — https://recognizingpatterns.substack.com/p/far-far-west-didnt-win-steam-next | Fest haftasında ~101 bin wishlist (tahmin); toplam 536 bin (doğrulanamadı) |
| K7 | Notebookcheck — https://www.notebookcheck.net/The-next-co-op-hit-Wild-West-shooter-launches-with-96-positive-reviews.1290551.0.html · FullCleared — https://fullcleared.com/features/far-far-west-first-impressions/ · GamingTrend — https://gamingtrend.com/previews/far-far-west-preview/ | 19.500+ inceleme, %96; tekrar/düşman çeşidi eleştirisi |
| K8 | GamingBolt — https://gamingbolt.com/mouse-p-i-for-hire-has-sold-730000-units-since-launch-recoups-publishers-expenses · Gematsu — https://www.gematsu.com/2026/07/mouse-p-i-for-hire-sales-top-one-million · GamesRadar — https://www.gamesradar.com/games/fps/mouse-p-i-for-hire-made-all-its-money-back-in-a-month-selling-730-000-copies-in-stronger-than-expected-launch-with-94-percent-positive-steam-reviews/ · Deku Deals — https://www.dekudeals.com/items/mouse-p-i-for-hire | MOUSE satış, konsol payı, fiyat, inceleme |
| K9 | GamesUnit — https://gamesunit.de/artikel/deadzone-rogue-the-new-roguelite-fps-sells-150k-copies-in-first-two-weeks-of-early-access-212823.html · Notebookcheck — https://www.notebookcheck.net/New-co-op-roguelike-FPS-game-hits-full-release-on-Steam-with-promising-reviews-and-20-launch-discount.1085240.0.html | Deadzone: Rogue 150 bin / 2 hafta; 24,99 $; 1.0 tarihi |
| K10 | Indie-Games.eu — https://www.indie-games.eu/indie-fps-roguelite-sulfur-sold-200000-copies/ · Game8 — https://indie.game8.co/news/sulfur-early-access-version-out-now-on-steam/ · Gematsu — https://www.gematsu.com/2026/10/sulfur-launches-october-28 · Destructoid — https://www.destructoid.com/gun-customizing-goblin-slaying-fps-sulfur-is-out-now-and-you-should-play-it/ | SULFUR 200 bin, 19,99 $, 1.0 tarihi, co-op eşiği |
| K11 | Mynewsdesk (basın bültenleri) — https://www.mynewsdesk.com/uk/swipe-right/pressreleases/indie-spacefaring-adventure-jump-space-hits-250000-players-in-its-debut-weekend-among-top-5-best-sellers-globally-on-steam-3406320 · https://www.mynewsdesk.com/uk/swipe-right/pressreleases/co-op-adventure-jump-space-celebrates-anniversary-with-september-update-and-free-weekend-3466699 · Game8 — https://game8.co/articles/reviews/8554 | Jump Space 430 bin / 3 hafta, 900 bin+, 19,99 $ |
| K12 | GamingBolt — https://gamingbolt.com/witchfire-has-sold-500000-copies-since-entering-early-access | Witchfire 500 bin+ |
| K13 | Game Developer (basın bülteni) — https://www.gamedeveloper.com/press-release/wardogs-is-coming-to-steam-early-access-on-10-september · Tech4Gamers — https://tech4gamers.com/indie-shooter-wardogs-400k-players-steam/ · Steam — https://store.steampowered.com/app/1867240 | WARDOGS tarih, fiyat, satış iddiaları |
| K14 | TechRadar — https://www.techradar.com/gaming/highguard-will-shut-down-just-45-days-after-launch-we-are-witnessing-the-era-of-two-month-lifespan-shooters · PC Gamer — https://www.pcgamer.com/games/fps/highguard-is-closing-next-week-despite-the-passion-and-hard-work-of-our-team-we-have-not-been-able-to-build-a-sustainable-player-base-to-support-the-game-long-term/ | Highguard kapanışı |
| K15 | Instant Gaming News — https://news.instant-gaming.com/en/articles/18769-pubg-blindspot-is-shutting-down-its-servers-less-than-two-months-after-its-release | PUBG: Blindspot |
| K16 | PC Gamer — https://www.pcgamer.com/games/action/extraction-shooter-the-cube-save-us-has-announced-its-shutting-down-just-three-weeks-after-it-launched/ | The Cube, Save Us |
| K17 | PC Gamer — https://pcgamer.com/games/fps/just-two-weeks-after-launch-extraction-survival-horror-game-level-zero-extraction-ends-development-its-no-longer-feasible-for-our-relatively-small-teams-to-sustain-the-game | Level Zero: Extraction |
| K18 | MMOs.com — https://mmos.com/news/f2p-moba-extraction-game-seekers-of-skyveil-shuts-down-18-days-after-launch-studio-elodie-closes | Seekers of Skyveil |
| K19 | PC Gamer — https://pcgamer.com/games/fps/1-year-after-launch-spaceship-shooter-wildgate-will-end-development-after-its-next-update/ · TechPowerUp — https://www.techpowerup.com/350315/wildgate-gets-final-major-update-less-than-1-year-after-launch | Wildgate |
| K20 | PC Gamer — https://pcgamer.com/games/fps/last-flag-the-debut-game-from-the-studio-founded-by-imagine-dragons-lead-singer-becomes-the-latest-multiplayer-shooter-struggling-to-find-an-audience · MeinMMO — https://mein-mmo.de/en/co-op-game-raises-17-million-from-investors-crashes-spectacularly-on-steam,1570599/ | Last Flag; Starlight Re:Volver |
| K21 | PC Gamer — https://pcgamer.com/games/fps/spectre-divide-and-its-studio-are-shutting-down-after-just-six-months-the-industry-is-in-a-tough-spot-right-now | Spectre Divide |
| K22 | PC Gamer — https://www.pcgamer.com/games/fps/remedy-is-unsatisfied-by-poor-sales-of-fbc-firebreak-and-a-tiny-pc-playerbase-but-says-despite-the-rocky-launch-we-believe-we-have-a-solid-game-to-build-on · 80.lv — https://80.lv/articles/fbc-firebreak-breaks-1-million-players-and-the-only-question-everyone-has-is-how | FBC: Firebreak |
| K23 | OpenCritic — https://opencritic.com/news/19362/-this-game-is-not-worth-your-money-right-now-killing-floor-3-has-launched-to-some-surprisingly-negative-reviews · WhatCulture — https://whatculture.com/gaming/20-recent-video-games-that-were-sent-out-to-die?page=13 · Raijin — https://raijin.gg/app/1430190/Killing_Floor_3 | Killing Floor 3 inceleme, oyuncu kaybı, tahmini satış |
| K24 | Steam Page Analyzer — https://www.steampageanalyzer.com/niches/boomer-shooter | Boomer shooter çıkış sayıları (Eylül 2026 görüntüsü) |
| K25 | Game World Observer — https://gameworldobserver.com/2024/06/17/selaco-sales-51k-copies-gzdoom-shooter-success | Selaco |
| K26 | Gamepressure — https://www.gamepressure.com/newsroom/3d-realms-has-weathered-criticism-of-its-new-boomer-shooter-poorl/z36cb8 | Phantom Fury |
| K27 | How To Market A Game — https://howtomarketagame.com/2025/03/26/benchmarks-how-many-wishlists-can-i-get-from-steam-next-fest/ · Ziva — https://ziva.sh/blogs/steam-next-fest-2026 · Recognizing Patterns — https://recognizingpatterns.substack.com/p/steam-next-fest-june-2026-communities | Next Fest kademeleri, medyan 806, r = 0,825, erken demo 2,5× |
| K28 | PC Gamer — https://pcgamer.com/software/platforms/the-50-most-played-steam-next-fest-demos-include-7-incremental-games-4-ai-disclosures-and-only-one-instance-of-haystack-slop/ | Ekim 2026 Next Fest ölçeği |
| K29 | GameDev Reports (GameDiscoverCo özeti) — https://gamedevreports.substack.com/p/gamediscoverco-the-state-of-steam · GameDiscoverCo — https://newsletter.gamediscover.co/p/the-state-of-steam-wishlist-conversions | Wishlist → ilk hafta dönüşümü (0,15 / 0,10) |
| K30 | PC Gamer — https://www.pcgamer.com/gaming-industry/valve-says-over-5000-games-made-over-usd100k-on-steam-last-year · TechSpot — https://www.techspot.com/news/111663-game-developers-cash-steam-sees-5863-titles-earn.html | 5.863 oyun 100 bin $+ |
| K31 | 80.lv — https://80.lv/articles/analysts-report-drop-in-game-revenue-on-steam-despite-growing-number-of-releases · https://80.lv/articles/steam-earned-usd16b-in-2025-but-nearly-half-of-19-000-games-got-under-10-reviews | Medyan 249 $; ~300 oyun 1 milyon $+ |
| K32 | Sensor Tower — https://sensortower.com/blog/indie-and-new-ip-break-out-without-denting-aaa · PC Gamer — https://www.pcgamer.com/gaming-industry/indies-and-non-sequels-have-exploded-in-the-last-year-and-its-not-all-friendslop/ | 76 / 55 çıkış; bağımsız 33 / 13; co-op kümesi |
| K33 | Outlook Respawn — https://respawn.outlookindia.com/gaming/gaming-news/from-game-jam-to-10-million-units-indie-hit-peak-success · Gamereactor — https://www.gamereactor.eu/arc-raiders-friendslop-games-continue-to-sell-huge-numbers-in-2026-1670383/ | PEAK, R.E.P.O. |
| K34 | Steam Page Analyzer — https://www.steampageanalyzer.com/blog/steam-review-to-sales-multiplier | İnceleme çarpanı (ikincil) |
| K35 | Levvvel — https://levvvel.com/ultrakill-statistics/ · Steam Page Analyzer — https://www.steampageanalyzer.com/games/1229490 | ULTRAKILL tahmin aralığı |
| K36 | Game Revenue Data — https://gamerevenuedata.pages.dev/games/trepang2 | Trepang2 tahmini |
| K37 | Wikipedia — https://en.wikipedia.org/wiki/Bodycam_(video_game) · 80.lv — https://80.lv/articles/bodycam-developer-talks-the-studio-s-growth-bodycam-development-future-plans | Bodycam satış, ekip büyümesi |
| K38 | VGChartz — https://www.vgchartz.com/article/465689/ready-or-not-sales-top-13-million-units-including-3-million-units-on-consoles/ | Ready or Not |
| K39 | GamesRadar — https://www.gamesradar.com/games/fps/deep-rock-galactic-roguelike-spin-off-launches-to-mostly-positive-steam-reviews-that-cant-decide-if-they-love-or-hate-its-timer-and-shared-progression-tool/ · Steam — https://store.steampowered.com/app/2605790/Deep_Rock_Galactic_Rogue_Core | Rogue Core incelemeler, fiyat |
| K40 | Steam — https://store.steampowered.com/app/4864580/ (TideFall) · https://store.steampowered.com/app/2269650 (Flood Escape) · https://store.steampowered.com/sub/670928 (TIDEBREAK) · Steambase — https://steambase.io/games/tidefall/info | "Su yükselir" komşu oyunlar |
| K41 | SteamDB — https://steamdb.info/app/1721110 · Steambase — https://steambase.io/games/abyssus/reviews · Notebookcheck — https://www.notebookcheck.net/New-Doom-inspired-co-op-roguelite-FPS-debuts-on-Steam-with-Very-Positive-reviews-and-launch-discount.1086224.0.html | Abyssus |
| K42 | SteamDB — https://steamdb.info/app/3247750 · SteamPulse — https://steampulse.org/game/3247750 | Mycopunk |
| K43 | PCGamesN — https://www.pcgamesn.com/steam/linux-users-hardware-survey · iTechGuides — https://www.itechguides.com/42-4-of-steam-users-play-pc-games-exclusively/ · God is a Geek — https://godisageek.com/2026/08/valve-steam-deck-steam-machine-memory-crisis/ | Steam Deck payı, Steam Machine |
| K44 | KitGuru — https://www.kitguru.net/?p=750761 · iXBT — https://ixbt.games/en/news/2026/09/02/valve-nazvala-samye-populiarnye-videokarty-steam-v-avguste-2026-goda-i-podtverdila-vaznost-russkoi-lokalizacii.amp.html · Quantumrun — https://www.quantumrun.com/consulting/steam-language-statistics/ | Steam dil payları |

### Aranıp bulunamayan veriler

- Far Far West, Jump Space, MOUSE, SULFUR, Deadzone: Rogue, Abyssus, Mycopunk için **doğrulanmış ekip büyüklüğü** (yalnız Far Far West için ~7 kişi bulundu).
- VOID/BREAKER, DRG: Rogue Core, Phantom Fury, Turbo Overkill, Cultic, Roboquest, Gunfire Reborn için **satış rakamı**.
- Haziran ve Ekim 2026 Next Fest için **medyan wishlist** ve en çok oynanan demolar içinde FPS dökümü.
- 2026 için GameDiscoverCo/Gamalytic **birincil inceleme çarpanı** ve **ilk yıl / ilk hafta satış oranı**.
- FPS türüne özgü **medyan gelir** (Totally Human Media raporunda var; sayfaya erişilemedi).
- Steam'de Brezilya Portekizcesi dil payı.
- Rakiplerin geliştirme bütçeleri.
