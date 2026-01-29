#pragma once

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
	typedef SOCKET socket_t;
	typedef int socklen_t_use;
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	typedef int socket_t;
	#define INVALID_SOCKET (-1)
	#define SOCKET_ERROR (-1)
	typedef socklen_t socklen_t_use;
#endif

#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <cstddef>

struct ClientConn {
	socket_t sock;
	int consumerId;
};

class SocketServer {
public:
	SocketServer();
	~SocketServer();

	bool start(int port);
	void stop();
	bool isRunning() const { return running_; }

	bool sendLine(socket_t sock, const std::string& line);
	bool recvLine(socket_t sock, std::string& out);

	void addClient(socket_t sock, int consumerId);
	void removeClient(socket_t sock);
	void getClientsCopy(std::vector<ClientConn>& out) const;

	socket_t getListenSocket() const { return listenSock_; }

private:
	socket_t listenSock_;
	std::atomic<bool> running_;
	std::vector<ClientConn> clients_;
	mutable std::mutex clientsMutex_;
};
