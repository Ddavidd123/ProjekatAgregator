#include "SocketClient.h"
#include "Protocol.h"
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <locale>
#include <random>
#include <cstdlib>

#ifdef _WIN32
#include <winsock2.h>
#endif

using namespace std;

static double generateConsumption() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(10.0, 100.0);
	return dis(gen);
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		cerr << "WSAStartup failed.\n";
		return 1;
	}
#endif

	const char* host = "127.0.0.1";
	int port = Protocol::DEFAULT_PORT;
	int consumerId = -1;
	bool autoAssign = (argc < 2);

	if (argc >= 2) consumerId = atoi(argv[1]);
	if (argc >= 3) host = argv[2];
	if (argc >= 4) port = atoi(argv[3]);

	if (!autoAssign && (consumerId < 10 || consumerId > 15)) {
		cerr << "ConsumerId mora biti 10-15. Bez arg = auto-dodela: pokreni samo AgregatorClient u svakom terminalu.\n";
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}
	if (autoAssign)
		cout << "AgregatorClient: Spajanje (auto-dodela ID) na " << host << ":" << port << " ...\n";
	else
		cout << "[Potrosac " << consumerId << "] Spajanje na " << host << ":" << port << " ...\n";

	SocketClient client;
	if (!client.connect(host, port)) {
		cerr << (autoAssign ? "AgregatorClient: " : "[Potrosac " + to_string(consumerId) + "] ") << "Konekcija nije uspela. Pokrenite server (opcije 1, 2).\n";
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}
	if (autoAssign)
		cout << "AgregatorClient: Povezan. Registracija (trazi slobodan ID)...\n";
	else
		cout << "[Potrosac " << consumerId << "] Povezan. Registracija...\n";

	ostringstream reg;
	reg << Protocol::CMD_REGISTER;
	if (!autoAssign) reg << " " << consumerId;
	if (!client.sendLine(reg.str())) {
		cerr << (autoAssign ? "AgregatorClient: " : "[Potrosac " + to_string(consumerId) + "] ") << "Slanje REGISTER nije uspelo.\n";
		client.disconnect();
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}

	string line;
	if (!client.recvLine(line)) {
		cerr << (autoAssign ? "AgregatorClient: " : "[Potrosac " + to_string(consumerId) + "] ") << "Nema odgovora od servera.\n";
		client.disconnect();
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}
	if (line.compare(0, 2, Protocol::CMD_OK) != 0) {
		cerr << (autoAssign ? "AgregatorClient: " : "[Potrosac " + to_string(consumerId) + "] ") << "Server odbio: " << line << "\n";
		client.disconnect();
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}
	// OK <id> – uvek parsiraj dodeljeni ID
	{
		size_t i = 2;
		while (i < line.size() && (line[i] == ' ' || line[i] == '\t')) ++i;
		if (i < line.size()) consumerId = atoi(line.c_str() + i);
	}
	if (consumerId < 10 || consumerId > 15) {
		cerr << "AgregatorClient: Server vratio nevalidan ID " << consumerId << ".\n";
		client.disconnect();
#ifdef _WIN32
		WSACleanup();
#endif
		return 1;
	}
	cout << "[Potrosac " << consumerId << "] Registrovan. Cekam zahteve.\n";

	double batchAccum = 0.0;
	double lastConsumption = -1.0;  // zadnja potrosnja; na REQUEST_SUBTREE saljemo istu, ne menjamo
	while (client.recvLine(line)) {
		if (line == Protocol::CMD_REQUEST) {
			double v = generateConsumption();
			lastConsumption = v;
			ostringstream os;
			os.imbue(std::locale::classic());
			os << Protocol::CMD_CONSUMPTION << " " << std::fixed << std::setprecision(6) << v;
			if (!client.sendLine(os.str())) break;
			cout << "  [ " << consumerId << " ] REQUEST -> poslao " << std::fixed << std::setprecision(2) << v << " kWh\n";
		}
		else if (line == Protocol::CMD_REQUEST_SUBTREE) {
			if (lastConsumption < 0) { lastConsumption = generateConsumption(); }
			ostringstream os;
			os.imbue(std::locale::classic());
			os << Protocol::CMD_CONSUMPTION << " " << std::fixed << std::setprecision(6) << lastConsumption;
			if (!client.sendLine(os.str())) break;
			cout << "  [ " << consumerId << " ] SUBTREE (deo drzave) -> poslao " << std::fixed << std::setprecision(2) << lastConsumption << " kWh (ista vrednost)\n";
		}
		else if (line == Protocol::CMD_REQUEST_BATCH) {
			batchAccum += generateConsumption();
		}
		else if (line == Protocol::CMD_REQUEST_BATCH_END) {
			double sent = batchAccum;
			ostringstream os;
			os.imbue(std::locale::classic());
			os << Protocol::CMD_CONSUMPTION << " " << std::fixed << std::setprecision(6) << sent;
			if (!client.sendLine(os.str())) break;
			cout << "  [ " << consumerId << " ] BATCH -> poslao " << std::fixed << std::setprecision(2) << sent << " kWh\n";
			batchAccum = 0.0;
		}
	}

	cout << "[Potrosac " << consumerId << "] Veza prekinuta. Izlaz.\n";
	client.disconnect();
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
