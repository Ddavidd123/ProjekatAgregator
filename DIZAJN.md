# Dizajn i stanje projekta Agregator

## 1. Specifikacija (IKpProj) – šta traži

| Zahtev | Stanje |
|--------|--------|
| Distribuirani servis, agregacija od odredišta ka izvoru | ✅ Urađeno |
| Topologija stabla: koren = država, odredišta = delovi države, mrežna odredišta = potrošači | ✅ Urađeno |
| Zahtev **svim** potrošačima | ✅ Urađeno (Automatski / Batch) |
| Zahtev **određenim delovima** države i njihovim potrošačima | ✅ **Opcija 7** u meniju (Zahtev prema delu države, nodeId) |
| Potrošači periodično šalju potrošnju u centralu (čvor iznad) | ✅ Na zahtev servera šalju; režim kao „periodično“ |
| **Režim 1**: automatski šalju potrošnju | ✅ Automatski rezim |
| **Režim 2**: objedinjuju u vremenskom intervalu, šalju objedinjeni izveštaj | ✅ Batch rezim |
| Izvorni čvor **samo štampa** ukupnu potrošnju | ✅ Urađeno |
| Testirati oba režima, mali i veliki broj (~10.000), dokumentovati | ✅ Opcija **9** – Testiraj (100 + ~10.000 izveštaja, oba režima); rezultati u `TestResults.txt` |

---

## 2. Tvoj dizajn (slika – Država, Centrala, Potrošač)

| Element | Opis | Stanje u kodu |
|---------|------|----------------|
| **Država** | Šalje zahtev za potrošnju, eventualno „region x“ | ✅ Šalje zahtev (Automatski/Batch). **„Region x“** = opcija **7** (Zahtev prema delu države, nodeId) |
| **Centrala** | Prima konekcije, HashMap potrošača, kružni bafer, thread pool, paralelna obrada | ✅ HashMap (socket→consumerId), CircularBuffer, ThreadPool (8 niti); consumer thread čita iz bafera |
| **Potrošač** | ID, gde pripada, koliko potrošio | ✅ ID i „gde pripada“ (parent u stablu). Klijent = jedan proces |

---

## 3. Dokumentacija kolega – šta oni imaju

- **Prijemni thread**: prima nove potrošače, čuva u **HashMap** (ključ socket_fd).  
  **Kod**: accept thread + **HashMap** `socketToConsumerId_` (socket → consumerId) u SocketServer.
- **Worker threads**: šalju zahtev klijentima, primaju CONSUMPTION, stavljaju u **kružni bafer**.  
  **Kod**: **ThreadPool** (8 niti) submit-uje taskove; svaki task šalje REQUEST, prima CONSUMPTION, push u **CircularBuffer**.
- **Consumer thread**: čita iz kružnog bafera, agregira u stablo.  
  **Kod**: consumer thread sa **popWait**, poziva `parent->receiveConsumption()`.
- **Thread za nadređenog**: prima odgovore, javlja workerima.  
  **Kod**: **nema** hijerarhiju agregatora (jedan server).
- **Hijerarhija**: centralni + podređeni agregatori, svaki sa istim dizajnom.  
  **Kod**: **jedan** server (država), klijenti = potrošači; **nema** nivoa agregatora između.

---

## 4. Šta je konkretno urađeno

- **Server (Agregator.exe)**: TCP na portu, gradi stablo (Data Source → Agr 0…5 → potrošači 10–15), prima klijente, REGISTER.
- **Klijenti (AgregatorClient.exe)**: connect, REGISTER, na REQUEST šalju CONSUMPTION; batch podržan.
- **Automatski rezim**: zahtev svima → prima potrošnju → agregira → štampa ukupno.
- **Batch rezim**: interval, REQUEST_BATCH / REQUEST_BATCH_END → agregira → štampa.
- **Stablo**: root = država, čvorovi 1–6, potrošači 10–15; agregacija prema rootu.
- **Prikaz strukture stabla** i **ukupna potrošnja** u meniju.

---

## 5. Samostalne strukture podataka (bez STL)

Implementirane strukture (bez `std::vector`, `std::map`, `std::queue`, `std::set`):

| Struktura | Fajl | Namena |
|-----------|------|--------|
| **DynamicArray** | `DynamicArray.h` | Dinamički niz (analogon vector) |
| **HashMap** | `HashMap.h` | Hash mapa (otvoreno adresiranje) – Network (nodeId→Node, consumerId→parent), SocketServer (socket→consumerId) |
| **CircularBuffer** | `CircularBuffer.h/cpp` | Kružni bafer za ConsumptionReport (producer-consumer) |
| **TaskQueue** | `Queue.h` | Red zadataka u ThreadPool |
| **IntSet** | `IntSet.h` | Skup registrovanih consumerId (Agregator) |

## 6. Šta eventualno dopuniti

1. **0 kWh u Automatskom rezimu** – ako se desi, proveri klijente i REQUEST/CONSUMPTION protokol.
2. **Stress testovi** – dogovor sa asistentima za **dva** stress testa (npr. malo + veliko; ili drugačiji scenariji). Rezultati u `TestResults.txt`.
3. **VS Profiler** – pogledaj README sekciju ispod.

---

## 7. Kako pokrenuti i šta raditi

1. **Build**: Agregator (server) + AgregatorClient (klijent).
2. **Server**:  
   - Pokretanje → meni **1** (Inicijalizuj mrežu) → **2** (Pokreni server).  
   - Port 12345 (ili drugi po izboru).
3. **Klijenti**:  
   - `AgregatorClient.exe` → potrošač 10;  
   - `AgregatorClient.exe 11` → potrošač 11; itd. za 12, 13, 14, 15.
4. **Na serveru**:  
   - **3** Automatski rezim → zahtev svima, agregacija, štampa ukupno;  
   - **4** Batch rezim → interval, zatim agregacija i štampa;  
   - **5** Prikazi ukupnu potrošnju; **6** Prikazi strukturu stabla;  
   - **7** Zahtev prema delu države (nodeId 0–6) → zahtev samo klijentima u tom delu;  
   - **8** Zaustavi server.
5. **Provera**: Pri 3 ili 4 treba da vidiš **„Primljeno N izvestaja“** i **ukupnu potrošnju > 0**. Ako je N = 0 ili potrošnja 0, proveri da su klijenti zaista povezani i da šalju (npr. AgregatorClient za 10–15).

---

## 8. Rezime

- **Prema specu**: client–server, stablo, oba režima, izvor štampa, opcija 7 (deo države), testiranje (opcija 9).  
- **Prema tvom dizajnu**: Država ✅; Centrala sa HashMap, CircularBuffer, ThreadPool ✅; Potrošač ✅.  
- **Strukture podataka**: sve samostalno implementirane (DynamicArray, HashMap, CircularBuffer, TaskQueue, IntSet) – bez STL struktura.
