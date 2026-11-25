---
name: fps-level-design
description: FPS seviye tasarımı — ölçüler (metrics), blockout süreci, görüş hatları ve siper, çatışma arenaları, akış ve yönlendirme, PvP harita yapısı, playtest döngüsü. Harita/seviye blockout'u yaparken, arena veya encounter alanı tasarlarken ya da bir seviyenin neden kötü oynandığını incelerken kullan. Triggers - level design, blockout, greybox, sightline, cover, arena, map layout, three-lane.
---

# FPS Seviye Tasarımı

## 1. Önce ölçüler (metrics)

Blockout'tan önce karakter ölçülerinden bir **metrics sayfası** ve test haritası (gym) oluştur. UE'de 1 birim = 1 cm. Başlangıç değerleri (karakter kapsülü ve hareket ayarlarına göre düzelt):

| Öğe | Değer |
|---|---|
| Oyuncu kapsülü | ~68 cm çap, ~176–190 cm boy |
| Göz yüksekliği | ~160–170 cm |
| Kapı | 120–150 cm genişlik, 220–250 cm yükseklik |
| Koridor (tek kişi / çatışma) | min 200 cm / 400+ cm |
| Alçak siper (çömelme) | 100–110 cm |
| Tam siper | 200+ cm |
| Basamak (otomatik çıkılan) | ≤ 45 cm |
| Zıplanabilir yükseklik / boşluk | hareket ayarından ölç, gym'de doğrula |

Gerçek dünya ölçüleri FPS'te dar hissettirir; iç mekânları ~%20–30 büyüt.

## 2. Süreç

1. **Kâğıt:** amaç, tempo eğrisi, üstten kroki, encounter listesi.
2. **Blockout (greybox):** yalnızca ilkel geometri, tek tip ızgara materyali. Sanat yok.
3. **Playtest:** oynanır hale gelir gelmez; gözlemle, yönlendirme yapma.
4. **Yinele:** akış ve çatışma oturana kadar sanat başlamaz. Oturmamış blockout'a sanat yapmak en pahalı hatadır.
5. **Art pass → ışık → optimizasyon → cila.**

## 3. Çatışma alanı

- **Siper:** yoğunluk TTK'ya bağlıdır (düşük TTK = daha çok siper). Siperden sipere koşu 2–4 sn. Yarım ve tam siperi karıştır.
- **Görüş hatları:** uzun hatları bilinçli koy ve kır. Her güçlü pozisyonun en az iki karşı açısı olsun.
- **Dikeylik:** yüksek zemin avantajdır; karşılığında açıkta kalma veya sınırlı çıkış ver.
- **Döngüler:** çıkmaz sokak yok. Her alanın en az iki giriş/çıkışı; yan yollar (flank) ana yoldan uzun ama korunaklı.
- **Arena boyutu** silah menziline uyar: pompalı arenası ile nişancı arenası aynı olamaz.
- **PvE arena:** oyuncu girişi net, düşman doğuş noktaları görüş dışında ama sesle telegraflı, mermi/can alımı riskli noktalarda (hareket ettirir).

## 4. Akış ve yönlendirme

Oyuncu "nereye gideceğim?" diye durmamalı:

- Işık, renk kontrastı, hareket (kuş, kıvılcım, bayrak), çerçeveleme, uzaktan görünen **landmark**.
- Hedef önce uzaktan gösterilir, sonra yol dolaştırılır.
- Kapılar ve tek yönlü düşüşler geri dönüşü keser ve bellek yükünü sınırlar.
- Çatışma öncesi **gözlem noktası**: oyuncu arenayı ve düşmanı görerek plan yapar.

## 5. Tempo

Seviye ritmi: giriş (sakin) → öğretim → yükselen çatışmalar → zirve → ödül/nefes. Yeni mekanik veya düşman: önce güvenli ortamda tanıt, sonra baskı altında sına, sonra başka mekanikle birleştir.

## 6. PvP harita temelleri

- Üç koridor (three-lane) yapısı okunaklıdır; koridorlar arası bağlantılar rotasyon sağlar.
- Doğuş noktaları: düşman görüş hattı dışında, doğuştan ilk temasa ~5–10 sn.
- Simetri adalet verir, asimetri karakter verir; asimetride taraf değişimi kullan.
- Güç pozisyonu ve hedef noktalar için risk/ödül dengesi; tek bir baskın nokta haritayı öldürür.
- Isı haritası (ölüm/konum) topla; sezgiye değil veriye göre düzelt.

## 7. Performans için tasarım

Görüş hattı kırıcıları aynı zamanda occlusion sağlar. Büyük açık alanlar pahalıdır; alanları portal/kapı/kıvrımla böl. World Partition ve streaming sınırlarını blockout'ta planla.

## 8. Playtest kontrol listesi

- İlk kez oynayan kaybolmadan bitirdi mi? Nerede durakladı?
- Ölümler adil miydi (oyuncu neden öldüğünü söyleyebildi mi)?
- Kullanılmayan alan / hiç gidilmeyen yol var mı?
- Baskın bir pozisyon veya strateji çıktı mı?
- Tempo: sıkılma ve bunalma anları nerede?
