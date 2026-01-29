#include "SocketClient.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
static bool winsockStarted = false;
static void ensureWinsock() {
	if (winsockStarted) return;
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	winsockStarted = true;
}
#endif

SocketClient::SocketClient() : sock_(INVALID_SOCKET) {
#ifdef _WIN32
	ensureWinsock();
#endif
}

SocketClient::~SocketClient() {
	disconnect();
}

bool SocketClient::connect(const char* host, int port) {
	if (sock_ != INVALID_SOCKET) return true;

#ifdef _WIN32
	ensureWinsock();
#endif

	socket_t s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) return false;

	sockaddr_in addr {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast<unsigned short>(port));
#ifdef _WIN32
	inet_pton(AF_INET, host, &addr.sin_addr);
#else
	inet_pton(AF_INET, host, &addr.sin_addr);
#endif

	if (::connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
#ifdef _WIN32
		closesocket(s);
#else
		close(s);
#endif
		return false;
	}
	sock_ = s;
	return true;
}

void SocketClient::disconnect() {
	if (sock_ != INVALID_SOCKET) {
#ifdef _WIN32
		closesocket(sock_);
#else
		close(sock_);
#endif
		sock_ = INVALID_SOCKET;
	}
}

bool SocketClient::sendLine(const std::string& line) {
	if (sock_ == INVALID_SOCKET) return false;
	std::string buf = line + "\n";
	const char* p = buf.c_str();
	size_t left = buf.size();
	while (left > 0) {
#ifdef _WIN32
		int n = send(sock_, p, (int)left, 0);
#else
		ssize_t n = send(sock_, p, left, 0);
#endif
		if (n <= 0) return false;
		p += n;
		left -= (size_t)n;
	}
	return true;
}

bool SocketClient::recvLine(std::string& out) {
	if (sock_ == INVALID_SOCKET) return false;
	out.clear();
	char c;
	while (true) {
#ifdef _WIN32
		int n = recv(sock_, &c, 1, 0);
#else
		ssize_t n = recv(sock_, &c, 1, 0);
#endif
		if (n <= 0) return false;
		if (c == '\n') break;
		if (c != '\r') out += c;
	}
	return true;
}
