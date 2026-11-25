# ADR-001: Gelgit mimarisi ve ağ modeli

**Durum:** Kabul edildi (prototip) · **Tarih:** 25 Kasım 2026 · **Not:** Kod bu ortamda derlenmedi ve çalıştırılmadı; aşağıdaki davranışlar tasarım niyetidir, ölçülmüş sonuç değildir.

## Bağlam

EBBSHIFT 1–4 kişilik co-op PvE bir FPS'tir. Görev saati gelgittir: tek bir su düzlemi çizelgeye göre yükselir ve alçalır; karakterler ve düşmanlar suyun derinliğine göre etkilenir.

## Karar 1: Su yüksekliği replike edilmez, hesaplanır

- `ANFTideManager` seviyede tek aktördür. Yalnızca gelgit saatinin başlangıç bilgisini replike eder.
- Her makine su yüksekliğini sunucu dünya zamanından ve `UNFTideData` çizelgesinden kendisi hesaplar (`UNFTideData::Evaluate`). Aynı girdi, her makinede aynı çıktı.
- Islaklık (`UNFWetnessComponent`: Dry / Knee / Waist / Overhead) replike edilmez; karakterin konumu zaten replike olduğu için her makine kendi hesaplar. Tick yerine 10 Hz zamanlayıcı kullanır.
- Sudaki yavaşlama, hareket bileşeninde derinliğin sürekli bir fonksiyonudur. Basamaklı olsaydı istemci tahmini ile sunucu birkaç santim farkla farklı hız hesaplar ve düzeltme (lastik bant) üretirdi.
- Savakla bölgesel gecikme için `ANFSluiceZone` yerel su üst sınırı verir; yönetici `GetWaterZAt(konum)` ile sorgulanır.

**Reddedilen:** su yüksekliğini her kare replike etmek (gereksiz bant genişliği, istemcide titreme); ıslaklığı RPC ile bildirmek (hileye açık, gecikmeli).

**Açık nokta:** spesifikasyon hız düşüşünü basamaklı tanımlıyor (×0,75 / ×0,5); kod eşikler arasında enterpolasyon yapıyor. Playtest'te fark hissedilirse geçiş aralığı (`WadeBlendDepth`) daraltılır, basamağa dönülmez.

## Karar 2: Co-op listen server, özel sunucu yok

- Bir oyuncu sunucudur; diğerleri Steam oturumu ve arkadaş davetiyle katılır. Özel sunucu ve eşleştirme servisi yok: maliyeti ve işletmesi 6 kişilik ekibe uygun değil.
- Atışta sunucu; atış hızı, cephane, menzil ve görüş hattını doğrular. Lag compensation (sunucuda geri sarma) yok: PvE'de hedefler yapay zekâ olduğu ve rakip oyuncu bulunmadığı için istemcinin isabet iddiası toleransla kabul edilir.

**Riskler:**

- Sunucu olan oyuncunun bağlantısı herkesin deneyimini belirler; sunucu oyundan çıkarsa oturum biter (host migration yok).
- Toleranslı doğrulama hileye açıktır. Co-op'ta kabul edilebilir; ileride rekabetçi bir mod istenirse geri sarma eklenmeden açılmamalı.
- Dört oyuncu ve kalabalık düşman sürüsünde replikasyon yükü ölçülmedi.

## Karar 3: Hedef donanım (öneri, ölçülmedi)

- Hedef: orta sınıf masaüstü GPU'da 1080p / 60 fps ve Steam Deck'te oynanabilir kare hızı.
- Lumen, Nanite ve Virtual Shadow Maps kararı ölçümle verilecek. Stil rehberi dikey dilimde Nanite kapalı ve Lumen kapalıyken de okunan ışık öneriyor.
- Ölçüm planı: ilk gri kutu haritada `stat unit` ve `stat gpu` ile dört yapılandırma (Lumen açık/kapalı × VSM açık/kapalı), 4 oyuncu ve hedef düşman sayısıyla.

## Doğrulama (kullanıcının makinesinde yapılacak)

1. İki istemciyle oyna; `nf.Tide.Debug 1` ile iki ekranda su yüksekliğinin ve gelgit saatinin eşleştiğini karşılaştır.
2. `Net PktLag=120` ve `Net PktLoss=3` altında diz ve bel derinliğinde yürü; lastik bant olup olmadığına bak.
3. Geç katılan istemcide su yüksekliğinin doğru başladığını doğrula.
