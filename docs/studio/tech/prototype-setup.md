# Prototip kurulumu: FPS çekirdek iskeleti (3C + tek silah + tek hedef)

> **Uyarı: bu kod henüz hiç derlenmedi ve editörde hiç çalıştırılmadı.**
> İskelet, Unreal Engine kurulu olmayan bir bulut ortamında yazıldı; Unreal MCP de bağlı değildi.
> İlk derlemede hata çıkması olasıdır. Hata çıktısını (Output/Build log) olduğu gibi `gameplay-developer`'a iletin.
> En riskli noktalar bu belgenin sonundaki "Derleme riskleri" bölümündedir.

Hedef motor: **UE 5.8**. Konseptten bağımsızdır; hiçbir `.uasset` içermez. Kod, hiçbir varlık oluşturmadan da oynanabilecek biçimde yazıldı (yerleşik tuş atamaları, varsayılan silah değerleri, motorun temel şekillerinden hedef).

## 1. İlk açılış

1. UE 5.8 ve C++ araç zinciri kurulu olmalı (Windows: Visual Studio 2022 + "Game development with C++"; macOS: Xcode).
2. `NewFPS.uproject` dosyasına sağ tıklayın → **Generate Visual Studio project files** (macOS: **Services → Generate Xcode Project**).
3. `NewFPS.uproject` dosyasına çift tıklayın. "Missing modules... rebuild?" sorusuna **Yes** deyin. Ya da `NewFPS.sln` içinden **Development Editor / Win64** ile derleyip çalıştırın (hata mesajları burada daha okunaklıdır).
4. Editör açılınca **Edit → Plugins → Model Context Protocol** eklentisini açıp yeniden başlatın (bkz. `docs/MCP-SETUP.md`). Eklenti `.uproject` içine bilerek yazılmadı: eklentinin iç adı doğrulanamadı.

**Motor sürümünü değiştirmek:** `NewFPS.uproject` içindeki `"EngineAssociation": "5.8"` değerini kurulu sürümle değiştirin (örn. `"5.6"`), ya da dosyaya sağ tıklayıp **Switch Unreal Engine version...** seçin. Kaynaktan derlenmiş motorlarda bu alan bir GUID olur; aynı menü onu yazar. Kod 5.1 sonrası kararlı API'lerle yazıldı, ama 5.8 dışındaki sürümlerde de denenmedi.

## 2. Hiç varlık oluşturmadan ilk deneme

1. **File → New Level → Basic**. `Content/NewFPS/Maps/L_Prototype` olarak kaydedin.
2. Haritada bir **Player Start** olduğundan emin olun.
3. **Place Actors** panelinde `NF Target Dummy` aratıp birkaç tane, 5 m / 15 m / 30 m / 60 m uzağa koyun.
4. **Play**. GameMode `Config/DefaultEngine.ini` içinde `NFGameMode` olarak ayarlı; ayrıca bir şey seçmeniz gerekmez.
5. Haritayı varsayılan yapmak için `Config/DefaultEngine.ini` içindeki iki yorum satırını (`EditorStartupMap`, `GameDefaultMap`) açın.

Yerleşik tuşlar (hiç girdi varlığı atanmadığında devreye girer):

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

Görmeniz beklenen: nişangâh (yayılımla açılıp kapanır), sağ altta cephane, sol altta can, vuruşta hitmarker (kafa: sarı, öldürme: kırmızı) ve hasar sayısı, turuncu debug izleri (`nf.Weapon.DebugShots 0` ile kapanır), hedefte kızarma + büyüme tepkisi, ölünce kaybolma ve 3 sn sonra geri gelme. Silah modeli ve ses olmayacak (varlık yok).

Hasar hattını izlemek için konsola: `Log LogNewFPS Verbose`.

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

Sonra **Input → Input Mapping Context** → `IMC_Default`:

- `IA_Move`: `W` (modifier: **Swizzle Input Axis Values**, YXZ), `S` (**Negate**, sonra **Swizzle**), `D` (yok), `A` (**Negate**), `Gamepad Left Thumbstick 2D-Axis` (**Dead Zone**). Sonuç: X = sağ, Y = ileri.
- `IA_Look`: `Mouse XY 2D-Axis`. **Negate eklemeyin**; kod X = sağ, Y = yukarı bekler. Dikey eksen tersse karakterdeki `bInvertLookY` kutusunu kullanın.
- `IA_LookStick`: `Gamepad Right Thumbstick 2D-Axis` (**Dead Zone**).
- Diğerleri: yukarıdaki tablodaki tuşlar. Trigger eklemeyin; kod `Started`/`Completed` olaylarını dinler.

### 3.2 Veri varlıkları (`Content/NewFPS/Weapons/`, `Content/NewFPS/Data/`)

