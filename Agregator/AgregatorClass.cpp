#include "Agregator.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <ctime>

#ifdef _WIN32
	#include <io.h>
	#include <fcntl.h>
	#include <share.h>
	#include <sys/stat.h>
#endif

#ifdef _DEBUG
	#include <crtdbg.h>
#endif

#ifndef INVALID_SOCKET
	#define INVALID_SOCKET (-1)
#endif

using namespace std;

Agregator::Agregator() {}

Agregator::~Agregator() {
	stopServer();
	if (acceptThread_.joinable())
		acceptThread_.join();
}

bool Agregator::isInitialized() const {
	return network.getRoot() != nullptr;
}

void Agregator::initialize() {
	network.buildTree();
}

void Agregator::acceptLoop(int port) {
	if (!server_.start(port)) {
		acceptRunning_ = false;
		return;
	}
	serverPort_ = port;
	cout << "  Server slusa na portu " << port << ". Cekam klijente (potrosaci 10-15)...\n";

	while (acceptRunning_ && server_.isRunning()) {
		sockaddr_in clientAddr {};
#ifdef _WIN32
		int len = sizeof(clientAddr);
		socket_t clientSock = ::accept(server_.getListenSocket(), (sockaddr*)&clientAddr, &len);
#else
		socklen_t len = sizeof(clientAddr);
		socket_t clientSock = ::accept(server_.getListenSocket(), (sockaddr*)&clientAddr, &len);
#endif
		if (clientSock == INVALID_SOCKET || !acceptRunning_) {
			if (clientSock != INVALID_SOCKET) {
#ifdef _WIN32
				closesocket(clientSock);
#else
				close(clientSock);
#endif
			}
			continue;
		}

		string line;
		if (!server_.recvLine(clientSock, line)) {
#ifdef _WIN32
			closesocket(clientSock);
#else
			close(clientSock);
#endif
			continue;
		}

		int consumerId = -1;
		bool autoAssign = false;
		if (line.size() >= 8 && line.compare(0, 8, "REGISTER") == 0) {
			istringstream is(line.substr(8));
			if (is >> consumerId) { /* explicit ID */ }
			else { consumerId = -1; autoAssign = true; }
		}
		bool ok = false;
		string errMsg;
		if (autoAssign) {
			lock_guard<mutex> lock(registeredMutex_);
			for (int id = 10; id <= 15; ++id) {
				if (registeredIds_.count(id) == 0) {
					registeredIds_.insert(id);
					consumerId = id;
					ok = true;
					break;
				}
			}
			if (!ok) errMsg = " nema slobodnih ID (10-15 svi zauzeti). Zatvori neki klijent pa probaj ponovo.";
		} else if (consumerId >= 0 && network.isValidConsumerId(consumerId)) {
			lock_guard<mutex> lock(registeredMutex_);
			if (registeredIds_.count(consumerId) == 0) {
				registeredIds_.insert(consumerId);
				ok = true;
			} else {
				errMsg = " consumerId " + to_string(consumerId) + " zauzet. Pokreni bez arg (auto-dodela): AgregatorClient";
				bool first = true;
				for (int id = 10; id <= 15; ++id) {
					if (registeredIds_.count(id) == 0) {
						if (!first) errMsg += ", ";
						errMsg += "AgregatorClient " + to_string(id);
						first = false;
					}
				}
			}
		} else {
			errMsg = " neispravan consumerId. Bez arg = auto-dodela, ili: AgregatorClient 10, AgregatorClient 11, ...";
		}
		if (ok) {
			server_.addClient(clientSock, consumerId);
			server_.sendLine(clientSock, string(Protocol::CMD_OK) + " " + to_string(consumerId));
			cout << "  + Potrosac " << consumerId << " prikljucen" << (autoAssign ? " (auto)" : "") << ".\n";
		} else {
			server_.sendLine(clientSock, string(Protocol::CMD_ERROR) + errMsg);
#ifdef _WIN32
			closesocket(clientSock);
#else
			close(clientSock);
#endif
		}
	}
	{
		lock_guard<mutex> lock(registeredMutex_);
		registeredIds_.clear();
	}
	acceptRunning_ = false;
}

bool Agregator::startServer(int port) {
	if (!isInitialized()) {
		cout << "Prvo inicijalizujte mrezu (opcija 1).\n";
		return false;
	}
	if (acceptRunning_ || server_.isRunning()) {
		cout << "Server je vec pokrenut.\n";
		return false;
	}
	acceptRunning_ = true;
	acceptThread_ = thread(&Agregator::acceptLoop, this, port);
	return true;
}

void Agregator::stopServer() {
	if (!server_.isRunning() && !acceptThread_.joinable()) return;
	acceptRunning_ = false;
	server_.stop();
	if (acceptThread_.joinable()) {
		acceptThread_.join();
	}
	cout << "Server zaustavljen.\n";
}

