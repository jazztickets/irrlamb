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
#pragma once
#include <sqlite3.h>

// Classes
class _Database {

	public:

		_Database();
		~_Database();

		int OpenDatabase(const char *Filename);
		int OpenDatabaseCreate(const char *Filename);

		int RunQuery(const char *QueryString);
		int RunDataQuery(const char *QueryString, int Handle=0);
		int RunCountQuery(const char *QueryString);
		int FetchRow(int Handle=0);
		int CloseQuery(int Handle=0);
		sqlite3_int64 GetLastInsertID();

		int GetInt(int ColumnIndex, int Handle=0);
		float GetFloat(int ColumnIndex, int Handle=0);
		const char *GetString(int ColumnIndex, int Handle=0);

	private:

		sqlite3 *Database;
		sqlite3_stmt *QueryHandle[2];

};