Content Browser → sağ tık → **Miscellaneous → Data Asset**:

- Sınıf `NFWeaponData` → `DA_Weapon_Rifle` (**`Content/NewFPS/Weapons/` altında olmalı**; Asset Manager bu klasörü tarar, bkz. `Config/DefaultGame.ini`).
- Sınıf `NFMovementData` → `DA_Movement_Default`.

### 3.3 Blueprint sınıfları (`Content/NewFPS/Blueprints/`)

1. `NFCharacter`'dan türeyen `BP_NFCharacter`. Class Defaults:
   - **NewFPS | Input**: `Default Mapping Context` = `IMC_Default`; dokuz `... Action` alanına ilgili `IA_*`.
     (Mapping context atanıp bir action boş bırakılırsa o eylemin tuşu olmaz; log'a uyarı düşer.)
   - **NewFPS | Movement**: `Movement Data` = `DA_Movement_Default`.
   - Bileşen `WeaponComponent` → **Weapon | Weapon Data** = `DA_Weapon_Rifle`.
2. `NFGameMode`'dan türeyen `BP_NFGameMode`: `Default Pawn Class` = `BP_NFCharacter`.
3. **Project Settings → Maps & Modes → Default GameMode** = `BP_NFGameMode` (ya da haritanın World Settings → GameMode Override).

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
   - İki oyuncu birbirini vurup öldürebiliyor ve 3 sn sonra yeniden doğuyor mu? (Diğer oyuncu şimdilik gri bir silindir olarak görünür.)

## 5. Designer: hangi değer nerede

Kod derlemeye gerek yok; veri varlığını açıp değiştirin, Play'e basın.

**`DA_Weapon_Rifle` (NFWeaponData)**

| Grup | Alanlar | Not |
|---|---|---|
| Damage | `Damage`, `HeadshotMultiplier`, `LimbMultiplier`, `RangeMeters`, `DamageFalloffCurve` | Eğri: X = metre, Y = çarpan. Boş eğri = düşüş yok. |
| Fire | `RoundsPerMinute`, `bFullAuto`, `FireInputBuffer` | TTK = (öldürmek için atış − 1) × 60 / RPM. |
| Ammo | `MagazineSize`, `bInfiniteReserve`, `StartingReserveAmmo`, `ReloadTime`, `bAutoReloadOnEmpty` | |
| Spread | `HipSpread`, `AdsSpread`, `MoveSpread`, `AirborneSpread`, `SpreadPerShot`, `MaxBloom`, `SpreadRecoveryPerSecond`, `AdsSpreadModifier` | Yarım açı, derece. İlk atışta bloom yoktur. |
| Recoil | `RecoilPattern` (X = sağa yaw, Y = yukarı pitch, derece/atış), `RecoilLoopStartIndex`, `RecoilRandomness`, `AdsRecoilMultiplier`, `RecoilKickSpeed`, `RecoilRecoveryDelay`, `RecoilRecoverySpeed`, `RecoilResetTime` | Desen biterse `RecoilLoopStartIndex`'ten döner. `RecoilRecoverySpeed = 0` toparlanmayı kapatır. |
| ADS | `AdsFOV`, `AdsTime`, `AdsSensitivityScale` | Hassasiyet FOV oranına göre otomatik ölçeklenir; `AdsSensitivityScale` onun üstüne çarpandır. |

**`DA_Movement_Default` (NFMovementData):** `WalkSpeed`, `SprintSpeed`, `CrouchSpeed`, `AimWalkSpeedMultiplier`, `MaxAcceleration`, `BrakingDeceleration`, `GroundFriction`, `JumpZVelocity`, `AirControl`, `GravityScale`, `CrouchedHalfHeight`. Değerler doğuşta uygulanır; değişiklikten sonra Play'i yeniden başlatın.

**`BP_NFCharacter` → NewFPS | View:** `DefaultFOV`, `MouseSensitivity`, `GamepadLookRate`, `bInvertLookY`, `CrouchCameraInterpSpeed`. `HealthComponent` → `MaxHealth`, `DamageTakenMultiplier`.

**`BP_NFGameMode`:** `PlayerRespawnDelay`. **Hedef (haritadaki örnek ya da `NFTargetDummy` Blueprint'i):** `RespawnDelay`, `HitReactDuration`, renkler; `HealthComponent` → `MaxHealth`.

Koddaki varsayılanlar yer tutucudur (600 RPM, 25 hasar, 100 can → gövdeden 4 atış ≈ 0,3 sn). Konseptin TTK hedefi belli olunca designer bunları değiştirmeli.

## 6. Fab / Marketplace paketlerini bağlama

| Varlık | Nereye |
|---|---|
| FP kol iskelet mesh'i | `BP_NFCharacter` → bileşen `FirstPersonArms` → Skeletal Mesh. Konumu viewport'ta kameraya göre ayarlayın. |
| FP kol AnimBP'si | `FirstPersonArms` → Anim Class. Okunabilecek durumlar: `GetWeaponComponent` → `IsAiming`, `GetAdsAlpha`, `IsReloading`; olaylar: `OnFired`, `OnReloadStateChanged`, `OnDryFire`. |
| Silahın ele oturduğu soket | Kol iskeletinde (genelde `hand_r`) bir soket açın, adını `BP_NFCharacter` → **NewFPS \| Presentation → Weapon Attach Socket Name** alanına yazın (varsayılan `GripPoint`). Soket yoksa silah `WeaponCameraOffset` ile kameraya asılır. |
| Silah modeli | `DA_Weapon_Rifle` → **Presentation**: iskeletliyse `WeaponSkeletalMesh`, değilse `WeaponStaticMesh` (yalnız biri). İnce ayar: `WeaponAttachOffset`. |
| Namlu ucu | Silah mesh'inde namlu ucuna +X ileri bakan bir soket; adı `MuzzleSocketName` (varsayılan `Muzzle`). İz ve namlu alevi buradan çıkar; isabet yine kameradan hesaplanır. |
| Sesler | `FireSound`, `DryFireSound`, `ReloadSound`, `HitConfirmSound`, `HeadshotConfirmSound`, `KillConfirmSound`. |
| VFX (Niagara) | `MuzzleFlashFX`, `ImpactFX`, `TracerFX`. Tracer sistemi dünya uzayında bir Vector kullanıcı parametresi okumalı; adı `TracerEndParameter` (varsayılan `BeamEnd`; Niagara'da `User.BeamEnd`). |
| Üçüncü şahıs karakter | `BP_NFCharacter` → `Mesh (CharacterMesh0)` → Skeletal Mesh + AnimBP (sahibi görmez). Sonra `PlaceholderBody` bileşeninin Static Mesh'ini boşaltın. |
| Kemik bazlı kafa/uzuv vuruşu | Mesh'in Physics Asset'i olmalı. `HealthComponent` → `HeadBones` / `LimbBones` listelerini iskeletin kemik adlarına göre güncelleyin (varsayılan UE mannequin adları). Ardından kapsülde **Collision → Weapon = Ignore** yapın; yoksa iz hep kapsüle çarpar ve her vuruş "gövde" sayılır. |
| İskeletsiz hedeflerde kafa | Kafa collision bileşenine `NF.Head` (uzuv için `NF.Limb`) **Component Tag** ekleyin. |

Paketlerin Cascade (eski) parçacıkları doğrudan bağlanamaz; Niagara'ya çevrilmeli ya da Blueprint'te `OnFired` olayından oynatılmalı.

## 7. Mimari özeti

Tüm kaynak `Source/NewFPS/` altında (`Public/` başlıklar, `Private/` gövdeler).

| Sınıf | Görev |
|---|---|
| `ANFCharacter` | Kamera, FP kol/silah yuvaları, Enhanced Input, bileşenler arası bağ. Oynanış mantığı içermez. |
| `UNFCharacterMovementComponent` | Koşu ve ADS yürüyüşünü `FSavedMove` sıkıştırılmış bayraklarıyla (Custom_0, Custom_1) tahminli taşır. |
| `UNFWeaponComponent` | Hitscan, atış zamanlaması, cephane/reload, yayılım, recoil, ADS, ağ. |
| `UNFWeaponData`, `UNFMovementData` | Designer verisi (`UPrimaryDataAsset`). |
| `UNFHealthComponent` | Can, tek hasar hattı (`ApplyDamage`), ölüm/dirilme, replikasyon. |
| `ANFTargetDummy` | Gri kutu hedef. |
| `ANFGameMode`, `ANFPlayerController`, `ANFHUD` | Varsayılan sınıflar, oyuncu yeniden doğuşu, Canvas HUD. |

**Atış akışı.** Sahip istemci: `CanFire` → kameradan iz (yayılım atış başına seed ile) → namlu-engel kontrolü → recoil + ses/VFX/iz anında → cephane tahmini → `ServerFire` (unreliable). Sunucu: atış hızı (token bucket), cephane, reload durumu; isabet iddiası için köken mesafesi, menzil, hedefin sınır kutusu, statik dünyaya karşı görüş hattı → `UNFHealthComponent::ApplyDamage` → sahibine `ClientConfirmHit`, herkese `MulticastFireFX` (ikisi de unreliable).

**Hasar hattı.** `taban hasar × mesafe eğrisi × bölge çarpanı × DamageTakenMultiplier` → cana kırpılır. Tek fonksiyon, `LogNewFPS Verbose` ile loglanır.

**Replikasyon.** Can ve ölü durumu herkese. Cephane yalnız sahibine (`COND_OwnerOnly`). ADS ve reload durumu sahibi hariç herkese (`COND_SkipOwner`; sahibi kendi tahminini kullanır).

**Çarpışma.** `Weapon` adlı özel iz kanalı (`ECC_GameTraceChannel1`, kodda `NF_ECC_Weapon`), varsayılan yanıt Block; tetikleyici/overlap profilleri Ignore.

## 8. Bilinen eksikler

- **Derlenmedi, çalıştırılmadı.** Aşağıdaki her şey kâğıt üzerindedir.
- **Lag compensation (server rewind) yok.** Sunucu istemcinin isabet iddiasını toleranslarla kabul eder (`ServerHitBoundsTolerance` = 150 cm, vuruş bölgesi istemciden alınır). Co-op/PvE için yeterli, rekabetçi PvP için değil. Arayüz `systems-developer` ile netleştirilecek: `FNFShotData` zaman damgası + hitbox geçmişi.
- `ServerFire` unreliable: paket kaybında atış sunucuya ulaşmayabilir; cephane ~0,6 sn sonra sunucu değerine döner.
- Hitmarker sunucu onayıyla gelir (ping kadar gecikir); yerel tahminli hitmarker yok.
- Gamepad aim assist, tepki eğrisi ve ayarlanabilir dead-zone yok.
- Coyote time ve jump buffer yok. Koşudan ateşe geçişte gecikme (sprint-out) yok.
- Silah sway/bob, görsel silah tepmesi (mesh), ADS'te silahın nişangâha hizalanması yok: yalnız kamera FOV'u ve kamera recoil'i var. Bunlar AnimBP/prosedürel katman işi.
- Üçüncü şahıs silah mesh'i yok; diğer oyuncuların izi göz hizasından bir ofsetle çıkar.
- Bölge çarpanı kemik adı / bileşen etiketiyle; physical material tabanlı değil. Zırh yok (`DamageTakenMultiplier` tek sayı).
- Tek silah: silah değiştirme, envanter, cephane toplama yok. `WeaponData` çalışma anında değiştirilemez.
- Fare hassasiyeti (`MouseSensitivity` = 1.0 → fare biriminde 1 derece) tahmini bir başlangıçtır; ilk açılışta ayarlanmalı. Ayarlar menüsü yok.
- HUD hata ayıklama düzeyindedir (Canvas); DPI ölçekleme yok.
- Oyuncu ölünce gövde gizlenir; ragdoll/ölüm kamerası yok.
- Hedef vurulunca çarpışma şekli de ~%8 büyür (dedicated server hariç); gri kutu için önemsiz.
- Performans ölçülmedi. Silah bileşeni her karede tick eder (atış birikimi, recoil, ADS).

## 9. Derleme riskleri (ilk hata çıkarsa buraya bakın)

1. **`Source/*.Target.cs`**: `BuildSettingsVersion.Latest` / `EngineIncludeOrderVersion.Latest` kullanıldı. Derleyici katılığı sürümle artar; eski alışkanlıkla yazılmış bir satır uyarı-hata verebilir.
2. **`UNFWeaponComponent::RequestVisualAssets`**: `FStreamableManager::RequestAsyncLoad` imzası 5.x içinde değişti; `MoveTemp(Paths)` ile iki biçime de uyması hedeflendi.
3. **`ANFCharacter::BuildFallbackInput`**: çalışma anında `UInputAction` / `UInputMappingContext::MapKey` / modifier oluşturma. Derlenmezse en hızlı çözüm bu fonksiyonun gövdesini boşaltıp 3. bölümdeki varlıkları oluşturmaktır.
4. **`FNFSavedMove` / `GetPredictionData_Client`**: klasik kalıp; sanal fonksiyon imzalarından biri 5.8'de değiştiyse `override` hatası verir.
5. **Niagara**: `UNiagaraFunctionLibrary::SpawnSystemAtLocation`, `UNiagaraComponent::SetVectorParameter`. `Niagara` modülü `NewFPS.Build.cs` içinde ve eklenti `.uproject` içinde.
6. **`Config/DefaultEngine.ini` çarpışma profilleri** (`+EditProfiles`): elle yazıldı. Yanlışsa derlemeyi bozmaz, ama tetikleyici hacimler mermiyi durdurur; **Project Settings → Collision**'da `Weapon` kanalını ve Trigger/Overlap profillerini kontrol edin.
7. **Fare dikey yönü**: `Mouse XY 2D-Axis` Y değerinin yukarı = pozitif olduğu varsayıldı. Tersse `bInvertLookY`.
8. **Hedefin kızarması**: motorun `BasicShapeMaterial`'ında `Color` parametresi olduğu varsayıldı; yoksa yalnız büyüme tepkisi görünür.
