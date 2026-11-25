# Bağımsız derleme incelemesi (ilk derleme öncesi)

- **Tarih:** 2026-11-25
- **İnceleyen:** `systems-developer` (kodu `gameplay-developer` yazdı)
- **Kapsam:** `Source/NewFPS` (29 başlık, 25 kaynak, yaklaşık 11.800 satır), `Source/*.Target.cs`, `NewFPS.Build.cs`, `NewFPS.uproject`, `Config/*.ini`
- **Hedef motor:** UE 5.8

> **Bu inceleme derleme yerine geçmez.** İncelemenin yapıldığı ortamda Unreal Engine yok: kod **derlenmedi, UHT çalıştırılmadı, editörde açılmadı**. Buradaki her "sorun yok" hükmü okuma ve betik karşılaştırmasına dayanır.
> Motor API'lerinin 5.6–5.8 arasındaki durumu da doğrulanamadı: motor kaynağı yok, Epic'in doküman sitesi (`dev.epicgames.com`) bu oturumun ağ izinlerinin dışında. Aşağıda "doğrulanamadı" ya da "tahmin" yazan her şey bu yüzden öyle etiketli.

## Özet

- **Kesin derleme hatası bulunmadı** (0 düzeltme).
- Üç başlığa davranışı değiştirmeyen **önleyici ileri bildirim** eklendi (Bölüm 1).
- İlk derlemede hata çıkarsa en olası yerler ve hazır düzeltmeleri Bölüm 2'de, olasılık sırasıyla.
- Derleme geçtikten sonra ilk bakılacak çalışma zamanı şüphelileri Bölüm 4'te.

## Yöntem

Her başlık kendi kaynağıyla birlikte baştan sona okundu. Ayrıca depoya eklenmeyen bir Python betiği şu çapraz denetimleri yaptı:

| Denetim | Sonuç |
|---|---|
| Başlıkta bildirilen 432 üye fonksiyon ↔ `.cpp` içindeki 378 tanım (ad, parametre türleri, `const`) | Eşleşmeyen yok (fark satır içi ve saf sanal olanlar) |
| 12 RPC ↔ 12 `_Implementation` (ad ve imza) | Tam |
| 24 `Replicated`/`ReplicatedUsing` özellik ↔ 24 `DOREPLIFETIME` kaydı; `Net/UnrealNetwork.h` | Tam |
| `OnRep_*` ve 10 `AddDynamic`/`AddUniqueDynamic`/`RemoveDynamic` hedefi `UFUNCTION` mı | Hepsi `UFUNCTION` |
| `DECLARE_*_DELEGATE` parametre sayısı ↔ `Broadcast` çağrıları | Uyumlu |
| `#include "NF*.h"` hedefleri var mı; `.generated.h` son include mi; `.cpp` ilk include kendi başlığı mı | Tam |
| Unity build: dosya kapsamı `static`'ler ve ad alanı sabitleri arasında çakışma | Yok |
| `private:` altında `BlueprintReadOnly/ReadWrite` özellik (UHT hatası) | Yok |
| Süslü ayraç / parantez dengesi; `GENERATED_BODY` sayısı; override'larda `Super::` çağrısı | Tam |
| ASCII dışı karakter, CRLF | Yok |
| `Config/*.ini` içindeki `/Script/NewFPS.*` yolları ↔ sınıf adları | `NFGameMode`, `NFWeaponData` var |

Elle bakılanlar: `override` imzaları (CMC, `FSavedMove_Character`, `ACharacter`, `AHUD`, `AGameModeBase`), UHT'nin gölgeleme kuralı (üst sınıftaki bir `UPROPERTY` ile aynı adlı özellik ya da `UFUNCTION` parametresi), yerel değişkenlerin sınıf üyelerini gölgelemesi, `const` doğruluğu, erişim belirteçleri, `FMath::Max/Min/Clamp` çağrılarında karışık tür, biçim dizgisi argümanları, C++20'de kaldırılan kalıplar, `Build.cs` modülleri ↔ kullanılan başlıklar, çarpışma kanalı ↔ `NF_ECC_Weapon`.

## 1. Düzeltilen kesin hatalar

