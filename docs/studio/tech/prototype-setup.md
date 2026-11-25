# Prototip kurulumu: EBBSHIFT prototipi (3C, gelgit, iki silah + Kedge, ganimet, iki düşman)

> **Uyarı: bu kod henüz hiç derlenmedi ve editörde hiç çalıştırılmadı.**
> İlk tur (iskelet), gelgit entegrasyonu ve ikinci tur (bu belge) Unreal Engine kurulu olmayan bir bulut ortamında yazıldı; Unreal MCP de bağlı değildi.
> İlk derlemede hata çıkması olasıdır. Hata çıktısını (Output/Build log) olduğu gibi `gameplay-developer`'a iletin.
> En riskli noktalar bu belgenin sonundaki "Derleme riskleri" bölümündedir. Aşağıdaki "görmeniz beklenen" cümlelerinin tamamı tasarım niyetidir, gözlem değildir.

Hedef motor: **UE 5.8**. Hiçbir `.uasset` içermez. Kod, hiçbir varlık oluşturmadan da oynanabilecek biçimde yazıldı: yerleşik tuş atamaları, spec değerlerini sınıf varsayılanı olarak taşıyan veri sınıfları (Purser, Gaffer, Kedge, 3C, gelgit çizelgesi, kurallar, düşmanlar, dalga betiği) ve motorun temel şekillerinden gri kutu görseller.

Sayıların kaynağı: [prototype-spec.md](../design/prototype-spec.md) (Revizyon 2). Mimari kararlar: [adr-001-tide-and-network.md](adr-001-tide-and-network.md).

## 1. İlk açılış

1. UE 5.8 ve C++ araç zinciri kurulu olmalı (Windows: Visual Studio 2022 + "Game development with C++"; macOS: Xcode).
2. `NewFPS.uproject` dosyasına sağ tıklayın → **Generate Visual Studio project files** (macOS: **Services → Generate Xcode Project**).
3. `NewFPS.uproject` dosyasına çift tıklayın. "Missing modules... rebuild?" sorusuna **Yes** deyin. Ya da `NewFPS.sln` içinden **Development Editor / Win64** ile derleyip çalıştırın (hata mesajları burada daha okunaklıdır).
4. Editör açılınca **Edit → Plugins → Model Context Protocol** eklentisini açıp yeniden başlatın (bkz. `docs/MCP-SETUP.md`). Eklenti `.uproject` içine bilerek yazılmadı: eklentinin iç adı doğrulanamadı.

**Motor sürümünü değiştirmek:** `NewFPS.uproject` içindeki `"EngineAssociation": "5.8"` değerini kurulu sürümle değiştirin (örn. `"5.6"`), ya da dosyaya sağ tıklayıp **Switch Unreal Engine version...** seçin. Kaynaktan derlenmiş motorlarda bu alan bir GUID olur; aynı menü onu yazar. Kod 5.1 sonrası kararlı API'lerle yazıldı, ama 5.8 dışındaki sürümlerde de denenmedi.

## 2. Hiç varlık oluşturmadan ilk deneme

### 2.1 Seviyeye konacak aktörler

1. **File → New Level → Basic**. `Content/NewFPS/Maps/L_Prototype` olarak kaydedin. GameMode `Config/DefaultEngine.ini` içinde `NFGameMode` olarak ayarlı.
2. **Place Actors** panelinden aşağıdakileri koyun (hepsi C++ sınıfıdır, ada göre aratın):

