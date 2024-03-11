#include "cookieedit.h"
#include <assert.h>
#include <sys/time.h>
#include "ini.h"
#include "gtrace.h"

CookieEdit::CookieEdit() {
}

CookieEdit::~CookieEdit() {
	close();
}

bool CookieEdit::open(std::string fileName) {
	int res = sqlite3_open(fileName.data(), &db_);
	if (res != SQLITE_OK) {
		GTRACE("can not open database: %s", sqlite3_errmsg(db_));
		return false;
	}
	return true;
}

void CookieEdit::close() {
	if (db_ != nullptr) {
		sqlite3_close(db_);
		db_ = nullptr;
	}
}

inline void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));
}

inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}

inline void trim(std::string &s) { rtrim(s); ltrim(s); }

bool CookieEdit::insert(std::string host, std::string cookies) {
	std::istringstream stream(cookies);
	std::string cookie;

	std::string sql = "select max(id) as max_id from moz_cookies";
	sqlite3_stmt* stmt;
	int res = sqlite3_prepare(db_, sql.data(), sql.size(), &stmt, nullptr);
	if (res != SQLITE_OK) {
		GTRACE("sqlite3_prepare(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
		return false;
	}

	int maxId = 0;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		const unsigned char* maxIdStr = sqlite3_column_text(stmt, 0);
		if (maxIdStr != nullptr)
			maxId = std::atoi(reinterpret_cast<const char*>(maxIdStr));
	}
	GTRACE("maxId=%d", maxId);
	struct timeval now;
	gettimeofday(&now, NULL);

	while (std::getline(stream, cookie, ';')) {
		trim(cookie);
		GTRACE("'%s'", cookie.data());

		std::istringstream oneStream(cookie);
		std::string name;
		std::string value;
		if (!std::getline(oneStream, name, '=')) {
			GTRACE("std::getline(%s) return false", cookie.data());
			return false;
		}
		if (!std::getline(oneStream, value, '=')) { // no value
			GTRACE("std::getline(%s) return false", cookie.data());
			value = "";
		}
		trim(name);
		trim(value);
		GTRACE("%s %s", name.data(), value.data());
		if (!insert(++maxId, host, name, value, now.tv_sec + 31536000))
			return false;
	}
	return true;
}

/*
CREATE TABLE moz_cookies (
	id INTEGER PRIMARY KEY,
	originAttributes TEXT NOT NULL DEFAULT '',
	name TEXT,
	value TEXT,
	host TEXT,
	path TEXT,
	expiry INTEGER,
	lastAccessed INTEGER,
	creationTime INTEGER,
	isSecure INTEGER,
	isHttpOnly INTEGER,
	inBrowserElement INTEGER DEFAULT 0,
	sameSite INTEGER DEFAULT 0,
	rawSameSite INTEGER DEFAULT 0,
	schemeMap INTEGER DEFAULT 0,
	CONSTRAINT moz_uniqueid UNIQUE (name, host, path, originAttributes)
)
*/

bool CookieEdit::insert(int id, std::string host, std::string name, std::string value, time_t now, std::string path, std::string originAttributes) {
	std::string sql = "SELECT id FROM moz_cookies WHERE name='" + name + "' AND host='"+ host + "'AND path='" + path + "' AND originAttributes='" + originAttributes + "'";
	GTRACE("%s", sql.data());
	sqlite3_stmt* stmt;
	int res = sqlite3_prepare(db_, sql.data(), sql.size(), &stmt, nullptr);
	if (res != SQLITE_OK) {
		GTRACE("sqlite3_prepare(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
		return false;
	}

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		const unsigned char* currendItStr = sqlite3_column_text(stmt, 0);
		assert(currendItStr != nullptr);
		int currentId = std::atoi(reinterpret_cast<const char*>(currendItStr));
		sql = "UPDATE moz_cookies SET value='" + value + "' WHERE id=" + std::to_string(currentId);
		int res = sqlite3_prepare(db_, sql.data(), sql.size(), &stmt, nullptr);
		if (res != SQLITE_OK) {
			GTRACE("sqlite3_prepare(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
			return false;
		}
		res = sqlite3_step(stmt);
		if (res != SQLITE_DONE) {
			GTRACE("sqlite3_step(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
			return false;
		}
		return true;
	}

	std::string nowStr = std::to_string(now);
	sql = std::string("INSERT INTO moz_cookies (id, name, value, host, path, originAttributes, expiry, lastAccessed, creationTime) ") +
		"VALUES (" + std::to_string(id) + ", '" + name + "', '" + value + "', '" + host + "', '" + path + "', '" + originAttributes + "', " + nowStr + "," + nowStr + "," + nowStr + ")";
	res = sqlite3_prepare(db_, sql.data(), sql.size(), &stmt, nullptr);
	if (res != SQLITE_OK) {
		GTRACE("sqlite3_prepare(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
		return false;
	}
	res = sqlite3_step(stmt);
	if (res != SQLITE_DONE) {
		GTRACE("sqlite3_step(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
		return false;
	}

	return true;
}

std::string CookieEdit::findSqliteFileName(std::string profilesDir) {
#ifdef WIN32
	std::string fileName = profilesDir + "\\profiles.ini";
#else
	std::string fileName = profilesDir + "/profiles.ini";
#endif

	mINI::INIFile file(fileName);
	mINI::INIStructure ini;
	if (!file.read(ini)) {
		GTRACE("file.read(%s) return false", fileName.data());
	}

	std::string path = ini["Profile0"]["Path"];
#ifdef WIN32
	return profilesDir + "\\" + path + "\\cookies.sqlite";
#else
	return profilesDir + "/" + path + "/cookies.sqlite";
#endif
}
