#include "Agregator.h"
#include "CircularBuffer.h"
#include "ThreadPool.h"
#include "DynamicArray.h"
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
	#include <Windows.h>
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
				if (!registeredIds_.contains(id)) {
					registeredIds_.insert(id);
					consumerId = id;
					ok = true;
					break;
				}
			}
			if (!ok) errMsg = " nema slobodnih ID (10-15 svi zauzeti). Zatvori neki klijent pa probaj ponovo.";
		} else if (consumerId >= 0 && network.isValidConsumerId(consumerId)) {
			lock_guard<mutex> lock(registeredMutex_);
			if (!registeredIds_.contains(consumerId)) {
				registeredIds_.insert(consumerId);
				ok = true;
			} else {
				errMsg = " consumerId " + to_string(consumerId) + " zauzet. Pokreni bez arg : AgregatorClient";
				bool first = true;
				for (int id = 10; id <= 15; ++id) {
					if (!registeredIds_.contains(id)) {
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
			cout << "  + Potrosac " << consumerId << " prikljucen.\n";
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

	DynamicArray<ClientConn> clients;
	server_.getClientsCopy(clients);
	if (clients.empty()) {
		cout << "  Nema povezanih klijenata. Pokrenite: AgregatorClient 10, AgregatorClient 11, ...\n";
			return;
		}

	cout << "\n=== Automatski rezim (mrezni) ===\n";
	network.setAllNodesMode(OperationMode::AUTOMATIC);
	network.resetAllConsumptions();

	CircularBuffer buffer(64);
	ThreadPool pool(8);
	int received = 0;
	DynamicArray<int> reportedIds;
	for (const auto& c : clients) {
		pool.submit([this, c, &buffer]() {
			if (!server_.sendLine(c.sock, Protocol::CMD_REQUEST)) {
				server_.removeClient(c.sock);
				{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
				return;
			}
			string line;
			if (!server_.recvLine(c.sock, line)) {
				server_.removeClient(c.sock);
				{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
				return;
			}
			double v = 0;
			if (!parseConsumption(line, v)) {
				cerr << "  ! Ocekivan CONSUMPTION, dobijeno: " << line.substr(0, 50) << "\n";
				return;
			}
			buffer.push(ConsumptionReport{ c.consumerId, v });
		});
	}
	thread consumerThread([this, &buffer, &received, &reportedIds]() {
		ConsumptionReport rep;
		while (true) {
			if (buffer.popWait(rep, 500)) {
				Node* parent = network.getParentOfConsumer(rep.consumerId);
				if (parent) { parent->receiveConsumption(rep.value); ++received; reportedIds.push_back(rep.consumerId); }
			} else if (buffer.isDone() && buffer.size() == 0)
				break;
		}
	});
	pool.waitAll();
	buffer.setDone();
	if (consumerThread.joinable()) consumerThread.join();
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

	DynamicArray<ClientConn> clients;
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

	CircularBuffer buffer(64);
	ThreadPool pool(8);
	int received = 0;
	DynamicArray<int> batchReportedIds;
	for (const auto& c : clients) {
		pool.submit([this, c, &buffer]() {
			string line;
			if (!server_.recvLine(c.sock, line)) {
				server_.removeClient(c.sock);
				{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
				return;
			}
			double v = 0;
			if (!parseConsumption(line, v)) {
				cerr << "  ! Ocekivan CONSUMPTION (batch), dobijeno: " << line.substr(0, 50) << "\n";
				return;
			}
			buffer.push(ConsumptionReport{ c.consumerId, v });
		});
	}
	thread consumerThread([this, &buffer, &received, &batchReportedIds]() {
		ConsumptionReport r;
		while (true) {
			if (buffer.popWait(r, 500)) {
				Node* parent = network.getParentOfConsumer(r.consumerId);
				if (parent) { parent->receiveConsumption(r.value); ++received; batchReportedIds.push_back(r.consumerId); }
			} else if (buffer.isDone() && buffer.size() == 0)
				break;
		}
	});
	pool.waitAll();
	buffer.setDone();
	if (consumerThread.joinable()) consumerThread.join();
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

	DynamicArray<ClientConn> clients;
	server_.getClientsCopy(clients);
	DynamicArray<ClientConn> inSubtree;
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

	CircularBuffer buffer(64);
	ThreadPool pool(8);
	int received = 0;
	DynamicArray<int> subtreeReportedIds;
	for (const auto& c : inSubtree) {
		pool.submit([this, c, &buffer]() {
			if (!server_.sendLine(c.sock, Protocol::CMD_REQUEST_SUBTREE)) {
				server_.removeClient(c.sock);
				{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
				return;
			}
			string line;
			if (!server_.recvLine(c.sock, line)) {
				server_.removeClient(c.sock);
				{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
				return;
			}
			double v = 0;
			if (!parseConsumption(line, v)) {
				cerr << "  ! Ocekivan CONSUMPTION, dobijeno: " << line.substr(0, 50) << "\n";
				return;
			}
			buffer.push(ConsumptionReport{ c.consumerId, v });
		});
	}
	thread consumerThread([this, &buffer, &received, &subtreeReportedIds]() {
		ConsumptionReport r;
		while (true) {
			if (buffer.popWait(r, 500)) {
				Node* parent = network.getParentOfConsumer(r.consumerId);
				if (parent) { parent->receiveConsumption(r.value); ++received; subtreeReportedIds.push_back(r.consumerId); }
			} else if (buffer.isDone() && buffer.size() == 0)
				break;
		}
	});
	pool.waitAll();
	buffer.setDone();
	if (consumerThread.joinable()) consumerThread.join();
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

void Agregator::runStressTest30s() {
	if (!isInitialized()) { cout << "Prvo inicijalizujte mrezu (opcija 1).\n"; return; }
	if (!server_.isRunning()) { cout << "Prvo pokrenite server (opcija 2).\n"; return; }

	DynamicArray<ClientConn> clients;
	server_.getClientsCopy(clients);
	if (clients.empty()) {
		cout << "  Nema klijenata. Pokrenite AgregatorClient (10-15) pa ponovo 9.\n";
		return;
	}

	const int DURATION_SEC = 30;
	cout << "\n=== Stress test: for petlja ~" << DURATION_SEC << " sekundi ===\n";
	cout << "  Klijenata: " << clients.size() << ". Pokrecem...\n\n";

#ifdef _DEBUG
	_CrtMemState s0, s1, diff;
	_CrtMemCheckpoint(&s0);
#endif

	network.setAllNodesMode(OperationMode::AUTOMATIC);
	network.resetAllConsumptions();

	auto tStart = chrono::steady_clock::now();
	int totalZahteva = 0;
	int lastReportSec = 0;

	for (;;) {
		server_.getClientsCopy(clients);
		if (clients.empty()) break;

		auto elapsed = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - tStart).count();
		if (elapsed >= DURATION_SEC) break;

		size_t cap = clients.size() + 256;
		if (cap > 4096) cap = 4096;
		CircularBuffer buffer(static_cast<size_t>(cap));
		ThreadPool pool(8);
		int received = 0;

		for (size_t i = 0; i < clients.size(); ++i) {
			ClientConn c = clients[i];
			pool.submit([this, c, &buffer]() {
				if (!server_.sendLine(c.sock, Protocol::CMD_REQUEST)) {
					server_.removeClient(c.sock);
					{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
					return;
				}
				string line;
				if (!server_.recvLine(c.sock, line)) {
					server_.removeClient(c.sock);
					{ lock_guard<mutex> lock(registeredMutex_); registeredIds_.erase(c.consumerId); }
					return;
				}
				double v = 0;
				if (parseConsumption(line, v))
					buffer.push(ConsumptionReport{ c.consumerId, v });
			});
		}

		thread consumerThread([this, &buffer, &received]() {
			ConsumptionReport rep;
			while (buffer.popWait(rep, 200)) {
				Node* parent = network.getParentOfConsumer(rep.consumerId);
				if (parent) { parent->receiveConsumption(rep.value); ++received; }
			}
			while (buffer.size() > 0 && buffer.pop(rep)) {
				Node* parent = network.getParentOfConsumer(rep.consumerId);
				if (parent) { parent->receiveConsumption(rep.value); ++received; }
			}
		});
		pool.waitAll();
		buffer.setDone();
		if (consumerThread.joinable()) consumerThread.join();

		totalZahteva += received;
		network.resetAllConsumptions();

		if (static_cast<int>(elapsed) >= lastReportSec + 5) {
			lastReportSec = static_cast<int>(elapsed);
			cout << "  [" << elapsed << "s] Ukupno zahteva: " << totalZahteva << ", klijenata: " << clients.size() << "\n";
		}
	}

	auto tEnd = chrono::steady_clock::now();
	auto totalMs = chrono::duration_cast<chrono::milliseconds>(tEnd - tStart).count();
	cout << "\n  Zavrseno. Ukupno " << totalZahteva << " zahteva u " << totalMs << " ms.\n";

#ifdef _DEBUG
	_CrtMemCheckpoint(&s1);
#endif

	// Ispis u TestResults.txt (pored .exe da se uvek nadje)
	std::string path;
#ifdef _WIN32
	char exePath[MAX_PATH];
	if (GetModuleFileNameA(nullptr, exePath, MAX_PATH) != 0) {
		std::string dir(exePath);
		size_t last = dir.find_last_of("\\/");
		if (last != std::string::npos) dir.resize(last + 1);
		path = dir + "TestResults.txt";
	} else
		path = "TestResults.txt";
#else
	path = "TestResults.txt";
#endif

	std::ofstream out(path, std::ios::out | std::ios::trunc);
	if (out) {
		time_t now = time(nullptr);
		char buf[80];
#ifdef _WIN32
		struct tm t;
		localtime_s(&t, &now);
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
#else
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
#endif
		out << "Datum: " << buf << "\n\n";
		out << "Stress test (~30s): " << totalZahteva << " zahteva, " << totalMs << " ms\n";

#ifdef _DEBUG
		if (_CrtMemDifference(&diff, &s0, &s1)) {
			out << "Curenje heap-a: DA\n";
			out << "(Build u Debug modu za detalje - HEAP SUMMARY, LEAK CHECK)\n";
		} else {
			out << "Curenje heap-a: NE\n";
		}
#else
		out << "Curenje heap-a: (proveri u Debug build-u sa CRT)\n";
#endif

		out.flush();
		out.close();
		cout << "  Rezultati upisani u " << path << "\n";
	} else {
		cout << "  Greska: nije moguce otvoriti " << path << " za pisanje.\n";
	}
	cout << "\n";
}
