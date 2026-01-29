#pragma once

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
	typedef SOCKET socket_t;
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	typedef int socket_t;
	#define INVALID_SOCKET (-1)
	#define SOCKET_ERROR (-1)
#endif

#include <string>

class SocketClient {
public:
	SocketClient();
	~SocketClient();

	bool connect(const char* host, int port);
	void disconnect();
	bool isConnected() const { return sock_ != INVALID_SOCKET; }

	bool sendLine(const std::string& line);
	bool recvLine(std::string& out);

private:
	socket_t sock_;
};
