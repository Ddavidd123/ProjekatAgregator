#pragma once

#include "Network.h"
#include "SocketServer.h"
#include "Protocol.h"
#include <thread>
#include <atomic>
#include <set>
#include <mutex>

class Agregator {
private:
	Network network;
	SocketServer server_;
	std::thread acceptThread_;
	std::atomic<bool> acceptRunning_{ false };
	std::set<int> registeredIds_;
	std::mutex registeredMutex_;
	int serverPort_ = Protocol::DEFAULT_PORT;

	void acceptLoop(int port);
	static bool parseConsumption(const std::string& line, double& out);

public:
	Agregator();
	~Agregator();

	void initialize();
	bool startServer(int port = Protocol::DEFAULT_PORT);
	void stopServer();
	bool isServerRunning() const { return server_.isRunning(); }

	void runAutomaticMode();
	void runBatchMode(int intervalSeconds = 3);
	void runSubtreeRequest(int nodeId);
	void printTotalConsumption();
	void printTreeStructure();

	bool isInitialized() const;
};
