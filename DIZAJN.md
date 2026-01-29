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
| Testirati oba režima, mali i veliki broj (~10.000), dokumentovati | ⚠️ Testovi uklonjeni pri prelasku na client–server; treba opisati kako testirati |

---

## 2. Tvoj dizajn (slika – Država, Centrala, Potrošač)

| Element | Opis | Stanje u kodu |
|---------|------|----------------|
| **Država** | Šalje zahtev za potrošnju, eventualno „region x“ | ✅ Šalje zahtev (Automatski/Batch). **„Region x“** = opcija **7** (Zahtev prema delu države, nodeId) |
| **Centrala** | Prima konekcije, HashMap potrošača, kružni bafer, thread pool, paralelna obrada | ⚠️ Lista klijenata (socket + consumerId), **nema** HashMap, **nema** kružni bafer, **nema** worker thread pool. Obrada **sekvencijalna** |
| **Potrošač** | ID, gde pripada, koliko potrošio; thread pool za zahteve | ✅ ID i „gde pripada“ (parent u stablu). Klijent = jedan proces, jedan thread; **nema** thread pool |

---

## 3. Dokumentacija kolega – šta oni imaju

- **Prijemni thread**: prima nove potrošače, čuva u **HashMap** (ključ socket_fd).  
  **Kod**: imamo accept thread i listu `(socket, consumerId)`, **nema** HashMap.
- **Listener thread**: čita zahteve od potrošača, stavlja u **red** (kružni bafer).  
  **Kod**: **nema** listener niti red; mi šaljemo zahtev prema klijentima, oni odgovaraju.
- **Worker thread**: uzima iz reda, obrađuje zahtev; ako nema dovoljno struje, ide nadređenom.  
  **Kod**: **nema** worker pool; obrada u glavnoj niti.
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

## 5. Šta treba dopuniti da sve „stoji“ prema specu i dizajnu

1. **0 kWh u Automatskom rezimu**  
   - Kada je jedan klijent povezan i registrovan, a izabereš Automatski rezim, očekivano je **> 0** kWh.  
   - Ako ostane 0, treba proveriti: da li server zaista šalje REQUEST, da li klijent šalje CONSUMPTION, da li se vrednost ispravno parsira i dodaje u stablo.

2. **Testiranje (spec: malo / veliko, ~10.000, dokumentovano)**  
   - Mali broj: npr. 1–2 klijenta, nekoliko uzastopnih Automatski + Batch.  
   - Veliki: npr. 6 klijenata (10–15), više rundi ( Automatski ili Batch) da ukupno imaš reda ~10.000 merenja.  
   - Kratko opisati u README / DIZAJN: kako pokrenuti, šta očekivati, gde se vidi ukupna potrošnja.

3. **Opciono (bliže dizajnu i kolegama)**  
   - **HashMap** umesto/uz listu (npr. socket → consumerId, brz pristup).  
   - **Kružni bafer + worker thread pool**: zahteve (ili odgovore) u red, obrada paralelno; bitno ako želiš arhitekturu kao na slici.  
   - **Hijerarhija agregatora**: više nivoa (centrala ↔ podređeni agregatori) – veća izmena arhitekture.

---

## 6. Kako pokrenuti i šta raditi

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

## 7. Rezime

- **Prema specu**: radimo client–server, stablo, oba režima, izvor samo štampa; **nedostaje** opcija za deo države u meniju i jasan opis testiranja.  
- **Prema tvom dizajnu**: Država šalje zahtev ✅; Centrala ima osnovnu logiku ✅, ali **nema** HashMap, bafer, thread pool; Potrošač kao proces ✅, bez thread poola.  
- **Prema kolegama**: značajno jednostavnija varijanta – jedan server, lista klijenata, sekvencijalna obrada; **nema** njihovu hijerarhiju, redove i worker pool.

**Urađeno u ovom krugu**: opcija **7** (Zahtev prema delu države), **isConsumerInSubtree**, ispis **„Primljeno N izvestaja“** u Automatskom i Batch rezimu, i **upozorenje** ako nijedan klijent ne pošalje validan CONSUMPTION (pomaže pri debugu 0 kWh).
