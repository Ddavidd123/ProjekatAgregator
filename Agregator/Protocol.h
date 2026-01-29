#pragma once

namespace Protocol {
	const int DEFAULT_PORT = 12345;
	const char* const CMD_REGISTER = "REGISTER";
	const char* const CMD_OK = "OK";
	const char* const CMD_ERROR = "ERROR";
	const char* const CMD_REQUEST = "REQUEST";
	const char* const CMD_REQUEST_SUBTREE = "REQUEST_SUBTREE";
	const char* const CMD_REQUEST_BATCH = "REQUEST_BATCH";
	const char* const CMD_REQUEST_BATCH_END = "REQUEST_BATCH_END";
	const char* const CMD_CONSUMPTION = "CONSUMPTION";
}
