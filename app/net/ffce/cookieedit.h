#pragma once

#include <string>
#include "sqlite3.h"

struct CookieEdit {
	sqlite3* db_{nullptr};
	CookieEdit();
	virtual ~CookieEdit();
	bool open(std::string fileName);
	void close();
	bool insert(std::string host, std::string cookies);
	bool insert(int id, std::string host, std::string name, std::string value, time_t now, std::string path = "/", std::string originAttributes = "");
	static std::string findSqliteFileName(std::string profilesDir);
};
