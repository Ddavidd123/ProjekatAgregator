#include "SocketServer.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
static bool winsockStarted = false;
static void ensureWinsock() {
	if (winsockStarted) return;
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		// caller will handle
		return;
	}
	winsockStarted = true;
}
#endif

SocketServer::SocketServer() : listenSock_(INVALID_SOCKET), running_(false) {
#ifdef _WIN32
	ensureWinsock();
#endif
}

SocketServer::~SocketServer() {
	stop();
}

bool SocketServer::start(int port) {
	if (listenSock_ != INVALID_SOCKET) return true;

#ifdef _WIN32
	ensureWinsock();
#endif

	socket_t s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET) {
		std::cerr << "Socket kreiranje nije uspelo.\n";
		return false;
	}

	int opt = 1;
#ifdef _WIN32
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

	sockaddr_in addr {};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(static_cast<unsigned short>(port));

	if (bind(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "Bind nije uspeo na portu " << port << ".\n";
#ifdef _WIN32
		closesocket(s);
#else
		close(s);
#endif
		return false;
	}

	if (listen(s, 5) == SOCKET_ERROR) {
		std::cerr << "Listen nije uspeo.\n";
#ifdef _WIN32
		closesocket(s);
#else
		close(s);
#endif
		return false;
	}

	listenSock_ = s;
	running_ = true;
	return true;
}

void SocketServer::stop() {
	running_ = false;
	{
		std::lock_guard<std::mutex> lock(clientsMutex_);
		for (auto& c : clients_) {
#ifdef _WIN32
			closesocket(c.sock);
#else
			close(c.sock);
#endif
		}
		clients_.clear();
	}
	if (listenSock_ != INVALID_SOCKET) {
#ifdef _WIN32
		closesocket(listenSock_);
#else
		close(listenSock_);
#endif
		listenSock_ = INVALID_SOCKET;
	}
}

bool SocketServer::sendLine(socket_t sock, const std::string& line) {
	std::string buf = line + "\n";
	const char* p = buf.c_str();
	size_t left = buf.size();
	while (left > 0) {
#ifdef _WIN32
		int n = send(sock, p, (int)left, 0);
#else
		ssize_t n = send(sock, p, left, 0);
#endif
		if (n <= 0) return false;
		p += n;
		left -= (size_t)n;
	}
	return true;
}

bool SocketServer::recvLine(socket_t sock, std::string& out) {
	out.clear();
	char c;
	while (true) {
#ifdef _WIN32
		int n = recv(sock, &c, 1, 0);
#else
		ssize_t n = recv(sock, &c, 1, 0);
#endif
		if (n <= 0) return false;
		if (c == '\n') break;
		if (c != '\r') out += c;
	}
	return true;
}

void SocketServer::addClient(socket_t sock, int consumerId) {
	std::lock_guard<std::mutex> lock(clientsMutex_);
	clients_.push_back({ sock, consumerId });
}

void SocketServer::removeClient(socket_t sock) {
	std::lock_guard<std::mutex> lock(clientsMutex_);
	for (auto it = clients_.begin(); it != clients_.end(); ++it) {
		if (it->sock == sock) {
#ifdef _WIN32
			closesocket(it->sock);
#else
			close(it->sock);
#endif
			clients_.erase(it);
			return;
		}
	}
}

void SocketServer::getClientsCopy(std::vector<ClientConn>& out) const {
	std::lock_guard<std::mutex> lock(clientsMutex_);
	out = clients_;
}
