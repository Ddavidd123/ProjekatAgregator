# Agregator – Server i klijenti

## Arhitektura

- **Server (Agregator.exe)** – država/agregator. Gradi topologiju stabla, osluškuje na portu, prima potrošnju od klijenata, agregira i štampa ukupnu potrošnju.
- **Klijenti (AgregatorClient.exe)** – potrošači. Povezuju se na server, registruju se sa `consumerId` (10–15), zatim na zahtev šalju potrošnju.

## Pokretanje

1. **Pokreni server**
   - Otvori `Agregator.sln`, build (npr. Release x64).
   - Pokreni `Agregator.exe` (ili iz VS: F5 za Agregator projekat).
   - U meniju: **1** Inicijalizuj mrežu → **2** Pokreni server.
   - Server čeka klijente na portu 12345 (možeš promeniti kad izabereš opciju 2).

2. **Pokreni klijente**
   - Pokreni jednu ili više instanci `AgregatorClient.exe`.
   - Sintaksa: `AgregatorClient.exe [consumerId] [host] [port]`
   - Primeri:
     - `AgregatorClient.exe`        → potrošač 10, localhost:12345
     - `AgregatorClient.exe 11`     → potrošač 11
     - `AgregatorClient.exe 12 127.0.0.1 12345`
   - Važeći `consumerId`: 10, 11, 12, 13, 14, 15 (prema topologiji).

3. **Na serveru**
   - **3** Automatski rezim – zahtev svima, prima potrošnju, agregira, štampa ukupno.
   - **4** Batch rezim – interval (npr. 3 s), zatim agregira i štampa.
   - **5** Prikaži ukupnu potrošnju.
   - **6** Prikaži strukturu stabla (nodeId 0–6).
   - **7** Zahtev prema delu države (nodeId) – zahtev samo klijentima u tom delu.
   - **8** Zaustavi server.
   - **9** Stress test (~30s, for petlja, ispis u `TestResults.txt`).
   - **0** Izlaz.

## Testiranje

- **Opcija 9 – Stress test (~30s, for petlja)**
  - Zahteva: mreža (**1**), server (**2**), bar jedan klijent (npr. 6× `AgregatorClient`).
  - For petlja ~30 sekundi – server šalje REQUEST klijentima, prima CONSUMPTION. Veliki broj zahteva opterećuje server.
  - CRT debug heap snima stanje pre i posle testa, proverava curenje memorije.
  - Rezultati se **ispisuju u `TestResults.txt`** (broj zahteva, vreme u ms, da li ima curenja heap-a, datum).

- **VS Profiler** (CPU, memorija)
  - U Visual Studio: **Debug** → **Performance Profiler** (Alt+F2).
  - Izaberi **CPU Usage** i/ili **Memory Usage**.
  - Pokreni Agregator, inicijalizuj mrežu (1), pokreni server (2), pokreni klijente, zatim opciju **9**.

- **Performance Monitor (perfmon)**
  - `Win + R` → **perfmon** → Enter.
  - Add Counters (**+**): **Process** → **Private Bytes**, **Working Set**; instance **Agregator**.
  - Pokreni opciju **9** dok perfmon prati (pre, za vreme, posle – provera curenja memorije i handle-ova).

- Ako vidiš **0 kWh** ili **„Nijedan klijent nije poslao validan CONSUMPTION“**: proveri klijente i da server šalje zahtev (3 ili 4).

## Protokol (TCP, tekstualno)

- Klijent → server: `REGISTER <consumerId>`
- Server → klijent: `OK` ili `ERROR ...`
- Server → klijent: `REQUEST` (jedan report) ili `REQUEST_BATCH` / `REQUEST_BATCH_END` (batch)
- Klijent → server: `CONSUMPTION <vrednost>`

## Meni (server)

| Opcija | Akcija |
|--------|--------|
| 1 | Inicijalizuj mrežu (gradi topologiju) |
| 2 | Pokreni server (port) |
| 3 | Automatski rezim |
| 4 | Batch rezim (interval u sekundama) |
| 5 | Prikaži ukupnu potrošnju |
| 6 | Prikaži strukturu stabla |
| 7 | Zahtev prema delu države (nodeId 0–6) |
| 8 | Zaustavi server |
| 9 | Stress test (~30s, for petlja, ispis u `TestResults.txt`) |
| 0 | Izlaz |
