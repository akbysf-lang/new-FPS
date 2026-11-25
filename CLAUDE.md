# new-FPS

Unreal Engine 5 + Blender ile geliştirilen, Batılı PC oyuncusuna yönelik ticari FPS projesi. Kullanıcıyla Türkçe konuş; oyun içi metinler ve mağaza metinleri İngilizce.

## Sanal stüdyo

Proje, `.claude/agents/` altındaki 6 agent'lık ekiple yürütülür: `studio-gm`, `gameplay-developer`, `systems-developer`, `weapon-character-artist`, `environment-artist`, `game-designer`. Birden fazla disiplini ilgilendiren işlerde önce `fps-studio` skill'ini yükle ve görevi doğru agent'a devret. Ekip ve durum: [docs/studio/README.md](docs/studio/README.md).

## FPS skill'leri

`.claude/skills/` altında: `fps-studio`, `fps-game-design`, `fps-gunplay-systems`, `fps-level-design`, `fps-art-pipeline`, `fps-market-research`, `fps-narrative`. FPS ile ilgili bir işe başlamadan ilgili skill'i yükle.

## Kurallar

- Pazar verisi ve rakamlar güncel kaynakla doğrulanır, kaynak gösterilir; doğrulanamayan "tahmin" diye etiketlenir.
- Mevcut oyunların adı, karakteri ve dünyası kopyalanmaz.
- Blender/Unreal MCP yalnızca kullanıcının makinesinde uygulamalar açıkken çalışır ([docs/MCP-SETUP.md](docs/MCP-SETUP.md)); bağlantı yoksa işi betik/belge olarak hazırla ve engeli bildir.
- Tasarım kararları `docs/studio/design/`, teknik kararlar `docs/studio/tech/`, sanat kararları `docs/studio/art/` altına yazılır.
