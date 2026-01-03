# Vodič za implementaciju - Agregator

## Opis projekta
Treba da implementiraš distribuirani servis za proračun potrošnje električne energije sa agregacijom podataka. Sistem radi u topologiji stabla gde:
- **Koren (root)** = država (glavni čvor)
- **Srednji čvorovi** = delovi države (regioni, gradovi, itd.)
- **Listovi (leaf nodes)** = potrošači

## Korak 1: Struktura fajlova

Kreiraj sledeće fajlove u folderu `Agregator/`:

1. **Node.h** - header fajl za klasu čvora (čvor stabla)
2. **Node.cpp** - implementacija klase čvora
3. **Network.h** - header fajl za mrežu/topologiju stabla
4. **Network.cpp** - implementacija mreže
5. **Consumer.h** - header fajl za potrošača
6. **Consumer.cpp** - implementacija potrošača
7. **Message.h** - header fajl za poruke između čvorova
8. **Message.cpp** - implementacija poruka
9. **Aggregator.h** - header fajl za glavnu klasu agregatora
10. **Aggregator.cpp** - implementacija agregatora
11. **main.cpp** - glavna funkcija (preimenuj Agregator.cpp u main.cpp)

## Korak 2: Šta treba u svakom fajlu

### Message.h i Message.cpp
**Message.h** treba da definiše:
- Strukturu/klasu za poruke između čvorova
- Tip poruke (REQUEST, DATA, AGGREGATE, itd.)
- Podatke koje poruka nosi (ID čvora, vrednost potrošnje, timestamp)

**Message.cpp** treba da implementira:
- Konstruktore
- Gettere i settere
- Serializaciju/deserializaciju (ako koristiš networking)

### Consumer.h i Consumer.cpp
**Consumer.h** treba da definiše:
- Klasu `Consumer` koja predstavlja potrošača (list u stablu)
- Metode:
  - `generateConsumption()` - generiše potrošnju
  - `sendConsumption()` - šalje potrošnju ka parent čvoru
  - `getId()` - vraća ID potrošača

**Consumer.cpp** treba da implementira ove metode.

### Node.h i Node.cpp
**Node.h** treba da definiše:
- Klasu `Node` koja predstavlja čvor u stablu
- Članove:
  - `nodeId` - jedinstveni ID čvora
  - `parent` - pokazivač na parent čvor
  - `children` - lista/pokazivači na child čvorove
  - `aggregatedConsumption` - agregirana potrošnja
  - `operationMode` - režim rada (AUTOMATIC ili BATCH)
- Metode:
  - `addChild(Node* child)` - dodaje child čvor
  - `receiveConsumption(double value)` - prima potrošnju od child-a
  - `aggregate()` - agregira potrošnje
  - `sendToParent()` - šalje agregirane podatke parent-u
  - `requestConsumption()` - šalje zahtev za potrošnjom
  - `setMode(int mode)` - postavlja režim rada

**Node.cpp** treba da implementira sve ove metode.

### Network.h i Network.cpp
**Network.h** treba da definiše:
- Klasu `Network` koja predstavlja celokupnu mrežu
- Članove:
  - `root` - pokazivač na korenski čvor
  - `allNodes` - mapa/list svih čvorova
  - `consumers` - lista svih potrošača
- Metode:
  - `buildTree()` - gradi stablo na osnovu konfiguracije
  - `findNode(int nodeId)` - pronalazi čvor po ID-u
  - `sendRequest(int targetNodeId)` - šalje zahtev određenom čvoru
  - `sendRequestToAll()` - šalje zahtev svim čvorovima

**Network.cpp** treba da implementira ove metode.

### Aggregator.h i Aggregator.cpp
**Aggregator.h** treba da definiše:
- Klasu `Aggregator` - glavna klasa koja upravlja sistemom
- Članove:
  - `network` - instanca Network klase
- Metode:
  - `initialize()` - inicijalizuje sistem
  - `runAutomaticMode()` - pokreće automatski režim
  - `runBatchMode()` - pokreće batch režim
  - `printTotalConsumption()` - štampa ukupnu potrošnju
  - `testWithSmallData()` - test sa malim brojem podataka
  - `testWithLargeData()` - test sa ~10,000 podataka

**Aggregator.cpp** treba da implementira sve ove metode.

### main.cpp
**main.cpp** treba da sadrži:
- `main()` funkciju
- Meni za izbor režima rada
- Pozive test funkcija
- Prikaz rezultata

## Korak 3: Redosled implementacije

