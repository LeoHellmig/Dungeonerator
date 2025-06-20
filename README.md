Dungeonerator

Exmaple usage:

```cpp
#include "dungeonerator.hpp"

int main() {
  DungeonGenerator::GenerationData generationData(30, 5, 1.0, {1.0f, 1.0f}, {100.0f, 100.0f}, false, true, 0.3f);
  DungeonGenerator::Dungeon myDungeon(generationData);

  return 0;
}
```
