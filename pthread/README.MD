# Boid Szimuláció

Ez a projekt egy **boid algoritmust** mutat be, ahol a felhasználó irányítani tud egy boid-ot. A szimuláció modellezi a madarak kollektív viselkedését három fő irányelv  alapján: **kohézió, szeparáció és igazodás.**

## Funkciók

- Valós idejű boid szimuláció
- Billentyűzettel irányítható egy boid
- Minimalista grafikus megjelenítés SDL2-ben

## Program állapota

- [x] Boid algoritmus implementálása
- [x] Akadályok létrehozása és kezelése
- [x] Player boid implementálása
- [x] Felhasználói felület létrehozása

## Assets

- Az assetek legyenek ugyan abban a mappában, mint a többi file.
- **https://drive.google.com/drive/folders/13ERKN3sApPU-pHCgUZ5b99KhewzjwvRP?usp=sharing**

## Felhasználói felület

**player beállítás**
- peaceful mode: a player karakter nem fogja bántani a boidokat
- doom mode: a player megöli a boidokat ha hozzáérnek, **ez az alapértelmezett beállítás**

**boid beállítás**
- boid is friendly: ha a boidok látókörébe kerül a játékos, akkor megpróbálnak hozzá eljutni
- boid is natural: a boidok nem vesznek  tudomást a játékosról
- boid is scared: ha a boidok látókörébe kerül a játékos, akkor elkezdenek menekülni tőle, **ez az alapértelmezett beállítás**

## Guide (F1)

Irányítás:
- W - felfelé mozgás
- S - lefelé mozgás
- D - jobbra mozgás
- A - balra mozgás
- TAB - beállítások kinyitása/bezárása
- ENTER - beállítás mentése
- ARROW UP - a beállításokban felfelé mozgás
- ARROW DOWN - a beállításokban lefelé mozgás
- ARROW LEFT - balra mozgás a beeállítások között
- ARROW RIGHT - jobbra mozgás a beállítások között
- ESC - kilépés a programból
- F1 - használati útmutató kinyitás/becsukás
- 1. JOBB KATTINTÁS - elmenti a kör kezdő pozícióját
- 2. JOBB KATTINTÁS - elmenti a kör átmérőjét

## Párhuzamosítás

- párhuzamosítás: pthread
- párhuzamosított függvények: update_boids(), render_boids()

## Fordítás

Fordításhozs szükséges header-ök: sdl2, sdl2_ttf, pthread
```cmd
make
boid_sim.exe
