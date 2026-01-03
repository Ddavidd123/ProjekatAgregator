# Agregator - Distribuirani Servis za Proračun Potrošnje Električne Energije

## Opis projekta

Distribuirani servis za proračun potrošnje električne energije sa agregacijom podataka od odredišnih servisa ka izvoru. Sistem radi u topologiji stabla gde:
- **Koren (root)** = država (glavni čvor)
- **Srednji čvorovi** = delovi države (regioni, gradovi)
- **Listovi (leaf nodes)** = potrošači

## Zahtevi

- Dva režima rada:
  1. **AUTOMATIC** - automatsko slanje potrošnje u centralu distribucije
  2. **BATCH** - objedinjavanje potrošnje u određenom vremenskom intervalu i slanje objedinjenog izveštaja

- Testiranje na malom i velikom broju podataka (~10.000)
- Dokumentovati testiranja

## Struktura projekta

```
Agregator/
├── main.cpp              - Glavna funkcija i meni
├── Message.h/cpp         - Struktura poruka između čvorova
├── Consumer.h/cpp        - Potrošač (list u stablu)
├── Node.h/cpp            - Čvor u stablu (agregacija podataka)
├── Network.h/cpp         - Mreža/topologija stabla
└── Aggregator.h/cpp      - Glavna klasa za upravljanje sistemom
```

## Dokumentacija

1. **STRUKTURA_PROJEKTA.md** - Detaljan opis strukture projekta, klasa i metoda
2. **POCETAK_RADA.md** - Korak-po-korak vodič za početak rada

## Kako početi

1. Otvori `STRUKTURA_PROJEKTA.md` da vidiš šta sve treba da implementiraš
2. Otvori `POCETAK_RADA.md` da vidiš redosled rada
3. Dodaj sve template fajlove u Visual Studio projekat
4. Počni sa implementacijom redom: Message → Consumer → Node → Network → Aggregator → main

## Kompilacija

Koristi Visual Studio projekat (`.sln` i `.vcxproj` fajlove).

## Testiranje

- Test sa malim brojem podataka (10-50)
- Test sa velikim brojem podataka (~10.000)
- Dokumentovati rezultate (vreme izvršavanja, rezultati)

## Napomene

- Koristi C++11 ili noviji standard
- Pazljivo upravljaj memorijom (memory leaks)
- Testiraj često tokom razvoja