**Yok.** Aşağıdaki üç değişiklik hata düzeltmesi değil, önleyici sertleştirmedir: tür adları bugün motor başlıklarından dolaylı olarak geliyor; ileri bildirim bunu motorun include düzenine bağımlı olmaktan çıkarır. Davranış ve ikili çıktı değişmez.

| Dosya:satır | Durum | Yapılan |
|---|---|---|
| `Source/NewFPS/Public/NFHUD.h:8-10` | `DrawTide(const APawn*)` ve `HandleHitConfirmed(AActor*, …)` türleri yalnızca `GameFramework/HUD.h` üzerinden görünüyordu | `class AActor;` ve `class APawn;` eklendi |
| `Source/NewFPS/Public/NFTargetDummy.h:7` | `UFUNCTION` parametrelerindeki `AController*` yalnızca `GameFramework/Actor.h` üzerinden görünüyordu | `class AController;` eklendi |
| `Source/NewFPS/Public/NFEnemyDirector.h:8-10` | Başlık yalnızca `CoreMinimal.h` + `WorldSubsystem.h` içeriyor; `AActor*` ve `const UObject*` kullanıyor | `class AActor;` ve `class UObject;` eklendi |

## 2. İlk derlemede doğrulanmalı (olasılık sırasıyla)

Olasılıklar tahmindir. Her madde için: belirti, yer, hazır düzeltme.

### 2.1 Enhanced Input: çalışma anında eşleme kurma (orta)

- **Yer:** `NFCharacter.cpp:709-808` (`BuildFallbackInput`), özellikle `:760` `Context->MapKey(Action, Key)` ve `Mapping.Modifiers.Add(...)`.
- **Neden riskli:** Web aramasında, `UInputMappingContext::Mappings` dizisinin kullanımdan kaldırılıp `DefaultKeyMappings` yapısına taşındığı görülüyor (sürüm numarası doğrulanamadı). Kod `Mappings`'e doğrudan dokunmuyor, yalnızca `MapKey` çağırıyor; `MapKey`'in 5.8'deki imzası ve dışa aktarımı doğrulanamadı.
- **Belirti:** `MapKey` için C2039 / C2440 ya da bağlayıcıda `LNK2019 ... UInputMappingContext::MapKey`.
- **Hazır düzeltme:** `BuildFallbackInput` gövdesini `EnsureAction` çağrılarından sonra `return;` ile kesin (eylemler yine oluşur, tuş ataması olmaz) ve `prototype-setup.md` Bölüm 3'teki girdi varlıklarını editörde oluşturup `BP_NFCharacter`'a atayın. Sistem tarafı sonra yeni API'ye uyarlar.

### 2.2 `double` → `float` örtük daraltma uyarıları (düşük-orta)

- **Yer:** Onlarca satır; kalıp her yerde aynı: `const float Now = World->GetTimeSeconds();`, `const float Speed = Velocity.Size2D();`, `AddMovementInput(..., Axis.Y)`. Örnekler: `NFWeaponComponent.cpp:187, 342, 781, 977, 1355`, `NFCharacter.cpp:832-848`, `NFHUD.cpp:40, 63`, `NFHealthComponent.cpp:76`, `NFStrongbox.h:36`.
- **Neden riskli:** Bu dönüşümler UE5'te varsayılan olarak uyarı üretmez (Epic'in kendi şablonları da böyle yazılı). Ama `Target.cs` içindeki `BuildSettingsVersion.Latest`, 5.8'in en katı varsayılanlarını alır; 5.8'de `UnsafeTypeCastWarningLevel` varsayılanının yükseltilip yükseltilmediği doğrulanamadı.
- **Belirti:** Çok sayıda `C4244` (MSVC) ya da `-Wimplicit-float-conversion` / `-Wshorten-64-to-32` (Clang), "treated as error".
- **Hazır düzeltme:** `Source/NewFPS/NewFPS.Build.cs` kurucusuna tek satır (özellik adı sürüme göre değişir; UBT hangisini kabul ederse):

```csharp
// UE 5.6 ve sonrası (tahmin: ad doğrulanamadı)
CppCompileWarningSettings.UnsafeTypeCastWarningLevel = WarningLevel.Off;
// daha eski adlandırma
UnsafeTypeCastWarningLevel = WarningLevel.Off;
```

