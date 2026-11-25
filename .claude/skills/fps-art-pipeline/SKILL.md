---
name: fps-art-pipeline
description: FPS için sanat üretim hattı — sanat yönü ve okunabilirlik, Blender'dan UE5'e aktarım, first-person silah ve kol modelleme, karakter/düşman, modüler ortam, PBR doku ve materyal, ışık, VFX, bütçeler ve adlandırma. 3D varlık üretirken, Blender MCP ile modellerken, UE5'e import ederken veya sanat bütçesi/stil kararı verirken kullan. Triggers - weapon model, viewmodel, environment art, modular kit, texel density, PBR, Blender export, FBX, Nanite, LOD, art direction, VFX.
---

# FPS Sanat Üretim Hattı (Blender → UE5)

## 1. Sanat yönü: önce okunabilirlik

FPS'te oyuncu yarım saniyede karar verir. Öncelik sırası: **düşman > etkileşimli nesne > yol > arka plan.**

- Düşmanlar siluet ve değer (açık/koyu) kontrastıyla arka plandan ayrılır. Ekran görüntüsünü gri tonlamaya çevir: düşman hâlâ seçiliyorsa doğru.
- Renk dili tutarlı: tehlike, etkileşim, can, cephane renkleri oyun boyunca aynı.
- **Stil seçimi bir bütçe kararıdır.** Küçük ekip foto-gerçekçilikte AAA ile yarışamaz; güçlü stilizasyon (düşük poli + cesur ışık, retro, cel-shade, sınırlı palet) hem ayırt edilir hem üretimi ucuzlatır.
- Stil rehberi tek sayfa: palet, şekil dili, materyal karakteri, 3–5 referans görsel, yapılacak/yapılmayacak örnekleri.

## 2. Teknik sözleşme