| Aktör | Kaç tane | Nereye / ayar |
|---|---|---|
| `Player Start` | 1–4 | Vinç platformu (lifeline ile dönen oyuncu da buraya iner). |
| `Player Start` (**Player Start Tag = `FloodDrill`**) | 1–4 | Ganimet noktası C. Yalnız Flood Drill oyuncuları buraya taşır. |
| `NF Tide Manager` | tam 1 | Konumu önemsiz (su düzlemi X/Y'de aktörün olduğu yerde, 1000 m kare). `Tide Data` boşsa spec'in 12 dakikalık çizelgesi kullanılır. Düz bir test haritasında suyun zemine göre yüksekliğini `Water Z Offset` ile kaydırın. |
| `NF Strongbox` | 6 (spec) | Ganimet noktalarına, zemine. Kilitli dolap yok: sandıklar açıkta durur. |
| `NF Winch Point` | 1 | Vinç platformu zeminine (aktör orijini zeminde). 400 × 400 ağ; içine bırakılan sandık sayılır. |
| `NF Enemy Spawner` | tam 1 | Konumu önemsiz. `Wave Data` boşsa spec §8.2 betiği kullanılır. |
| `NF Enemy Spawn Point` | spec §7.5: 13 Drift + 7 Warden | Zemine. Her birinde `Faction`, `Water Rule`, gerekiyorsa `Submerged Margin` ve `Group` (aşağıdaki tablo). |
| `Nav Mesh Bounds Volume` | 1 | Tüm yürünebilir alanı kaplasın (**P** ile yeşil alanı görün). NavMesh yoksa düşmanlar doğar ama yürümez. |
| `NF Target Dummy` | isteğe bağlı | Atış denemesi için. |

Doğuş noktası ayarları (spec §7.5):

| Nokta | Faction | Water Rule | Submerged Margin | Group |
|---|---|---|---|---|
| D-Low, deniz kenarı ×4 | Drift | Always | 0 | — |
| D-Low, iskele altı ×1 | Drift | Submerged | 0 | — |
| D-Half ×4 (menfez, zemin Z 400) | Drift | Submerged | 50 (su Z ≥ 350) | — |
| D-High ×4 (oluk, zemin Z 800) | Drift | Submerged | 50 (su Z ≥ 750) | — |
| W-Half ×4 | Wardens | Dry | — | `W-Half` |
| W-High ×3 | Wardens | Always | — | `W-High` |

3. **Play**. Haritayı varsayılan yapmak için `Config/DefaultEngine.ini` içindeki iki yorum satırını (`EditorStartupMap`, `GameDefaultMap`) açın.

Düz zeminli bir "gym" haritasında (spec §7.7 adım 1) gelgiti elle gezdirmek için: `nf.Tide.Pause 1`, sonra `nf.Tide.SetElapsed <saniye>`.

### 2.2 Yerleşik tuşlar (hiç girdi varlığı atanmadığında devreye girer)

| Eylem | Klavye/fare | Gamepad |
|---|---|---|
| Hareket | W A S D | Sol çubuk |
| Bakış | Fare | Sağ çubuk |
| Zıplama | Space | A / Çarpı |
| Çömelme (basılı) | Sol Ctrl | B / Daire |
| Koşu (basılı, yalnız ileri) | Sol Shift | Sol çubuk tıklama |
| Ateş | Sol tık | Sağ tetik |
| ADS (basılı) | Sağ tık | Sol tetik |
| Reload | R | X / Kare |
| **Silah değiştir** (Purser ↔ Gaffer; sandık taşırken: bırak) | Q | Y / Üçgen |
| **Purser / Gaffer doğrudan** | 1 / 2 | — |
| **Kedge (yakın dövüş)** | V | Sağ çubuk tıklama |
| **Etkileşim** (basılı tut: sandık al 0,5 sn / arkadaşı kaldır 3 sn; taşırken bas: bırak) | E | D-pad aşağı |

Gamepad atamaları yer tutucudur (aim assist ve düzen dikey dilim işi).

### 2.3 Konsol komutları

| Komut | Ne yapar |
|---|---|
| `nf.FloodDrill` | Sunucu. Spec §8.3: gelgit saati 4:30, oyuncular `FloodDrill` etiketli Player Start'lara, çevrelerine 2 Attendant + 4 Scuttler, kota = 2 oyuncuya 1 sandık. |
| `nf.Mission.Restart` | Sunucu. Haritayı yeniden yükler. |
| `nf.Tide.SetElapsed <sn>` / `nf.Tide.Pause 0\|1` | Sunucu. Gelgit saatini taşır / dondurur. |
| `nf.Tide.Debug 0\|1` | Su yüksekliği, saat ve yerel derinlik satırı. |
| `nf.Weapon.DebugShots 0\|1` | Atış izleri (saçma başına bir çizgi). |
| `Log LogNewFPS Verbose` | Hasar hattı, reddedilen atışlar, atış başına saçma toplamı. |

Flood Drill'i her oturumda açmak için: PIE'de **Play → Advanced Settings → Server Game Options** alanına `?FloodDrill`, ya da `BP_NFGameMode` → `Start In Flood Drill`.

### 2.4 Test adımları (beklenen davranış; hiçbiri henüz gözlenmedi)

**3C (spec §2):** yürüme 500, koşu 650 (yalnız ileri), çömelme 250, ADS 300 cm/sn (`stat` yerine `DisplayAll CharacterMovementComponent Velocity` ya da gözle). Zıplama 110'luk kutunun üstüne çıkamamalı (86 cm). Çömelince kapsül üstü 110, göz 104.

**Islaklık ve silah (spec §3.3), gym havuzunda ya da `nf.Tide.SetElapsed` ile:**

| Derinlik | Beklenen |
|---|---|
| < 50 | Etki yok. |
| 50–99 | HUD "WADING"; hız 0,75'ten 0,5'e doğru **kademesiz** düşer (ADR-001: bilinçli, basamak değil); zıplama ×0,8. |
| 100–134 | HUD "WAIST DEEP - PURSER FROM THE HIP ONLY". Gaffer eldeyse kendiliğinden Purser'a geçer ve seçilemez; ADS açılmaz (sağ tık basılıyken sudan çıkınca kendiliğinden açılır); nişangâh ×1,5 açılır; reload 2,75 sn; koşu ve zıplama yok. |
| 135–164 | HUD "TOO DEEP TO SHOOT"; ateş, reload yok; silah adı gri; **Kedge çalışır**; sandık alınamaz (eldeki kalır). |
| ≥ 165 | HUD "UNDER WATER"; Kedge de yok; eldeki sandık düşer; 8 sn sonra 20 can/sn. |

**Silahlar (spec §4):** Purser 6 mermi, 0,40 sn aralık, tek tek tık (yarı otomatik), reload 2,2. Gaffer 8 iz, 0,857 sn aralık, 30 yedek; hedefte **tek** hasar sayısı (saçmaların toplamı; 7 m içinde tam isabette 120). Değiştirme 0,40 sn (HUD "..."), reload'u iptal eder. Kedge: V; 0,15 sn sonra 180 cm / 60° içindeki en yakın hedefe 40; 0,60 sn boyunca ateş ve ADS kilitli; süren reload'u iptal eder (mermi gelmez, yeniden başlamaz).

**Ganimet (spec §2.3, §6.4):** sandığa 220 cm içinde E'yi 0,5 sn tut → sol altta vekil kutu; hız 400, koşu yok, alçak zıplama; yalnız Purser, ADS ve reload yok, Kedge yok. E ya da Q → sandık ayak dibine iner. Vinç ağına bırakınca HUD "STRONGBOXES n / kota" artar. Kota = oyuncu + 1.

**Düşmanlar (spec §5):** Scuttler suda hızlı (504) ve iyileşir, kuruda 360, 20 sn kuru kalınca Parched (300, tek atışlık); Pinch öncesi kıskaçlar kalkar (0,5 sn), Lunge öncesi gövde çöker (0,6 sn); bir oyuncuya aynı anda en çok 2'si saldırır, kalanı çevrede döner. Attendant: lamba kırmızı (0,7 sn) → 3 mermi; fark ettikten sonraki ilk yaylım yana gider; 4 yaylımdan sonra 3 sn lamba 1 Hz göz kırpar; 150 cm içinde iter; tek atışta ≥ 80 hasarda sendeleyip geri gider; belde 2 sn kalınca devre dışı kalır.

**Can ve düşme (spec §6):** 5 sn hasarsızlıktan sonra 50'ye kadar 8 can/sn. Tek oyuncu: ilk düşüşte 3 sn sonra kendiliğinden kalkar (40 can), ikincide "SHIFT FAILED". Co-op: düşen sürünür, 60 sn (suda 3 kat hızlı) içinde arkadaşı E'yi 3 sn tutarsa 40 canla kalkar; yoksa 45 sn sonra Player Start'ta 50 canla döner. Herkes aynı anda yerdeyse görev biter. 12:00'de kota tamamsa "THAT'S A SHIFT".

## 3. Kalıcı kurulum: oluşturulacak varlıklar

Önerilen klasör: `Content/NewFPS/`. Adlar öneridir.

### 3.1 Girdi (`Content/NewFPS/Input/`)

Content Browser → sağ tık → **Input → Input Action**:

| Varlık | Value Type |
|---|---|
| `IA_Move` | Axis2D (Vector2D) |
| `IA_Look` | Axis2D |
| `IA_LookStick` | Axis2D |
| `IA_Jump`, `IA_Crouch`, `IA_Sprint`, `IA_Fire`, `IA_Aim`, `IA_Reload` | Digital (bool) |
| `IA_SwitchWeapon`, `IA_SlotOne`, `IA_SlotTwo`, `IA_Melee`, `IA_Interact` (**yeni**) | Digital (bool) |

Sonra **Input → Input Mapping Context** → `IMC_Default`:

- `IA_Move`: `W` (modifier: **Swizzle Input Axis Values**, YXZ), `S` (**Negate**, sonra **Swizzle**), `D` (yok), `A` (**Negate**), `Gamepad Left Thumbstick 2D-Axis` (**Dead Zone**). Sonuç: X = sağ, Y = ileri.
- `IA_Look`: `Mouse XY 2D-Axis`. **Negate eklemeyin**; kod X = sağ, Y = yukarı bekler. Dikey eksen tersse karakterdeki `bInvertLookY` kutusunu kullanın.
- `IA_LookStick`: `Gamepad Right Thumbstick 2D-Axis` (**Dead Zone**).
- Diğerleri: §2.2 tablosundaki tuşlar. Trigger eklemeyin; kod `Started`/`Completed` olaylarını dinler (`IA_Interact` için basılı tutma süresini sunucu sayar, Hold trigger eklemeyin).

### 3.2 Veri varlıkları (`Content/NewFPS/Weapons/`, `Content/NewFPS/Data/`)

Content Browser → sağ tık → **Miscellaneous → Data Asset**. Sınıfı seçince varlık **spec değerleriyle dolu** açılır; yalnız değiştirmek istediğinizi değiştirin. Hiçbiri zorunlu değildir: atanmayan her alan aynı sınıf varsayılanlarını kullanır.

| Sınıf | Önerilen ad | Atanacağı yer |
|---|---|---|
| `NFPurserData` | `DA_Purser` (**`Content/NewFPS/Weapons/` altında**; Asset Manager bu klasörü tarar) | `BP_NFCharacter` → `WeaponComponent` → **Loadout[0]** |
| `NFGafferData` | `DA_Gaffer` (aynı klasör) | **Loadout[1]** |
| `NFMeleeData` | `DA_Kedge` | `WeaponComponent` → **Melee Data** |
| `NFMovementData` | `DA_Movement_Default` | `BP_NFCharacter` → **NewFPS \| Movement** |
| `NFRulesData` | `DA_Rules` | `BP_NFCharacter` → **NewFPS \| Rules** ve `BP_NFGameMode` → **NewFPS \| Rules** (ikisine de aynı varlık) |
| `NFTideData` | `DA_Tide_Promenade` | Seviyedeki `NF Tide Manager` → **Tide Data** |
| `NFScuttlerData` / `NFAttendantData` | `DA_Scuttler` / `DA_Attendant` | `BP_Scuttler` / `BP_Attendant` → **Data** |
| `NFWaveData` | `DA_Waves_Promenade` | Seviyedeki `NF Enemy Spawner` → **Wave Data** |

`NFWeaponData` taban sınıfı da duruyor (600 RPM test tüfeği); üçüncü bir silah denemek için kullanılabilir.

### 3.3 Blueprint sınıfları (`Content/NewFPS/Blueprints/`)

1. `NFCharacter`'dan türeyen `BP_NFCharacter`. Class Defaults:
   - **NewFPS | Input**: `Default Mapping Context` = `IMC_Default`; on dört `... Action` alanına ilgili `IA_*`.
     (Mapping context atanıp bir action boş bırakılırsa o eylemin tuşu olmaz; log'a uyarı düşer.)
   - **NewFPS | Movement**: `Movement Data`; **NewFPS | Rules**: `Rules Data`.
   - Bileşen `WeaponComponent` → **Weapon | Loadout** (iki eleman: Purser, Gaffer), **Melee Data**.
2. `NFGameMode`'dan türeyen `BP_NFGameMode`: `Default Pawn Class` = `BP_NFCharacter`, `Rules Data`.
3. `NFScuttler` → `BP_Scuttler`, `NFAttendant` → `BP_Attendant` (`Data` ve, model gelince, mesh). Bunları kullanmak için `DA_Waves_Promenade` → **Classes** alanlarına yazın.
4. **Project Settings → Maps & Modes → Default GameMode** = `BP_NFGameMode` (ya da haritanın World Settings → GameMode Override).

## 4. Çok oyunculu test (2 istemci + gecikme)

1. Play düğmesinin yanındaki menü → **Number of Players = 2**, **Net Mode = Play As Client** (dedicated server + 2 istemci). Ayrıca **Play As Listen Server** ile de bir tur deneyin.
2. Her istemci penceresinde konsol (`~`): `Net PktLag=120`, sonra `Net PktLoss=3`. Kapatmak için ikisini de `0` yapın.
   Alternatif: **Editor Preferences → Level Editor → Play → Multiplayer Options → Enable Network Emulation**.
3. Kontrol listesi:
   - Tetik → ilk atış gecikmesiz mi (ses/iz/tepme sunucuyu beklememeli)?
   - Cephane sayacı atarken geri sıçrıyor mu? (Kısa süreli düzeltme beklenir, sürekli titreme beklenmez.)
   - Hitmarker yaklaşık ping kadar geç gelir; bu tasarım gereği (sunucu onayı).
   - Diğer istemci atışın izini ve hedefin tepkisini görüyor mu?
   - Koşu/ADS yürüyüşü sırasında "lastik bant" (rubber-band) var mı? Olmamalı; bu iki durum saved-move bayraklarıyla taşınıyor.
   - Reload bitince ilk atış reddediliyor mu? (Sunucu log'u: `Log LogNewFPS Verbose` → "Shot rejected".)
   - İki ekranda `nf.Tide.Debug 1` satırı (su Z, saat) eşleşiyor mu? Geç katılan istemcide de?
   - Bel suyuna girince iki tarafta da Gaffer iniyor, ateş reddedilmiyor mu ("Shot rejected ... weapon blocked" sürekli çıkmamalı)?
   - Silah değiştirme ve Kedge: istemcide anında, sunucu log'unda ret yok; Kedge isabeti ping kadar geç onaylanır (sunucu bulur).
   - Sandığı istemci alıyor, diğer oyuncu taşıyıcının yanında kutuyu görüyor, bırakınca kutu iki tarafta aynı yerde mi? Alırken/bırakırken tek bir küçük düzeltme beklenir.
   - Düşen oyuncuyu diğeri 3 sn'de kaldırabiliyor mu? Oyuncular birbirine hasar vermemeli (dost ateşi kapalı; diğer oyuncu gri bir silindir olarak görünür).
   - 2 oyuncuda dalgalar ×1,6, Attendant ateş token'ı 3, kota 3 mü?

## 5. Designer: hangi değer nerede

Kod derlemeye gerek yok; veri varlığını açıp değiştirin, Play'e basın. Koddaki varsayılanlar spec'in başlangıç değerleridir. Spec §9'daki "ilk oynanacak 10 değer"in yeri:

| # | Spec §9 değeri | Varlık → alan |
|---|---|---|
| 1, 2, 3 | F1 / F3 yükselme hızı, Half Tide duraklaması | `NFTideData` → **Cycle → Phases**: `Duration Weight` saniyedir (toplamı `Cycle Duration Seconds` ile aynı tutun), `Start/End Level` = su Z / `High Water Z` |
| 4 | Diz / bel hız çarpanı | `NFTideData` → **Effects**: `Knee Speed Multiplier`, `Waist Speed Multiplier`, `Chest Speed Multiplier`; geçişin yumuşaklığı `Wade Blend Depth` |
| 5 | Yüklü yürüme hızı | `NFMovementData` → **Carry**: `Carry Walk Speed` |
| 6 | Scuttler ıslak hız | `NFScuttlerData` → **Water**: `Wet Speed` |
| 7 | Scuttler tavanı, dalga büyüklüğü, M eğimi | `NFWaveData` → **Scaling** (`Drift Alive Cap By Player Count`, `Player Scale Slope`) ve **Waves** (`Solo Count`) |
| 8 | Attendant yaylım hasarı, ateş token'ı | `NFAttendantData` → **Punch Burst**: `Projectile Damage`, `Fire Tokens By Player Count` |
| 9 | High Water yüksekliği | `NFTideData` → **Height**: `High Water Z` (Phases oranları buna göre ölçeklenir) |
| 10 | Kota | `NFRulesData` → **Quota**. Dolap açma süresi: dolap yok (yapılmadı) |

Diğerleri:

| Varlık | Gruplar |
|---|---|
| `NFPurserData` / `NFGafferData` | **Damage** (`Damage` saçma başına, `Pellets Per Shot`, çarpanlar, menzil, düşüş eğrisi: X = metre) · **Fire** · **Handling** (`Equip Time`, `Usable While Carrying`, `Max Usable Wetness`) · **Ammo** · **Spread** (yarım açı, derece) · **Recoil** · **ADS** · **Presentation** |
| `NFMeleeData` | `Damage`, `Swing Interval`, `Hit Delay`, `Reach`, `Cone Angle` |
| `NFMovementData` | **Body** (çömelmiş yarı yükseklik, göz yükseklikleri zeminden) · **Ground** · **Air** · **Carry** · **Downed** |
| `NFTideData` | **Wetness** (`Knee/Waist/Chest/Overhead Depth`, histerezis 8 cm, gecikme 0,2 sn) · **Effects** (bel suyunda yayılım ×1,5 ve reload ×1,25, boğulma) · **Warnings** |
| `NFRulesData` | **Health** (yenilenme) · **Downed** · **Solo** · **Interaction** · **Quota** · **Flood Drill** |
| `NFScuttlerData` | **Water** · **Pinch** · **Lunge** · **Crowd** (token) · **Reactions** |
| `NFAttendantData` | **Movement** · **Punch Burst** · **Service** · **Shove** · **Reactions** · **Water** |

Değerler doğuşta okunur; değişiklikten sonra Play'i yeniden başlatın.

**Veri varlığında olmayanlar (karakter/düşman Blueprint'inde):** oyuncu kapsülü 34/90 (`BP_NFCharacter` → CapsuleComponent; motor çömelme bitince kapsülü sınıf varsayılanından geri yüklediği için veriden okunamıyor), Scuttler kapsülü 35/35 ve isabet hacimleri, Attendant kapsülü 40/95 ve lamba küresi (r 18, Z 172), `DefaultFOV` (95), fare hassasiyeti.

**Developer yer tutucuları (spec'te sayısı olmayanlar; "tahmin"):** Scuttler'ı "Gaffer iter" eşiği ve mesafesi (`Heavy Hit Damage` 45, `Heavy Hit Knockback` 120), isabet eden Lunge'ın toparlanması (0,4), Attendant ilk yaylım ıskalama açısı (6°), Shove bekleme süresi (1,5) ve isabet payı (40), yer değiştirme yarıçapı (800), geri çekilme arama yarıçapı (3000), spare line itme mesafesi (300), dalga yeniden deneme sayısı (6), Flood Drill düşman mesafesi (1200–2500). Hepsi ilgili veri varlığında, yorumunda "placeholder" yazıyor.

## 6. Fab / Marketplace paketlerini bağlama

| Varlık | Nereye |
|---|---|
| FP kol iskelet mesh'i | `BP_NFCharacter` → bileşen `FirstPersonArms` → Skeletal Mesh. Konumu viewport'ta kameraya göre ayarlayın. |
| FP kol AnimBP'si | `FirstPersonArms` → Anim Class. Okunabilecek durumlar: `GetWeaponComponent` → `IsAiming`, `GetAdsAlpha`, `IsReloading`; olaylar: `OnFired`, `OnReloadStateChanged`, `OnDryFire`. |
| Silahın ele oturduğu soket | Kol iskeletinde (genelde `hand_r`) bir soket açın, adını `BP_NFCharacter` → **NewFPS \| Presentation → Weapon Attach Socket Name** alanına yazın (varsayılan `GripPoint`). Soket yoksa silah `WeaponCameraOffset` ile kameraya asılır. |
| Silah modeli | `DA_Purser` / `DA_Gaffer` → **Presentation**: iskeletliyse `WeaponSkeletalMesh`, değilse `WeaponStaticMesh` (yalnız biri). İnce ayar: `WeaponAttachOffset`. |
| Namlu ucu | Silah mesh'inde namlu ucuna +X ileri bakan bir soket; adı `MuzzleSocketName` (varsayılan `Muzzle`). İz ve namlu alevi buradan çıkar; isabet yine kameradan hesaplanır. |
| Sesler | `FireSound`, `DryFireSound`, `ReloadSound`, `HitConfirmSound`, `HeadshotConfirmSound`, `KillConfirmSound`. |
| VFX (Niagara) | `MuzzleFlashFX`, `ImpactFX`, `TracerFX`. Tracer sistemi dünya uzayında bir Vector kullanıcı parametresi okumalı; adı `TracerEndParameter` (varsayılan `BeamEnd`; Niagara'da `User.BeamEnd`). |
| Üçüncü şahıs karakter | `BP_NFCharacter` → `Mesh (CharacterMesh0)` → Skeletal Mesh + AnimBP (sahibi görmez). Sonra `PlaceholderBody` bileşeninin Static Mesh'ini boşaltın. |
| Kemik bazlı kafa/uzuv vuruşu | Mesh'in Physics Asset'i olmalı. `HealthComponent` → `HeadBones` / `LimbBones` listelerini iskeletin kemik adlarına göre güncelleyin (varsayılan UE mannequin adları). Ardından kapsülde **Collision → Weapon = Ignore** yapın; yoksa iz hep kapsüle çarpar ve her vuruş "gövde" sayılır. |
| İskeletsiz hedeflerde kafa | Kafa collision bileşenine `NF.Head` (uzuv için `NF.Limb`) **Component Tag** ekleyin. |

Paketlerin Cascade (eski) parçacıkları doğrudan bağlanamaz; Niagara'ya çevrilmeli ya da Blueprint'te `OnFired` olayından oynatılmalı.

## 7. Mimari özeti

Tüm kaynak `Source/NewFPS/` altında (`Public/` başlıklar, `Private/` gövdeler). Modül bağımlılıkları: `Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `Niagara`, `AIModule`, `NavigationSystem`, `GameplayTasks`, `NetCore`.

| Sınıf | Görev |
|---|---|
| `ANFCharacter` | Kamera, FP kol/silah yuvaları, Enhanced Input, bileşenler arası bağ; yaşam döngüsü (Alive → Downed → Out). |
| `UNFCharacterMovementComponent` | Koşu ve ADS yürüyüşü `FSavedMove` bayraklarıyla; su, taşıma ve düşme durumuna göre hız ve zıplama. |
| `UNFWeaponComponent` | İki yuva + değiştirme, hitscan (saçmalı), cephane/reload/iptal, yayılım, recoil, ADS, Kedge, ıslaklık/taşıma kuralı, ağ. |
| `UNFWeaponData` (+ `UNFPurserData`, `UNFGafferData`), `UNFMeleeData`, `UNFMovementData`, `UNFRulesData` | Designer verisi. |
| `UNFHealthComponent` | Can, tek hasar hattı (`ApplyDamage`), yenilenme, kaldırma, sunucu tarafı hasar olayı. |
| `UNFInteractionComponent` | Basılı tutma etkileşimleri (sandık alma, kaldırma) ve taşıma; sunucu otoriter. |
| `ANFStrongbox`, `ANFWinchPoint`, `ANFGameState` | Ganimet, teslim ağı, replike kota sayacı ve görev sonucu. |
| `ANFTideManager`, `UNFTideData`, `UNFWetnessComponent`, `ANFSluiceZone` | Gelgit (bkz. ADR-001). |
| `ANFEnemyBase`, `ANFScuttler`, `ANFAttendant`, `ANFEnemyProjectile` | Düşmanlar: `AAIController` + C++ durum makinesi + NavMesh `MoveTo`. **Behavior Tree varlığı yok.** |
| `UNFEnemyDirector` | Dünya alt sistemi: canlı düşman sayımı ve saldırı token'ları. |
| `ANFEnemySpawner`, `ANFEnemySpawnPoint`, `UNFWaveData` | Dalga betiği, doğuş kuralları, oyuncu sayısı ölçeği. |
| `ANFGameMode`, `ANFPlayerController`, `ANFHUD`, `ANFTargetDummy` | Kurallar (kota, düşme, görev sonu, Flood Drill), Canvas HUD, gri kutu hedef. |

**Atış akışı.** Sahip istemci: `CanFire` → kameradan saçma başına iz (yayılım atış başına seed ile) → namlu-engel kontrolü → recoil + ses/VFX/iz anında → cephane tahmini → `ServerFire` (unreliable, tüm saçmalar tek pakette). Sunucu: silah durumu (derinlik/taşıma/reload/değiştirme/savurma), atış hızı (token bucket), cephane; saçma başına menzil, yayılım konisi, hedefin sınır kutusu, statik dünyaya karşı görüş hattı; saçma sayısı silahınkini aşamaz → aynı hedefe değen saçmalar **tek** `ApplyDamage` çağrısında toplanır (Attendant sendeleme eşiği atışın toplamını görür) → `ClientConfirmHit`, `MulticastFireFX`.

**Kedge.** İstemci yalnız ister (`ServerStartMelee`); hedefi sunucu, kendi bildiği bakış yönünden 0,15 sn sonra bulur. İstemci iddiası yok.

**Islaklık–silah kuralı.** Replike bayrak yok: her makine sahibin su derinliğinden (`UNFWetnessComponent`, deterministik) ve taşıma durumundan `UNFWeaponComponent::EvaluateUse` ile aynı sonucu çıkarır. Sunucu uzak istemciye 15 cm derinlik payı tanır.

**Hasar hattı.** `taban hasar × mesafe eğrisi × bölge çarpanı × DamageTakenMultiplier` → cana kırpılır. Tek fonksiyon, `LogNewFPS Verbose` ile loglanır. Saçmalı atışta mesafe ve bölge saçma başına hesaplanıp toplanır.

**Replikasyon.** Can ve "0 can" durumu herkese. Eldeki silahın cephanesi yalnız sahibine; etkin yuva herkese. ADS ve reload sahibi hariç herkese. Yaşam durumu, kanama süresi, taşınan sandık herkese; süren etkileşim yalnız sahibine. Düşmanlarda hareket (ACharacter), can ve görsel durum (`ENFEnemyVisualState` + saldırı kimliği). Su yüksekliği ve ıslaklık replike edilmez.

**Çarpışma.** `Weapon` adlı özel iz kanalı (`ECC_GameTraceChannel1`, kodda `NF_ECC_Weapon`). Düşman kapsülleri bu kanalı yok sayar; mermi yalnız isabet hacimlerine çarpar (`NF.Head` etiketli hacim = zayıf nokta).

**Artist/ses bağlama noktaları.** `UNFWeaponComponent`: `OnFired`, `OnReloadStateChanged` (false = bitti ya da iptal), `OnDryFire`, `OnWeaponSwitched`, `OnMeleeStarted`, `IsSwitching`, `IsMeleeActive`, `IsCarrying`. Düşman: `OnVisualStateChanged` / Blueprint olayı "On Visual State Changed" (durum + saldırı kimliği: Scuttler 1 = Pinch, 2 = Lunge; Attendant 1 = Punch Burst, 2 = Shove), `GetWaterState` (Scuttler), `IsSparking` (Attendant). Doğuş telgrafı: `ANFEnemySpawner::OnSpawnTelegraph`. Gelgit: `OnTidePhaseChanged`, `OnTideWarning` (`FloodT60`, `FloodT30`, `FloodT10`, `HighWaterSoon`).

## 8. Bilinen eksikler

- **Derlenmedi, çalıştırılmadı.** Bu belgedeki her şey kâğıt üzerindedir.

**Bu turda yapılmayanlar (spec'te var):**

- Coyote time, jump buffer, koşudan ateşe geçiş gecikmesi (0,20); viewmodel FOV 70; head-bob/screen shake.
- Kilitli dolap (15 sn çevirme kolu), ilk yardım kutusu, mermi sandığı aktörü (sunucu fonksiyonu hazır: `AuthAddReserveAmmo`), vinç hattına kancalanma (10:30). Teslim edilen sandık hemen sayılır ve geri alınamaz (spec: çıkışta sayılır).
- Attendant: siper kullanımı (yerine rastgele yer değiştirme), alçak siperde diz kırma, devriye, 8+ sn aynı siperdeki oyuncunun üstüne gitme, suya gönüllü girmeme (d ≥ 30) için yol maliyeti. Geri çekilme, elle konmuş noktalar yerine NavMesh'ten örneklenen en yüksek noktaya yürür.
- Scuttler: yolunu suyun içinden seçme (nav alan maliyeti). İsabet hacimleri telgraf pozuyla birlikte hareket etmez.
- Doğuş: flood'da "su çizgisinin en çok 800 cm gerisi" kuralı ve "ilk oyuncu Half'a basınca" tetiği (1:00'da doğar).
- Telgraf sesleri, bark'lar, çan, Aggie/tannoy satırları (yalnız bağlama noktaları var), telemetri CSV.
- Gamepad aim assist, tepki eğrisi, ayarlanabilir dead-zone.

**Bilinen sınırlar:**

- **Lag compensation yok.** Sunucu istemcinin isabet iddiasını toleranslarla kabul eder. Co-op/PvE için yeterli (ADR-001).
- `ServerFire` unreliable: paket kaybında atış sunucuya ulaşmayabilir; cephane ~0,6 sn sonra sunucu değerine döner. Hitmarker sunucu onayıyla gelir.
- Taşıma ve düşme hızları tahminli değil, replike duruma bağlı: sandığı alırken/bırakırken ve düşerken istemcide tek bir küçük konum düzeltmesi beklenir.
- Koşunun bel derinliğinde kesilmesi ve zıplamanın kapanması basamaklıdır (hız çarpanı değil); eşikte küçük düzeltme olabilir.
- Dost ateşi kapalı (`WeaponComponent` → `Friendly Fire`); ilk turdaki "iki oyuncu birbirini vurur" testi artık geçerli değil.
- Kedge'in itmesi ve Shove `LaunchCharacter` ile yapılır; mesafeler yaklaşıktır (balistik tahmin).
- Düşman mermisi aktör olarak doğar (havuz yok); 4 oyuncuda aynı anda en çok 12 mermi.
- Görev bitince otomatik yeniden başlamaz (`BP_NFGameMode` → `Restart Delay After Mission` ya da `nf.Mission.Restart`).
- Silah sway/bob, görsel silah tepmesi, ADS'te nişangâha hizalama, equip/reload/savurma animasyonları yok. Üçüncü şahıs silah mesh'i yok.
- HUD hata ayıklama düzeyindedir (Canvas); DPI ölçekleme yok. Performans ölçülmedi.

## 9. Derleme riskleri (ilk hata çıkarsa buraya bakın)

1. **`Source/*.Target.cs`**: `BuildSettingsVersion.Latest` / `EngineIncludeOrderVersion.Latest`; derleyici katılığı sürümle artar.
2. **`UNFWeaponComponent::RequestVisualAssets`**: `FStreamableManager::RequestAsyncLoad` imzası 5.x içinde değişti.
3. **`ANFCharacter::BuildFallbackInput`**: çalışma anında `UInputAction` / `MapKey` / modifier oluşturma. Derlenmezse gövdeyi boşaltıp 3. bölümdeki varlıkları oluşturun.
4. **`FNFSavedMove` / `GetPredictionData_Client`** ve bu turda eklenen `CanAttemptJump`, `UpdateCharacterStateBeforeMovement`, `ANFCharacter::RecalculateBaseEyeHeight` override'ları: imzalardan biri 5.8'de değiştiyse `override` hatası verir.
5. **`#include "Engine/OverlapResult.h"`** (`NFWeaponComponent.cpp`, Kedge taraması): `FOverlapResult` eski sürümlerde `Engine/EngineTypes.h` içindeydi.
6. **AI başlıkları**: `AIController.h`, `Navigation/PathFollowingComponent.h` (`EPathFollowingStatus`), `NavigationSystem.h` (`UNavigationSystemV1`, `FNavLocation`). Bağlantı hatası çıkarsa `NewFPS.Build.cs` içindeki `AIModule`, `NavigationSystem`, `GameplayTasks` satırlarına bakın.
7. **RPC parametreleri**: `MulticastFireFX(const TArray<FVector_NetQuantize>&, uint8)` ve `ServerFire(const FNFShotData&)` (içinde `TArray<FNFPelletData>`); UHT bir yapıyı reddederse hata `NFWeaponComponent.generated.h` içinde görünür.
8. **`static constexpr` sınıf sabitleri** (`UNFWeaponComponent::NumSlots`, düşman saldırı kimlikleri) ve `UNFTideData`/`UNFWaveData` kurucularındaki lambda'lar.
9. **`AActor::SetReplicateMovement`, `SetNetUpdateFrequency`, `ACharacter::LaunchCharacter`, `UProjectileMovementComponent::OnProjectileStop`**: uzun süredir kararlı, ama bu sürümde doğrulanmadı.
10. **Unity build ad çakışması**: `.cpp` dosyalarındaki yardımcı ad alanları (`NFWeapon`, `NFCharacter`, `NFEnemy`, `NFScuttlerConst` …) benzersiz tutuldu; "redefinition" hatası çıkarsa ilgili ad alanını yeniden adlandırın.
11. **Niagara** ve **`Config/DefaultEngine.ini` çarpışma profilleri**: ilk turdaki gibi. Yanlışsa derlemeyi bozmaz ama tetikleyici hacimler mermiyi durdurur.
12. **Çalışma zamanı (derleme değil)**: fare dikey yönü (`bInvertLookY`); gri kutu renkleri motorun `BasicShapeMaterial`'ındaki `Color` parametresine bağlı, yoksa düşman durum renkleri ve lamba görünmez (pozlar yine görünür); `ServerTravel("?Restart")` PIE'de çalışmayabilir.

## Git LFS (ilk ikili varlıktan önce, bir kez)

Depodaki `.gitattributes`, `.uasset`, `.umap`, FBX, doku ve ses dosyalarını Git LFS'e yönlendirir. Editörde ilk varlığı kaydetmeden önce kendi makinenizde bir kez çalıştırın:

```bash
git lfs install
```

LFS kurulu değilken eklenen ikili dosyalar depoya doğrudan girer ve sonradan taşımak zahmetlidir.

## Gelgit sistemi

Mimari ve doğrulama adımları: [adr-001-tide-and-network.md](adr-001-tide-and-network.md). Seviyeye bir `ANFTideManager` yerleştirin; `NFTideData` varlığı atanmazsa sınıf varsayılanları kullanılır. Varsayılanlar artık spec §3.2'nin 12 dakikalık, parça parça doğrusal çizelgesidir (tek sefer, döngüsüz; 5:00'te dönüş). Uyarılar ve HUD saati "sonraki çizelge satırına" değil **evre adının değişmesine** (suyun dönmesine) kalan süreyi sayar; çizelgede aynı adlı ardışık satırlar (F1–F5) tek evre olarak okunur.