Bu yalnızca bu modülü etkiler. Kalıcı çözüm (satır satır `static_cast<float>`) ayrı iş olarak açılır.

### 2.3 `FStreamableManager::RequestAsyncLoad` aşırı yükleme çözümü (düşük-orta)

- **Yer:** `NFWeaponComponent.cpp:1510-1512`.
- **Belirti:** "no matching function" / "ambiguous call".
- **Hazır düzeltme:** `MoveTemp(Paths)` yerine `Paths` geçin. Yine olmazsa:

```cpp
VisualAssetsHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
	Paths, FStreamableDelegate::CreateUObject(this, &UNFWeaponComponent::HandleVisualAssetsLoaded),
	FStreamableManager::DefaultAsyncLoadPriority);
```

### 2.4 `override` imzaları (düşük)

- **Yer:** `NFCharacterMovementComponent.h:27-30, 79, 89-93, 106`; `NFCharacter.h:35-42`.
- **Durum:** İmzalar bildiğim 5.x başlıklarıyla birebir (`SetMoveFor(ACharacter*, float, FVector const&, FNetworkPredictionData_Client_Character&)`, `CanCombineWith(const FSavedMovePtr&, ACharacter*, float) const`, `CanAttemptJump() const`, `UpdateCharacterStateBeforeMovement(float)`, `RecalculateBaseEyeHeight()`, `OnStartCrouch(float, float)`). 5.8'de değişmediği doğrulanamadı.
- **Belirti:** `C3668: method with override specifier 'override' did not override any base class methods`.
- **Hazır düzeltme:** Hata satırındaki fonksiyonun imzasını motor başlığından (`CharacterMovementComponent.h` / `Character.h`) kopyalayın; gövde değişmez.

### 2.5 Erişimi daraltılmış motor üyeleri (düşük)

Motor, doğrudan erişilen üyeleri zamanla `private` yapıp erişimci ekliyor. Kod birkaç üyeye doğrudan erişiyor:

| Kullanım | Yer | Hata çıkarsa |
|---|---|---|
| `bReplicates = true;` | `NFStrongbox.cpp:12`, `NFTideManager.cpp:73`, `NFSluiceZone.cpp:11`, `NFTargetDummy.cpp:21`, `NFEnemyProjectile.cpp:15`, `NFEnemySpawner.cpp:150` | `SetReplicates(true);` |
| `Controller` (APawn üyesi) | `NFCharacter.cpp:812, 818, 821` | `GetController()` |
| `PlayerOwner` (AHUD üyesi) | `NFHUD.cpp:73-75` | `GetOwningPlayerController()` |
| `BaseEyeHeight`, `CrouchedEyeHeight` | `NFCharacter.cpp:57-58, 230-232, 242` | Hata mesajındaki erişimciyi kullanın |
| `Movement->MaxWalkSpeed`, `RotationRate` vb. | `NFCharacter.cpp:205-213`, düşman kurucuları | Aynı |

**Belirti:** `C2248: cannot access private member`. Uyarı (`C4996`, deprecated) hata değildir; derleme sürer.

### 2.6 Gölgeleme uyarıları hata olarak (düşük)

