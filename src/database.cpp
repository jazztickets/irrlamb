/******************************************************************************
* irrlamb - https://github.com/jazztickets/irrlamb
* Copyright (C) 2019  Alan Witkowski
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#include <database.h>
#include <log.h>
#include <cstring>
#include <cassert>

// Constructor
_Database::_Database() :
	Database(nullptr) {

	QueryHandle[0] = nullptr;
	QueryHandle[1] = nullptr;
}

// Destructor
_Database::~_Database() {

	// Close database
	if(Database)
		sqlite3_close(Database);
}

// Load a database file
int _Database::OpenDatabase(const char *Filename) {

	// Open database file
	int Result = sqlite3_open_v2(Filename, &Database, SQLITE_OPEN_READWRITE, nullptr);
	if(Result != SQLITE_OK) {
		Log.Write("Failed to open sqlite database: %s", sqlite3_errmsg(Database));
		sqlite3_close(Database);

		return 0;
	}

	return 1;
}

// Load a database file
int _Database::OpenDatabaseCreate(const char *Filename) {

	// Open database file
	int Result = sqlite3_open_v2(Filename, &Database, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
	if(Result != SQLITE_OK) {
		Log.Write("Failed to open sqlite database for creating: %s", sqlite3_errmsg(Database));
		sqlite3_close(Database);

		return 0;
	}

	return 1;
}

// Runs a query
int _Database::RunQuery(const char *QueryString) {

	sqlite3_stmt *NewQueryHandle;
	const char *Tail;
	int Result = sqlite3_prepare_v2(Database, QueryString, strlen(QueryString), &NewQueryHandle, &Tail);
	if(Result != SQLITE_OK) {
		Log.Write("sqlite3_prepare_v2 failed: %s", sqlite3_errmsg(Database));
		return 0;
	}

	Result = sqlite3_step(NewQueryHandle);
	if(Result != SQLITE_DONE && Result != SQLITE_ROW) {
		Log.Write("sqlite3_step failed: %s", sqlite3_errmsg(Database));
		return 0;
	}

	Result = sqlite3_finalize(NewQueryHandle);
	if(Result != SQLITE_OK) {
		Log.Write("sqlite3_finalize failed: %s", sqlite3_errmsg(Database));
		return 0;
	}

	return 1;
}

// Runs a query that returns data
int _Database::RunDataQuery(const char *QueryString, int Handle) {
	assert(QueryHandle[Handle] == nullptr);

	const char *Tail;
	int Result = sqlite3_prepare_v2(Database, QueryString, strlen(QueryString), &QueryHandle[Handle], &Tail);
	if(Result != SQLITE_OK) {
		Log.Write("sqlite3_prepare_v2 failed: %s", sqlite3_errmsg(Database));
		return 0;
	}

	return 1;
}

// Runs a query that counts a row and returns the result
int _Database::RunCountQuery(const char *QueryString) {

	RunDataQuery(QueryString);
	FetchRow();
	int Count = GetInt(0);

	CloseQuery();

	return Count;
}

// Fetch one row from a query
int _Database::FetchRow(int Handle) {

	int Result = sqlite3_step(QueryHandle[Handle]);
	switch(Result) {
		case SQLITE_ROW:
			return 1;
		break;
		case SQLITE_DONE:
		break;
		default:
		break;
	}

	return 0;
}

// Closes a query
int _Database::CloseQuery(int Handle) {

	int Result = sqlite3_finalize(QueryHandle[Handle]);
	if(Result != SQLITE_OK) {
		Log.Write("sqlite3_finalize failed: %s", sqlite3_errmsg(Database));
		return 0;
	}
	QueryHandle[Handle] = nullptr;

	return 1;
}

// Gets the last insert id
sqlite3_int64 _Database::GetLastInsertID() {

	return sqlite3_last_insert_rowid(Database);
}

// Returns an integer column
int _Database::GetInt(int ColumnIndex, int Handle) {

	return sqlite3_column_int(QueryHandle[Handle], ColumnIndex);
}

// Returns a float column
float _Database::GetFloat(int ColumnIndex, int Handle) {

	return static_cast<float>(sqlite3_column_double(QueryHandle[Handle], ColumnIndex));
}

// Returns a string column
const char *_Database::GetString(int ColumnIndex, int Handle) {

	return (const char *)sqlite3_column_text(QueryHandle[Handle], ColumnIndex);
}
