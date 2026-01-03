# Kako da počneš da kucaš kod

## Korak 1: Dodaj fajlove u Visual Studio projekat

U Visual Studio-u:

1. Desni klik na projekat "Agregator" u Solution Explorer-u
2. Izaberi "Add" → "Existing Item..." (ili "Add Existing Item...")
3. Dodaj sve nove fajlove (.h i .cpp) koje sam kreirao:
   - Message.h
   - Message.cpp
   - Consumer.h
   - Consumer.cpp
   - Node.h
   - Node.cpp
   - Network.h
   - Network.cpp
   - Aggregator.h
   - Aggregator.cpp
   - main.cpp

4. **VAŽNO**: U projektu već postoji `Agregator.cpp` koji ima `main()` funkciju. Ili:
   - Obriši sadržaj `Agregator.cpp` i koristi `main.cpp`, ILI
   - Obriši `Agregator.cpp` i koristi `main.cpp` kao glavni fajl

## Korak 2: Redosled rada - Šta gde kucaš PRVO

### 1. Message.h i Message.cpp
**POČNI OVDE!**

U `Message.h`:
- Odkomentariši primer strukture ili napravi svoju
- Definiši enum za tipove poruka
- Definiši strukturu/klasu za poruke

U `Message.cpp`:
- Implementiraj osnovne konstruktore
- Implementiraj gettere i settere

**Test**: Možeš napraviti jednostavan test u `main()` da proveriš da li poruka radi.

### 2. Consumer.h i Consumer.cpp
U `Consumer.h`:
- Odkomentariši include-ove koje trebaš
- Proveri da li su svi potrebni članovi definisani

U `Consumer.cpp`:
- Implementiraj konstruktor (postavi ID)
- Implementiraj `generateConsumption()` - koristi `rand()` ili `<random>`
  ```cpp
  // Primer:
  #include <random>
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(10.0, 100.0);
  return dis(gen);
  ```
- Implementiraj `getId()` - samo vrati `consumerId`
- `sendConsumption()` za sada može biti prazan (ili samo `std::cout` za test)

**Test**: U `main()` kreiraj Consumer i proveri da li generiše potrošnju.

### 3. Node.h i Node.cpp
U `Node.h`:
- Proveri da li su svi članovi definisani
- Dodaj include-ove ako treba

U `Node.cpp`:
- Implementiraj konstruktor (inicijalizuj sve članove)
- `addChild()` - dodaj u `children` vektor, pozovi `child->setParent(this)`
- `setParent()` - već je implementirano (samo `parent = p`)
- `getId()` i `getAggregatedConsumption()` - samo return vrednosti
- `receiveConsumption()` - za sada samo dodaj u `aggregatedConsumption` (jednostavno sabiranje)
- `aggregate()` - saberi sve iz children (za sada može biti prazno)
- `sendToParent()` - ako parent postoji, pozovi `parent->receiveConsumption(aggregatedConsumption)`
- `requestConsumption()` - za sada može biti prazno
- `processBatch()` - za sada može biti prazno

**Test**: Kreiraj 1 parent čvor i 2-3 child čvora, pošalji podatke i proveri agregaciju.

### 4. Network.h i Network.cpp
U `Network.cpp`:
- Implementiraj konstruktor (samo `root = nullptr`)
- `buildTree()` - kreiraj jednostavno stablo:
  ```cpp
  // Primer:
  root = new Node(0, OperationMode::AUTOMATIC);
  allNodes[0] = root;
  
  Node* node1 = new Node(1, OperationMode::AUTOMATIC);
  allNodes[1] = node1;
  root->addChild(node1);
  
  Consumer* c1 = new Consumer(10);
  consumers.push_back(c1);
  // TODO: Kako ćeš povezati consumer sa node-om?
  ```
- `findNode()` - već je skoro implementirano
- `sendRequest()` i `sendRequestToAll()` - pozovi `requestConsumption()` na čvoru

**Test**: Kreiraj mrežu i proveri da li se stablo kreira ispravno.

### 5. Aggregator.h i Aggregator.cpp
U `Aggregator.cpp`:
- `initialize()` - pozovi `network.buildTree()`
- `printTotalConsumption()` - već je skoro implementirano
- `runAutomaticMode()` - za sada samo pozovi `initialize()` i generiši nekoliko podataka
- `runBatchMode()` - za sada može biti prazno
- `testWithSmallData()` - kreiraj mrežu, generiši 10-50 podataka, izmeri vreme
- `testWithLargeData()` - kreiraj mrežu, generiši ~10,000 podataka, izmeri vreme

### 6. main.cpp
- Odkomentariši pozive funkcija u switch-u
- Dodaj `aggregator.initialize()` na početak

## Korak 3: Iterativni pristup

**NEMOJ** pokušavati da implementiraš sve odjednom!

1. **Faza 1**: Uradi Message i Consumer - testiraj da li radi
2. **Faza 2**: Dodaj Node - testiraj sa jednim parent i child
3. **Faza 3**: Dodaj Network - testiraj kreiranje stabla
4. **Faza 4**: Uradi AUTOMATIC režim (najjednostavniji)
5. **Faza 5**: Dodaj BATCH režim
6. **Faza 6**: Uradi testiranje i dokumentaciju

## Korak 4: Rešavanje problema

### Problem: "Cannot open include file"
- Proveri da li si dodao fajl u projekat
- Proveri da li su putanje ispravne

### Problem: "Unresolved external symbol"
- Proveri da li si implementirao sve metode koje si deklarisao
- Proveri da li si dodao .cpp fajlove u projekat

### Problem: "Redefinition"
- Proveri da li imaš `#pragma once` ili `#ifndef` u header fajlovima

### Problem: Memory leaks
- Dodaj destruktore koji brišu dinamički alocirane objekte
- Koristi `delete` za svaki `new`

## Korak 5: Dodatni saveti

1. **Kompajliraj često** - nakon svakog manjeg dela koda kompajliraj i proveri greške
2. **Koristi debugger** - stavi breakpoint i vidi šta se dešava
3. **Piši komentare** - dokumentuj šta radi svaka funkcija
4. **Testiraj mali deo** - pre nego što uradiš kompleksno, testiraj jednostavno

## Checklist za svaki fajl

Za svaki .cpp fajl proveri:
- [ ] Include-ovi su dodati
- [ ] Sve metode su implementirane
- [ ] Konstruktori inicijalizuju sve članove
- [ ] Nema memory leaks (delete za svaki new)
- [ ] Kod se kompajlira bez grešaka
- [ ] Kod se testira i radi

---

**Srećno kodiranje! 🚀**