- **Durum:** Yerel değişkenlerin kendi sınıflarının ve bilinen motor üst sınıflarının üyelerini gölgelemediği elle denetlendi (yazar `InHealthComponent`, `NFGameState`, `Numbers` gibi adlarla bundan bilinçli kaçınmış). Risk, 5.8'de bir üst sınıfa aynı adla yeni üye eklenmiş olması.
- **Aday adlar:** `Movement` (`NFEnemyBase.cpp:44, 251, 280`, `NFAttendant.cpp:46`, `NFScuttler.cpp:47`, `NFCharacter.cpp:197, 241`), `Controller` (`NFWeaponComponent.cpp:317`, `NFGameMode.cpp:181, 192`), `Pawn` (`NFGameMode.cpp:341`, `NFEnemySpawner.cpp:273`, `NFHUD.cpp:54`), `Owner` (yalnızca bileşenlerde).
- **Belirti:** `C4458: declaration of 'X' hides class member` (ya da `C4456`, `C4459`, Clang'de `-Wshadow`).
- **Hazır düzeltme:** Yerel değişkeni yeniden adlandırın (`Movement` → `MoveComp` gibi). Davranış değişmez.

### 2.7 Eksik include / ileri bildirim (düşük)

- **Neden riskli:** `EngineIncludeOrderVersion.Latest`, motor başlıklarının dolaylı içerdiklerini azaltır. Her `.cpp`'nin kullandığı türler için include'lar tek tek kontrol edildi, eksik görülmedi; ama dolaylı içerme ancak gerçek derlemede kesinleşir. Unity build bazı eksikleri gizler.
- **Belirti:** `C2027: use of undefined type`, `C2065: undeclared identifier`, `C2079`.
- **Hazır düzeltme:** Hata veren `.cpp`'nin include bloğuna adı geçen türün başlığını ekleyin. En olası adaylar: `Engine/World.h`, `GameFramework/Controller.h`, `GameFramework/Pawn.h`, `Engine/EngineTypes.h`, `Engine/TimerHandle.h`.

### 2.8 UHT'nin `UCLASS` gövdesindeki yerel C++ yapılarını ayrıştırması (düşük)

- **Yer:** `static constexpr` üyeler (`NFWeaponComponent.h:105`, `NFAttendant.h:47-48`, `NFScuttler.h:49-50`); süslü ayraçlı dizi başlatıcı (`NFWeaponComponent.h:432-433`); `UCLASS` içinde yerel `enum class` / `struct` (`NFAttendant.h:91`, `NFScuttler.h:93`, `NFEnemySpawner.h:221-232`, `NFEnemyDirector.h:47`).
- **Belirti:** Derleyiciden önce, `UnrealHeaderTool` aşamasında `Error: ...` (dosya ve satır `.h` içinde).
- **Hazır düzeltme:** Dizi başlatıcıyı kaldırıp (`int32 StoredMagazine[NumSlots];`) kurucuda sıfırlayın; `static constexpr` sabiti `.cpp`'deki ad alanına taşıyın; yerel `enum`/`struct`'ı sınıfın dışına, başlığın üstüne alın.

### 2.9 Tekil satırlar (çok düşük)

| Yer | Olası hata | Hazır düzeltme |
|---|---|---|
| `NFEnemySpawner.cpp:74` `PointGroup ? FName(PointGroup) : NAME_None` | Üçlü ifadede tür uyuşmazlığı | `: FName(NAME_None)` |
| `NFWeaponComponent.cpp:1445` `SetVectorParameter` | Üye yok / deprecated | `SetVariableVec3(Data->TracerEndParameter, TraceEnd)` |
| `NFHealthComponent.cpp:66` `GetNameSafe(DamageInfo.Instigator)` | `TObjectPtr` ile belirsiz aşırı yükleme | `DamageInfo.Instigator.Get()` |
| `NFInteractionComponent.h:56`, `NFStrongbox.h:30` (ileri bildirilmiş türle satır içi `TObjectPtr` dönüşü) | "incomplete type" | Gövdeyi `.cpp`'ye taşıyın |
| `NFTideManager.cpp:76` `SetNetUpdateFrequency` | Üye yok (5.5 öncesi motor) | `NetUpdateFrequency = 2.f;` |
| `NFCharacter.cpp:324, 340` `Set/GetSkeletalMeshAsset` | Üye yok (5.1 öncesi motor) | Motor sürümünü yükseltin |
| `Source/*.Target.cs` `BuildSettingsVersion.Latest` | "modifies the values of properties … unique build environment" | Mesajın önerdiği sabit sürümü yazın (5.7 için `BuildSettingsVersion.V6`; 5.8 karşılığı doğrulanamadı) |

### 2.10 Bağlayıcı hataları (çok düşük)

- **Durum:** `Build.cs` modülleri (`Core`, `CoreUObject`, `Engine`, `InputCore`, `EnhancedInput`, `Niagara`, `AIModule`, `NavigationSystem`, `GameplayTasks`, `NetCore`) kullanılan başlıkların hepsini karşılıyor; eksik modül görülmedi.
- **Belirti:** `LNK2019: unresolved external symbol`.
- **Hazır düzeltme:** Sembol bir motor sınıfına aitse o sınıfın modülünü `Build.cs`'e ekleyin; modül zaten varsa fonksiyon dışa aktarılmıyordur, çağrıyı değiştirmek gerekir (sistem tarafına iletin).

## 3. İlk derleme kontrol listesi

### 3.1 Hazırlık

1. Kurulu motor sürümünü doğrulayın. 5.8 değilse `NewFPS.uproject` → sağ tık → **Switch Unreal Engine version…**
2. Visual Studio 2022: "Game development with C++" iş yükü, güncel MSVC ve Windows SDK kurulu olsun.
3. Editör kapalı olsun. Depoda `Binaries/`, `Intermediate/` varsa silin (ilk derlemede olmamalı).

### 3.2 Derleme (bu sırayla)

1. `NewFPS.uproject` → sağ tık → **Generate Visual Studio project files**. Burada hata çıkarsa sorun `Target.cs` / `Build.cs` içindedir; pencere çıktısının tamamını iletin.
2. `NewFPS.sln`'i açın. Yapılandırma: **Development Editor / Win64**. **Build → Build Solution**. `.uproject`'e çift tıklayıp "rebuild?" demeyin: hata metni orada kesilir.
3. Derleme şu aşamalardan geçer; hata hangi aşamadaysa o bölüme bakın:

| Aşama | Hata nasıl görünür | Bölüm |
|---|---|---|
| UBT kuralları | `Target.cs` / `Build.cs` satırı, `error CS…` | 2.2, 2.9 |
| UnrealHeaderTool | `NF….h(satır): Error: …` | 2.8 |
| C++ derleme | `NF….cpp(satır): error C….` | 2.1–2.7, 2.9 |
| Bağlama | `error LNK2019 / LNK2001` | 2.1, 2.10 |

4. **Yalnızca ilk hatayı düzeltin, yeniden derleyin.** Sonraki hataların çoğu ilkinin zinciridir.

### 3.3 Hata çıkarsa ne iletilecek

- **Output** penceresi (Show output from: **Build**). **Error List** sekmesi değil: orada `note:` satırları ve şablon açılımı görünmez.
- İlk `error` satırından başlayarak 40–50 satır: dosya yolu, satır numarası, hata kodu ve altındaki `note:` satırlarıyla.
- UHT hatasıysa UHT'nin yazdığı bütün `Error:` satırları; bağlayıcı hatasıysa bütün `LNK` satırları.
- Motorun tam sürümü (ör. 5.8.0) ve derleyici sürümü (çıktının başında yazar).
- Tam log gerekirse: kurulu motorda genellikle `%LOCALAPPDATA%\UnrealBuildTool\Log.txt` (konum sürüme göre değişebilir).
- Kime: derleme/araç zinciri hataları `systems-developer`, oynanış kodu gövdesindeki hatalar `gameplay-developer`.

### 3.4 Derleme geçtikten sonra

1. Editörü açın, **Output Log**'da şunları arayın: `LogClass: Error`, `LogUObjectGlobals`, `Failed to find`, `LogNewFPS`. Beklenen tek uyarı: Asset Manager'ın `/Game/NewFPS/Weapons` klasörünü bulamaması (klasör henüz yok).
2. İkinci tur, eksik include'ları açığa çıkarmak için unity kapalı derleme (yol sizin kurulumunuza göre):

```bat
"<UE>\Engine\Build\BatchFiles\Build.bat" NewFPSEditor Win64 Development -Project="<depo>\NewFPS.uproject" -WaitMutex -DisableUnity
```

3. Üçüncü tur, paketleme öncesi: aynı komut `NewFPS Win64 Shipping` hedefiyle. Editörde derlenen kod Shipping'de derlenmeyebilir (hata ayıklama çizimleri, log makroları).
4. `prototype-setup.md` Bölüm 2'deki duman testi; ardından PIE'de 2 oyuncu, `Net PktLag=150`, `Net PktLoss=2`.

"Tamamlandı" sayılması için: Development Editor + unity kapalı + Shipping üçü de derlenmiş, tek oyuncu duman testi ve 2 istemcili gecikmeli deneme yapılmış olmalı.

## 4. Derleme geçtikten sonra ilk bakılacak şüpheli davranışlar

Bunlar kod okumasından çıkan şüphelerdir, gözlem değildir. Sıra: görülme olasılığı ve etkisi.

| # | Şüphe | Yer | Nasıl bakılır |
|---|---|---|---|
| 1 | **Düşmanlar doğuyor ama yürümüyor.** NavMesh yoksa `MoveTo` sessizce başarısız olur; `AuthSpawnGroupAround` hiç doğurmaz. | `NFEnemyBase.cpp:383-398`, `NFEnemySpawner.cpp:448` | Seviyede `Nav Mesh Bounds Volume`, **P** ile yeşil alan |
| 2 | **Fare bakış hızı ve dikey yönü.** Ham `Mouse2D` deltası doğrudan dereceye çevriliyor (`MouseSensitivity = 1`). Legacy ölçek kapalı; hız çok yüksek ya da düşük çıkabilir, dikey ters olabilir. | `NFCharacter.cpp:836-841`, `Config/DefaultInput.ini` | 800 DPI'da 360° için fare mesafesi; `bInvertLookY` |
| 3 | **Gri kutu renkleri görünmüyor.** Renkler motorun `BasicShapeMaterial`'ındaki `Color` parametresine yazılıyor; ad farklıysa lamba ve durum renkleri değişmez. Paketlenmiş build'de `/Engine/BasicShapes/*` cook edilmemiş olabilir. | `NFEnemyBase.cpp:20`, `NFTargetDummy.cpp:14` | Attendant lambası telegrafta kırmızı oluyor mu; paketli build'de düşman görünüyor mu |
| 4 | **Sahip istemcide tuşlar çalışmıyor / iki kez çalışıyor.** Yedek girdi eşlemesi pawn başına çalışma anında kuruluyor; yeniden doğuşta eski bağlam kaldırılıp yenisi ekleniyor. | `NFCharacter.cpp:507-519, 612-630, 709-808` | Tek oyuncu, sonra bleed-out → lifeline dönüşü sonrası tuşlar; listen host ve uzak istemci ayrı ayrı |
| 5 | **Mermi tetikleyici hacimlerde ya da su düzleminde duruyor.** `Weapon` kanalının varsayılanı Block; yalnızca beş profil Ignore'a çekildi. Özel profilli hacimler mermiyi durdurur. | `Config/DefaultEngine.ini:7-14` | `nf.Weapon.DebugShots 1` (varsayılan açık), izler nerede bitiyor |
| 6 | **Uzak istemcide isabetler reddediliyor.** Sunucu doğrulaması toleranslarla çalışıyor (köken 250 cm, hedef kutusu 150 cm, koni +2°); 150 ms'de hareketli hedefte reddedilme oranı bilinmiyor. İsabet onayı güvenilmez RPC, kayıpta vuruş işareti düşer. | `NFWeaponComponent.cpp:811-1005` | Sunucuda `log LogNewFPS Verbose`, "Hit rejected" satırları; `Net PktLag=150` |
| 7 | **Cephane sayacı geri sıçrıyor.** Sahip istemci cephaneyi öngörüyor, 0,6 sn sonra sunucu değerini alıyor; hızlı ateş + yüksek gecikmede sayaç oynayabilir. Geç katılan istemcide sunucu değeri sınıf varsayılanına eşitse ilk değer hiç gelmez. | `NFWeaponComponent.cpp:62-94, 137-154` | 150 ms + %5 kayıpta şarjör boşalt; oyun ortasında katıl |
| 8 | **Su eşiğinde hareket düzeltmesi (rubber-band).** Su yüksekliği her makinede sunucu saati tahmininden türetiliyor; koşunun bel derinliğinde kesilmesi ve zıplamanın kapanması basamaklı. Taşıma ve downed durumu istemciye bir gecikme geç ulaşıyor. | `NFCharacterMovementComponent.cpp:22-146`, `NFTideManager.cpp:192-232` | İki istemcide `nf.Tide.Debug 1` satırını karşılaştır; `p.NetShowCorrections 1` ile eşikte yürü |
| 9 | **Çömelmede kamera sıçraması.** Kamera kapsüle bağlı; yumuşatma `OnStartCrouch` içindeki elle hesaplanan ofsete dayanıyor. Hesap kâğıt üzerinde tutuyor (74 → 49, kapsül −35, ofset +60) ama simüle vekillerde ve düzeltme anında denenmedi. | `NFCharacter.cpp:238-307` | Çömel/kalk, yürürken ve 150 ms'de |
| 10 | **Dalga betiği başlangıcı.** Spawner `BeginPlay`'de betiği o anki gelgit saatine sarıyor; gelgit yöneticisi daha sonra `BeginPlay` olursa saat 0 okunur. `Start Elapsed Seconds` sıfırdan farklıysa geçmiş dalgaların hepsi ilk yoklamada birden doğar. | `NFEnemySpawner.cpp:162-175`, `NFTideManager.cpp:148-161` | `Start Elapsed Seconds = 300` ile başlat, ilk saniyede kaç düşman doğuyor |
| 11 | **Düşman mermisi oyuncuya değmiyor ya da duvardan geçiyor.** Çarpışma yalnız sorgu; isabet overlap'e, durma sweep'e bağlı. | `NFEnemyProjectile.cpp:11-68` | Attendant'ın önünde dur; ince duvar arkasında dur |
| 12 | **Kedge ıskalıyor.** Hedef, sunucunun bakış yönü ve hedef kutusunun merkezine göre seçiliyor; uzak istemcide pitch sıkıştırılmış geliyor. | `NFWeaponComponent.cpp:1128-1228` | Uzak istemciyle Scuttler'a yukarıdan ve yandan vur |
| 13 | **`nf.Mission.Restart` PIE'de çalışmıyor.** `ServerTravel("?Restart")` PIE'de güvenilir değil. | `NFGameMode.cpp:286-292` | Standalone ya da paketli build'de dene |
| 14 | **Asset Manager uyarıları.** Yalnız `NFWeapon` türü kayıtlı ve klasörü yok; diğer veri türleri (`NFMovement`, `NFRules`, `NFTide`, `NFMelee`, `NFEnemy`, `NFWaves`) kayıtlı değil. Sert referansla atanan varlıklar çalışır; kimlikle yükleme ve cook kuralları çalışmaz. | `Config/DefaultGame.ini:6` | Editör açılış logu; paketlemeden önce karar verilmeli |

Ölçülmeyenler: frame süresi, replikasyon bant genişliği, yükleme süresi. Hata ayıklama izleri (`nf.Weapon.DebugShots`) varsayılan açık; ölçüm yapmadan önce kapatın.

## 5. İncelemenin kapsamadığı yerler

- **Gerçek derleme, UHT ve bağlama.** Hiçbiri çalıştırılmadı.
- **UE 5.6–5.8 API değişiklikleri.** Motor kaynağı ve resmî doküman erişimi yoktu; imzalar 5.x bilgisine dayanıyor.
- **Derleyiciye özgü davranış.** MSVC ile Clang arasındaki farklar (macOS, Linux, Steam Deck araç zinciri) denenmedi.
- **Unity kapalı, Shipping, Test, paketleme, cook.** Yalnızca okuma düzeyinde düşünüldü.
- **Blueprint ve varlık tarafı.** Depoda `.uasset` yok; `BP_NFCharacter` gibi türevlerin davranışı kapsam dışı.
- **Çalışma zamanı doğruluğu ve performans.** Bölüm 4 şüphe listesidir, test sonucu değildir. Oynanış mantığı ve denge incelenmedi.
- **Mekanik betiğin sınırı.** Regex tabanlıdır: şablon çıkarımı, aşırı yükleme çözümü, örtük dönüşüm ve makro açılımı hatalarını göremez.

## Kaynaklar

Sürüme özgü iki bilgi web aramasıyla kısmen doğrulandı; resmî doküman sayfaları bu oturumdan açılamadı.

- 5.7 akışında `BuildSettingsVersion.V6` bulunduğu: [Standalone UE programs no longer compile in 5.7](https://forums.unrealengine.com/t/standalone-ue-programs-no-longer-compile-in-5-7/2719544)
- `InputMappingContext.mappings` için "deprecated: Use the DefaultKeyMappings struct instead" notu (sürüm belirtilmemiş): [Python API: InputMappingContext](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/InputMappingContext)
- `UnsafeTypeCastWarningLevel` tanımı (varsayılan değeri bulunamadı): [Build Configuration for Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/build-configuration-for-unreal-engine)
