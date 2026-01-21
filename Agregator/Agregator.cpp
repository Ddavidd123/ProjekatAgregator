#include "Agregator.h"
#include <iostream>

using namespace std;

int main() {
    Agregator aggregator;

    cout << "=== Agregator - Distribuirani Servis za Potrosnju Elektricne Energije ===\n\n";

    int choice;
    do {
        cout << "\nIzaberite opciju:\n";
        cout << "1. Automatski rezim\n";
        cout << "2. Batch rezim\n";
        cout << "3. Test sa malim brojem podataka (AUTOMATIC)\n";
        cout << "4. Test sa velikim brojem podataka (~10,000) (AUTOMATIC)\n";
        cout << "5. Test sa malim brojem podataka za oba rezima\n";
        cout << "6. Test sa velikim brojem podataka za oba rezima\n";
        cout << "7. Prikazi ukupnu potrosnju\n";
        cout << "8. Posalji zahtev odredjenom delu drzave (nodeId)\n";
        cout << "9. Prikazi strukturu stabla (topologiju mreze)\n";
        cout << "0. Izlaz\n";
        cout << "Vas izbor: ";

        cin >> choice;

        switch (choice) {
        case 1:
            aggregator.runAutomaticMode();
            break;
        case 2:
            aggregator.runBatchMode();
            break;
        case 3:
            aggregator.testWithSmallData();
            break;
        case 4:
            aggregator.testWithLargeData();
            break;
        case 5:
            aggregator.testAllModesSmall();
            break;
        case 6:
            aggregator.testAllModesLarge();
            break;
        case 7:
            aggregator.printTotalConsumption();
            break;
        case 8: {
            int nodeId;
            cout << "Unesite nodeId ciljnog dela drzave (subtree): ";
            cin >> nodeId;
            aggregator.sendRequestToSubtree(nodeId);
            break;
        }
        case 9:
            aggregator.printTreeStructure();
            break;
        case 0:
            cout << "Izlaz...\n";
            break;
        default:
            cout << "Nevalidan izbor!\n";
        }
    } while (choice != 0);

    return 0;
}