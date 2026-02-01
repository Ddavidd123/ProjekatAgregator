# Testiranje – dokumentacija

Testovi su rađeni na hijerarhijskoj topologiji mreže sa root čvorom (država), agregatorima (Agr 0–5) i potrošačima (10–15). Na svaki agregator su povezani potrošači putem AgregatorClient procesa.

Za testiranje se koristi **Stress test** (opcija 9 u meniju): for petlja u trajanju od ~30 sekundi u kojoj server u svakoj iteraciji šalje zahteve (REQUEST) povezanim klijentima, klijenti vraćaju potrošnju (CONSUMPTION), a server agregira kroz stablo uz thread pool i kružni bafer. Ovim se generiše veliki broj zahteva (reda hiljada ili desetina hiljada) i opterećuje server u zadatom vremenskom intervalu.

CRT debug heap koristi se za snimak stanja heap-a **pre** i **posle** testa te proveru curenja memorije. Rezultati (broj zahteva, vreme u ms, da li postoji curenje heap-a) upisuju se u `TestResults.txt`.

Svrha testiranja je provera performansi i stabilnosti pri većem broju zahteva i praćenje ispravnog korišćenja memorije u konkurentnom izvršavanju.
