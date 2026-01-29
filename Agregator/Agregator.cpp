#include "Agregator.h"
#include <iostream>

using namespace std;

int main() {
	Agregator aggregator;

	cout << "=== Agregator SERVER - Distribuirani servis za potrosnju elektricne energije ===\n\n";
	cout << "Redosled: 1. Inicijalizuj mrezu -> 2. Pokreni server -> pokreni klijente.\n";
	cout << "          U svakom terminalu: AgregatorClient (auto-dodela ID 10,11,...,15).\n";
	cout << "          Zatim 3. Automatski ili 4. Batch, 5. Prikazi potrosnju.\n\n";

	int choice;
	do {
		cout << "\n--- Meni ---\n";
		cout << "1. Inicijalizuj mrezu (gradi topologiju)\n";
		cout << "2. Pokreni server (ceka klijente na portu " << Protocol::DEFAULT_PORT << ")\n";
		cout << "3. Automatski rezim (zahtev svima -> agregiraj -> stampaj)\n";
		cout << "4. Batch rezim (interval u sekundama)\n";
		cout << "5. Prikazi ukupnu potrosnju\n";
		cout << "6. Prikazi strukturu stabla\n";
		cout << "7. Zahtev prema delu drzave (nodeId 0-6, vidi opciju 6)\n";
		cout << "8. Zaustavi server\n";
		cout << "0. Izlaz\n";
		cout << "Izbor: ";

		cin >> choice;

		switch (choice) {
		case 1:
			aggregator.initialize();
			cout << "Mreza inicijalizovana.\n";
			break;
		case 2: {
			int port = Protocol::DEFAULT_PORT;
			cout << "Port (default " << port << "): ";
			if (cin >> port && port > 0)
				aggregator.startServer(port);
			else {
				cin.clear();
				cin.ignore(10000, '\n');
				aggregator.startServer(Protocol::DEFAULT_PORT);
			}
			break;
		}
		case 3:
			aggregator.runAutomaticMode();
			break;
		case 4: {
			int sec = 3;
			cout << "Interval u sekundama (default 3): ";
			if (cin >> sec && sec > 0)
				aggregator.runBatchMode(sec);
			else {
				if (!cin) { cin.clear(); cin.ignore(10000, '\n'); }
				aggregator.runBatchMode(3);
			}
			break;
		}
		case 5:
			aggregator.printTotalConsumption();
			break;
		case 6:
			aggregator.printTreeStructure();
			break;
		case 7: {
			int nodeId = 0;
			cout << "nodeId (0=cela drzava, 1..6=Agr 0..5): ";
			if (cin >> nodeId)
				aggregator.runSubtreeRequest(nodeId);
			else {
				cin.clear();
				cin.ignore(10000, '\n');
				cout << "Nevalidan unos.\n";
			}
			break;
		}
		case 8:
			aggregator.stopServer();
			break;
		case 0:
			aggregator.stopServer();
			cout << "Izlaz.\n";
			break;
		default:
			cout << "Nevalidan izbor.\n";
		}
	} while (choice != 0);

	return 0;
}