bool Agregator::parseConsumption(const string& line, double& out) {
	if (line.size() < 11) return false;
	// "CONSUMPTION" + blanko + broj
	double v = 0;
#ifdef _MSC_VER
	if (sscanf_s(line.c_str(), "CONSUMPTION %lf", &v) != 1) return false;
#else
	if (sscanf(line.c_str(), "CONSUMPTION %lf", &v) != 1) return false;
#endif
	out = v;
	return true;
}

void Agregator::runAutomaticMode() {
	if (!isInitialized()) { cout << "Prvo inicijalizujte mrezu (opcija 1).\n"; return; }
	if (!server_.isRunning()) { cout << "Prvo pokrenite server (opcija 2).\n"; return; }

	vector<ClientConn> clients;
	server_.getClientsCopy(clients);
	if (clients.empty()) {
		cout << "  Nema povezanih klijenata. Pokrenite: AgregatorClient 10, AgregatorClient 11, ...\n";
			return;
		}

	cout << "\n=== Automatski rezim (mrezni) ===\n";
	network.setAllNodesMode(OperationMode::AUTOMATIC);
	network.resetAllConsumptions();

	int received = 0;
	vector<int> reportedIds;
	for (auto& c : clients) {
		if (!server_.sendLine(c.sock, Protocol::CMD_REQUEST)) {
			server_.removeClient(c.sock);
			{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
			continue;
		}
		string line;
		if (!server_.recvLine(c.sock, line)) {
			server_.removeClient(c.sock);
			{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
			continue;
		}
		double v = 0;
		if (!parseConsumption(line, v)) {
			cerr << "  ! Ocekivan CONSUMPTION, dobijeno: " << line.substr(0, 50) << "\n";
			continue;
		}
		Node* parent = network.getParentOfConsumer(c.consumerId);
		if (parent) { parent->receiveConsumption(v); ++received; reportedIds.push_back(c.consumerId); }
	}
	if (received > 0) {
		cout << "  Primljeno " << received << " izvestaja";
		if (!reportedIds.empty()) {
			cout << " (potrosaci: ";
			for (size_t i = 0; i < reportedIds.size(); ++i) cout << (i ? ", " : "") << reportedIds[i];
			cout << ")";
		}
		cout << ".\n";
	} else if (!clients.empty())
		cerr << "  Nijedan klijent nije poslao ispravan CONSUMPTION.\n";

	printTotalConsumption();
	cout << "\n";
}

void Agregator::runBatchMode(int intervalSeconds) {
	if (!isInitialized()) { cout << "Prvo inicijalizujte mrezu (opcija 1).\n"; return; }
	if (!server_.isRunning()) { cout << "Prvo pokrenite server (opcija 2).\n"; return; }

	vector<ClientConn> clients;
	server_.getClientsCopy(clients);
	if (clients.empty()) {
		cout << "  Nema povezanih klijenata. Pokrenite: AgregatorClient 10, AgregatorClient 11, ...\n";
			return;
		}

	cout << "\n=== BATCH rezim (mrezni), interval " << intervalSeconds << " s ===\n";
	network.setAllNodesMode(OperationMode::BATCH);
	network.resetAllConsumptions();

	int steps = 5;
	auto stepMs = (intervalSeconds * 1000) / steps;
	for (int i = 0; i < steps; i++) {
		for (auto& c : clients) server_.sendLine(c.sock, Protocol::CMD_REQUEST_BATCH);
		cout << "  Sakupljanje " << (i + 1) << "/" << steps << "...\n";
		this_thread::sleep_for(chrono::milliseconds(stepMs));
	}
	for (auto& c : clients) server_.sendLine(c.sock, Protocol::CMD_REQUEST_BATCH_END);

	int received = 0;
	vector<int> batchReportedIds;
	for (auto& c : clients) {
		string line;
		if (!server_.recvLine(c.sock, line)) {
			server_.removeClient(c.sock);
			{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
			continue;
		}
		double v = 0;
		if (!parseConsumption(line, v)) {
			cerr << "  ! Ocekivan CONSUMPTION (batch), dobijeno: " << line.substr(0, 50) << "\n";
			continue;
		}
		Node* parent = network.getParentOfConsumer(c.consumerId);
		if (parent) { parent->receiveConsumption(v); ++received; batchReportedIds.push_back(c.consumerId); }
	}
	if (received > 0) {
		cout << "  Primljeno " << received << " batch izvestaja";
		if (!batchReportedIds.empty()) {
			cout << " (potrosaci: ";
			for (size_t i = 0; i < batchReportedIds.size(); ++i) cout << (i ? ", " : "") << batchReportedIds[i];
			cout << ")";
		}
		cout << ".\n";
	} else if (!clients.empty())
		cerr << "  Nijedan klijent nije poslao ispravan CONSUMPTION.\n";

	network.processBatches();
	printTotalConsumption();
	cout << "\n";
}

void Agregator::printTotalConsumption() {
	Node* root = network.getRoot();
	if (root)
		cout << "  Ukupna potrosnja: " << root->getAggregatedConsumption() << " kWh\n";
	else
		cout << "  Mreza nije inicijalizovana.\n";
}

void Agregator::runSubtreeRequest(int nodeId) {
	if (!isInitialized()) { cout << "Prvo inicijalizujte mrezu (opcija 1).\n"; return; }
	if (!server_.isRunning()) { cout << "Prvo pokrenite server (opcija 2).\n"; return; }
	if (!network.nodeExists(nodeId)) {
		cout << "Node " << nodeId << " ne postoji. nodeId: 0 (cela) | 1..6 (Agr 0..5).\n";
		return;
	}

	vector<ClientConn> clients;
	server_.getClientsCopy(clients);
	vector<ClientConn> inSubtree;
	for (auto& c : clients)
		if (network.isConsumerInSubtree(c.consumerId, nodeId))
			inSubtree.push_back(c);

	if (inSubtree.empty()) {
		cout << "  Nema klijenata u delu drzave (nodeId " << nodeId << "). Pokrenite AgregatorClient sa odgovarajucim ID (vidi opciju 6).\n";
		return;
	}

	cout << "\n=== Zahtev prema delu drzave (nodeId " << nodeId << "), " << inSubtree.size() << " klijenata ===\n";
	network.setAllNodesMode(OperationMode::AUTOMATIC);
	network.resetAllConsumptions();

	int received = 0;
	vector<int> subtreeReportedIds;
	for (auto& c : inSubtree) {
		if (!server_.sendLine(c.sock, Protocol::CMD_REQUEST_SUBTREE)) {
			server_.removeClient(c.sock);
			{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
			continue;
		}
		string line;
		if (!server_.recvLine(c.sock, line)) {
			server_.removeClient(c.sock);
			{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
			continue;
		}
		double v = 0;
		if (!parseConsumption(line, v)) {
			cerr << "  ! Ocekivan CONSUMPTION, dobijeno: " << line.substr(0, 50) << "\n";
			continue;
		}
		Node* parent = network.getParentOfConsumer(c.consumerId);
		if (parent) { parent->receiveConsumption(v); ++received; subtreeReportedIds.push_back(c.consumerId); }
	}
	if (received > 0) {
		cout << "  Primljeno " << received << " izvestaja";
		if (!subtreeReportedIds.empty()) {
			cout << " (potrosaci: ";
			for (size_t i = 0; i < subtreeReportedIds.size(); ++i) cout << (i ? ", " : "") << subtreeReportedIds[i];
			cout << ")";
		}
		cout << ".\n";
	}

	Node* root = network.getRoot();
	if (root)
		cout << "  Ukupna potrosnja (nodeId " << nodeId << "): " << root->getAggregatedConsumption() << " kWh\n";
	else
		cout << "  Mreza nije inicijalizovana.\n";
	cout << "\n";
}

void Agregator::printTreeStructure() {
	if (!isInitialized()) { cout << "Prvo inicijalizujte mrezu (opcija 1).\n"; return; }
	network.printTreeStructure();
}

static void writeLine(std::ofstream& out, const std::string& s) {
	out << s << "\n";
	out.flush();
}

void Agregator::runTests() {
	if (!isInitialized()) { cout << "Prvo inicijalizujte mrezu (opcija 1).\n"; return; }
	if (!server_.isRunning()) { cout << "Prvo pokrenite server (opcija 2).\n"; return; }

	vector<ClientConn> clients;
	server_.getClientsCopy(clients);
	if (clients.empty()) {
		cout << "  Nema klijenata. Pokrenite AgregatorClient (npr. 6 terminala) pa ponovo 9.\n";
		return;
	}

	const std::string path = "TestResults.txt";
	std::ofstream out(path, std::ios::out | std::ios::trunc);
	if (!out) {
		cerr << "  Nije moguce otvoriti " << path << " za pisanje.\n";
		return;
	}

	time_t now = time(nullptr);
	char buf[80];
#ifdef _WIN32
	struct tm t;
	localtime_s(&t, &now);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
#else
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
#endif

	writeLine(out, std::string("Datum: ") + buf);
	writeLine(out, "");

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
	_CrtMemState s0, s1, diff;
	_CrtMemCheckpoint(&s0);
#endif

	const int MALO = 100;
	const int VELIKO = 10000;

	auto runAuto = [this, &clients](int target, int& totalReports, int64_t& elapsedMs) {
		network.setAllNodesMode(OperationMode::AUTOMATIC);
		network.resetAllConsumptions();
		totalReports = 0;
		auto t0 = chrono::steady_clock::now();
		while (totalReports < target) {
			for (auto& c : clients) {
				if (!server_.sendLine(c.sock, Protocol::CMD_REQUEST)) {
					server_.removeClient(c.sock);
					{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
					continue;
				}
				string line;
				if (!server_.recvLine(c.sock, line)) {
					server_.removeClient(c.sock);
					{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
					continue;
				}
				double v = 0;
				if (!parseConsumption(line, v)) continue;
				Node* parent = network.getParentOfConsumer(c.consumerId);
				if (parent) { parent->receiveConsumption(v); ++totalReports; }
			}
			server_.getClientsCopy(clients);
			if (clients.empty()) break;
		}
		auto t1 = chrono::steady_clock::now();
		elapsedMs = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
	};

	auto runBatch = [this, &clients](int target, int& totalReports, int64_t& elapsedMs) {
		network.setAllNodesMode(OperationMode::BATCH);
		network.resetAllConsumptions();
		totalReports = 0;
		auto t0 = chrono::steady_clock::now();
		while (totalReports < target) {
			for (int step = 0; step < 5; step++) {
				for (auto& c : clients) server_.sendLine(c.sock, Protocol::CMD_REQUEST_BATCH);
				this_thread::sleep_for(chrono::milliseconds(5));
			}
			for (auto& c : clients) server_.sendLine(c.sock, Protocol::CMD_REQUEST_BATCH_END);
			for (auto& c : clients) {
				string line;
				if (!server_.recvLine(c.sock, line)) {
					server_.removeClient(c.sock);
					{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
					continue;
				}
				double v = 0;
				if (!parseConsumption(line, v)) continue;
				Node* parent = network.getParentOfConsumer(c.consumerId);
				if (parent) { parent->receiveConsumption(v); ++totalReports; }
			}
			network.processBatches();
			server_.getClientsCopy(clients);
			if (clients.empty()) break;
		}
		auto t1 = chrono::steady_clock::now();
		elapsedMs = chrono::duration_cast<chrono::milliseconds>(t1 - t0).count();
	};

	int r1, r2, r3, r4;
	int64_t e1, e2, e3, e4;

	cout << "  Test malo (automatic)...\n";
	runAuto(MALO, r1, e1);
	writeLine(out, "Malo - Automatic: " + to_string(r1) + " izvestaja, " + to_string(e1) + " ms");

	cout << "  Test malo (batch)...\n";
	server_.getClientsCopy(clients);
	runBatch(MALO, r2, e2);
	writeLine(out, "Malo - Batch: " + to_string(r2) + " izvestaja, " + to_string(e2) + " ms");

	cout << "  Test veliko (automatic, ~10k)...\n";
	server_.getClientsCopy(clients);
	runAuto(VELIKO, r3, e3);
	writeLine(out, "Veliko - Automatic: " + to_string(r3) + " izvestaja, " + to_string(e3) + " ms");

	cout << "  Test veliko (batch, ~10k)...\n";
	server_.getClientsCopy(clients);
	runBatch(VELIKO, r4, e4);
	writeLine(out, "Veliko - Batch: " + to_string(r4) + " izvestaja, " + to_string(e4) + " ms");

	writeLine(out, "");

#ifdef _DEBUG
	_CrtMemCheckpoint(&s1);
	out.close();
	if (_CrtMemDifference(&diff, &s0, &s1)) {
		int fd = -1;
#ifdef _MSC_VER
		if (_sopen_s(&fd, path.c_str(), _O_WRONLY | _O_CREAT | _O_APPEND, _SH_DENYNO, _S_IREAD | _S_IWRITE) != 0)
			fd = -1;
#else
		fd = _open(path.c_str(), _O_WRONLY | _O_CREAT | _O_APPEND, _S_IREAD | _S_IWRITE);
#endif
		if (fd >= 0) {
			const char* h1 = "\nHEAP SUMMARY:\n";
			_write(fd, h1, (unsigned)strlen(h1));
			_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
			_CrtSetReportFile(_CRT_WARN, (_HFILE)(intptr_t)fd);
			_CrtMemDumpStatistics(&diff);
			const char* h2 = "\nLEAK CHECK:\n";
			_write(fd, h2, (unsigned)strlen(h2));
			_CrtDumpMemoryLeaks();
			_close(fd);
			_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
		} else {
			out.open(path, std::ios::out | std::ios::app);
			if (out) { writeLine(out, "(open fail)"); out.close(); }
		}
	} else {
		out.open(path, std::ios::out | std::ios::app);
		if (out) {
			writeLine(out, "HEAP SUMMARY: no diff pre/posle.");
			out.close();
		}
	}
#else
	out.close();
#endif

	cout << "  Rezultati upisani u " << path << " (ista mapa kao Agregator.exe, npr. x64/Debug).\n";
}