| Konu | Kural |
|---|---|
| Birim | Blender Unit Scale 0.01 / Metric (veya FBX export'ta ölçek doğrulanır); UE'de 1 uu = 1 cm |
| Eksen | UE: X ileri, Z yukarı. FBX export: `-Y Forward, Z Up`; UE'de yön doğrula |
| Transform | Export öncesi Apply Scale/Rotation; pivot mantıklı yerde (kapı menteşede, silah tutuş noktasında) |
| Format | FBX (statik/iskeletli); glTF alternatif. Smoothing: Face; tangent space MikkTSpace |
| Adlandırma | `SM_` statik, `SK_` iskeletli, `T_` doku (`_D/_N/_ORM`), `M_`/`MI_` materyal, `A_` animasyon, `VFX_`, `BP_` |
| Çarpışma | Basit çarpışma `UCX_<MeshAdı>_##`; karmaşık mesh çarpışması oynanış yüzeylerinde kullanılmaz |
| Doku | ORM paketleme (R=AO, G=Roughness, B=Metallic), sRGB yalnızca renk dokusunda |

İlk varlıkta uçtan uca hattı doğrula (Blender → FBX → UE → materyal → oyun içi) ve bir **ölçek mankeni** ile kontrol et; sonra seri üretime geç.

## 3. First-person silah ve kollar

Ekranın en çok bakılan varlığı; bütçenin aslan payı buraya.

- Oyuncunun gördüğü taraf (arka/üst/sol yan) en yüksek detay; görünmeyen yüzeylerde tasarruf.
- Başlangıç bütçesi: silah ~20–60 bin üçgen, 2K–4K doku seti; kollar ~15–30 bin. Hedef donanıma göre ayarla.
- Hareketli parçalar ayrı ve kemikli: tetik, sürgü, şarjör, emniyet, horoz. Mermi ve kovan ayrı mesh.
- Soketler: `Muzzle`, `ShellEject`, `Sight`, `Grip_L`, eklenti noktaları.
- Nişangâh hizası: ADS'de gez-arpacık kamera merkezine oturmalı; modellemede kamera testiyle doğrula.
- Animasyon seti: idle, ateş (additive), reload (taktik + boş), çekme/sokma, inceleme, koşu. Reload'da oynanış olayları (şarjör çıktı/girdi) için notify noktaları bırak.
- Dünyadaki (third-person/yerde duran) sürüm ayrı, düşük bütçeli LOD'dur.

## 4. Karakter ve düşman

- Siluet önce: düşman arketipi 1 saniyede tanınmalı. Zayıf nokta görsel olarak işaretli.
- Epic iskeletiyle (Manny/Quinn) uyumlu rig, animasyon yeniden hedeflemeyi ve pazar yeri animasyonlarını kullanılabilir kılar.
- Hit-react, ölüm ve ragdoll için fizik varlığı (Physics Asset) temiz kurulmalı; hitbox'lar bununla eşleşir.
- LOD'lar ve mesafeye göre animasyon güncelleme bütçesi: ekranda 20+ düşman varsayımıyla planla.

## 5. Ortam

- **Modüler kit:** ızgaraya oturan parçalar (ör. 50/100/200/400 cm). Duvar, zemin, köşe, kapı, trim. Kit, level designer'ın metrics sayfasına uyar.
- Texel density sabit tutulur (başlangıç: ~512–1024 px/m; FP'de yakından görülen yüzeyler üst sınırda).
- Tiling materyal + trim sheet + decal + vertex paint ile çeşitlilik; her prop'a özel doku yapma.
- Nanite statik geometri için uygundur; hedef donanımda maliyetini ölç. Kullanılmıyorsa LOD zinciri zorunlu.
- Blockout onaylanmadan art pass başlamaz; siper yükseklikleri ve geçiş genişlikleri sanat aşamasında **değişmez**.

## 6. Materyal ve ışık

- Master materyal + Material Instance; her varlığa ayrı materyal grafiği yok. Shader permütasyonu ve çizim çağrısı sayısını düşük tut.
- Işık oynanışa hizmet eder: yol ve hedef aydınlık, düşman alanları kontrastlı. Tam karanlık "atmosfer" değil, okunabilirlik sorunudur.
- Lumen dinamik GI pahalıdır; hedef donanımda ölç, gerekirse baked ışık veya daha ucuz ayar.
- Post-process ölçülü: aşırı bloom, film grain, motion blur ve chromatic aberration varsayılan kapalı veya ayarlardan kapatılabilir.

## 7. VFX ve oynanış geri bildirimi

- Muzzle flash kısa (1–3 frame) ve nişangâhı kapatmayacak kadar küçük.
- Tracer, isabet efekti (yüzey türüne göre), kan/hasar göstergesi, kovan atma: hepsi gunplay hissinin parçası; designer ve gameplay developer ile birlikte ayarlanır.
- Niagara'da overdraw'a dikkat: büyük yarı saydam duman FPS'in bir numaralı GPU katilidir.
- Efektler oynanış bilgisini asla gizlemez (düşman dumanın arkasında kaybolmamalı, kasıtlı değilse).

## 8. UI/HUD

Minimal ve köşelerde: can, cephane, nişangâh, hasar yönü. Nişangâh özelleştirilebilir. Renk körü modu, altyazı, HUD ölçeği Batı pazarında beklenen erişilebilirlik asgarisidir.

## 9. Yapay zekâ ile üretilen varlıklar

Blender MCP `generate_3d` ve benzeri üretimler prototip ve arka plan için hızlıdır. Nihai üründe kullanmadan önce: topoloji/UV temizliği, stil uyumu, lisans durumu ve Steam'in yapay zekâ içerik beyanı gerekliliğini kontrol et. Batılı oyuncu kitlesinin bir kısmı üretken yapay zekâ sanatına sert tepki verir; bu bir pazarlama riskidir, GM'e bildir.

## 10. Teslim kontrol listesi

Ölçek mankeniyle doğru · pivot ve yön doğru · adlandırma kurala uygun · çarpışma var · LOD/Nanite kararı verildi · doku boyutu bütçede · materyal instance kullanıyor · oyun içinde hedef ışıkta kontrol edildi · üçgen ve çizim çağrısı bütçede.