### Faza 1: Osnovne strukture
1. Prvo napiši **Message.h/cpp** - osnovna struktura za komunikaciju
2. Zatim **Consumer.h/cpp** - najjednostavniji element sistema
3. Testiraj Consumer izolovano

### Faza 2: Čvorovi
4. Napiši **Node.h/cpp** - počni sa osnovnim strukturama, bez kompleksnih metoda
5. Implementiraj metode za dodavanje children i osnovnu agregaciju
6. Testiraj jedan parent sa nekoliko child čvorova

### Faza 3: Mreža
7. Napiši **Network.h/cpp** - struktura za upravljanje celom mrežom
8. Implementiraj `buildTree()` - kreiraj jednostavno stablo (npr. 1 root, 2-3 children, nekoliko consumers)
9. Testiraj kreiranje mreže

### Faza 4: Režimi rada
10. U **Node.cpp** implementiraj AUTOMATIC režim:
    - Kada consumer pošalje podatke, odmah ih prosleđuj parent-u
11. U **Node.cpp** implementiraj BATCH režim:
    - Sakupljaj podatke u određenom vremenskom intervalu
    - Kada interval istekne, objedini i pošalji parent-u
    - Koristi `std::chrono` za praćenje vremena

### Faza 5: Agregator i main
12. Napiši **Aggregator.h/cpp** - glavna logika sistema
13. Napiši **main.cpp** - korisnički interfejs
14. Testiraj oba režima

### Faza 6: Testiranje i dokumentacija
15. Implementiraj `testWithSmallData()` - 10-50 podataka
16. Implementiraj `testWithLargeData()` - ~10,000 podataka
17. Dokumentuj rezultate testiranja

## Korak 4: Tehničke napomene

### Komunikacija između čvorova
Pošto je ovo distribuirani sistem, možeš koristiti:
- **Opcija 1**: Simulacija - čvorovi pozivaju metode direktno (lakše za početak)
- **Opcija 2**: Sockets (TCP/UDP) - prava distribucija (složenije)
- Za početak preporučujem Opciju 1 (simulacija), posle možeš dodati sockets

### Režimi rada
- **AUTOMATIC**: Consumer šalje podatke odmah → Node agregira → šalje parent-u odmah
- **BATCH**: Consumer šalje podatke → Node ih čuva → nakon X sekundi agregira sve i šalje parent-u

### Struktura stabla
```
        ROOT (država)
       /    |    \
   Node1  Node2  Node3 (regioni)
   /  \    /  \    /  \
  C1  C2  C3  C4  C5  C6 (consumers)
```

### Vrednosti potrošnje
- Consumer generiše slučajnu potrošnju (npr. 10-100 kWh)
- Koristi `rand()` ili `<random>` biblioteku

## Korak 5: Checklist implementacije

- [ ] Message.h/cpp - struktura poruka
- [ ] Consumer.h/cpp - potrošač sa generisanjem i slanjem podataka
- [ ] Node.h/cpp - čvor sa agregacijom i slanjem parent-u
- [ ] Network.h/cpp - kreiranje i upravljanje mrežom
- [ ] AUTOMATIC režim - odmah slanje
- [ ] BATCH režim - agregacija u intervalu
- [ ] Aggregator.h/cpp - glavna logika
- [ ] main.cpp - korisnički interfejs
- [ ] Test sa malim brojem podataka
- [ ] Test sa ~10,000 podataka
- [ ] Dokumentacija testiranja

## Korak 6: Dodatni saveti

1. **Počni jednostavno** - prvo uradi osnovnu funkcionalnost, pa dodavaj složenost
2. **Testiraj često** - nakon svakog koraka testiraj šta si napravio
3. **Koristi debugger** - Visual Studio debugger će ti pomoći da vidiš šta se dešava
4. **Logovanje** - dodaj `std::cout` poruke da vidiš tok podataka kroz sistem
5. **Memorija** - pazljivo upravljaj pokazivačima, izbegavaj memory leaks

## Pitanja za razmišljanje

1. Kako ćeš identifikovati čvorove? (ID sistem)
2. Kako ćeš čuvati topologiju? (konfiguracija fajl, hardcodovana, itd.)
3. Kako ćeš meriti performanse? (vreme izvršavanja, memorija)
4. Kako ćeš dokumentovati testiranje? (fajl, konzola, itd.)

---

**NAPOMENA**: Ovaj vodič ti daje strukturu. Ti ćeš pisati kod u svakom fajlu. Ako ti treba pomoć sa određenim delom, pitaj!

