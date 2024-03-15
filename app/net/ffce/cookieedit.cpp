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

	res = sqlite3_exec(db_, "BEGIN TRANSACTION", 0, 0, 0);
	if (res != SQLITE_OK) {
		GTRACE("sqlite3_exec(BEGIN TRANSACTION) return %d %s", res, sqlite3_errmsg(db_));
		return false;
	}
	return true;
}

void CookieEdit::close() {
	if (db_ != nullptr) {
		int res = sqlite3_exec(db_, "END TRANSACTION", 0, 0, 0);
		if (res != SQLITE_OK) {
			GTRACE("sqlite3_exec(END TRANSACTION) return %d %s", res, sqlite3_errmsg(db_));
		}

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

	std::string sql = "SELECT MAX(id) AS max_id FROM moz_cookies";
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
	sqlite3_finalize(stmt);

	struct timeval now;
	gettimeofday(&now, NULL);

	int index = 0;
	while (std::getline(stream, cookie, ';')) {
		trim(cookie);
		std::istringstream oneStream(cookie);
		std::string name;
		std::string value;
		if (!std::getline(oneStream, name, '=')) {
			GTRACE("std::getline(%s) return false", cookie.data());
			return false;
		}
		if (!std::getline(oneStream, value, '\0')) { // no value
			// GTRACE("std::getline(%s) return false", cookie.data());
			value = "";
		}
		trim(name);
		trim(value);
		GTRACE("%d \n%s\n%s\n", ++index, name.data(), value.data());
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
	std::string sql = "SELECT id FROM moz_cookies WHERE name=? AND host=? AND path=? AND originAttributes=?";
	sqlite3_stmt* stmt;
	int res = sqlite3_prepare(db_, sql.data(), sql.size(), &stmt, nullptr);
	if (res != SQLITE_OK) {
		GTRACE("sqlite3_prepare(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
		return false;
	}
	sqlite3_bind_text(stmt, 1, name.data(), name.size(), nullptr);
	sqlite3_bind_text(stmt, 2, host.data(), host.size(), nullptr);
	sqlite3_bind_text(stmt, 3, path.data(), path.size(), nullptr);
	sqlite3_bind_text(stmt, 4, originAttributes.data(), originAttributes.size(), nullptr);

	res = sqlite3_step(stmt);
	if (res == SQLITE_ROW) {
		const unsigned char* currendItStr = sqlite3_column_text(stmt, 0);
		assert(currendItStr != nullptr);
		int currentId = std::atoi(reinterpret_cast<const char*>(currendItStr));
		sql = "UPDATE moz_cookies SET value=? WHERE id=?";
		int res = sqlite3_prepare(db_, sql.data(), sql.size(), &stmt, nullptr);
		if (res != SQLITE_OK) {
			GTRACE("sqlite3_prepare(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
			return false;
		}

		sqlite3_bind_text(stmt, 1, value.data(), value.size(), nullptr);
		sqlite3_bind_int(stmt, 2, currentId);

		res = sqlite3_step(stmt);
		if (res != SQLITE_DONE) {
			GTRACE("sqlite3_step(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
			return false;
		}
		return true;
	}

	sql = "INSERT INTO moz_cookies (id, name, value, host, path, originAttributes, expiry, lastAccessed, creationTime) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)";
	res = sqlite3_prepare(db_, sql.data(), sql.size(), &stmt, nullptr);
	if (res != SQLITE_OK) {
		GTRACE("sqlite3_prepare(%s) return %d %s", sql.data(), res, sqlite3_errmsg(db_));
		return false;
	}

	sqlite3_bind_int(stmt, 1, id);
	sqlite3_bind_text(stmt, 2, name.data(), name.size(), nullptr);
	sqlite3_bind_text(stmt, 3, value.data(), value.size(), nullptr);
	sqlite3_bind_text(stmt, 4, host.data(), host.size(), nullptr);
	sqlite3_bind_text(stmt, 5, path.data(), path.size(), nullptr);
	sqlite3_bind_text(stmt, 6, originAttributes.data(), originAttributes.size(), nullptr);
	sqlite3_bind_int64(stmt, 7, now);
	sqlite3_bind_int64(stmt, 8, now * 1000000);
	sqlite3_bind_int64(stmt, 9, now * 1000000);

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
