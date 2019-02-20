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
#include <log.h>
#include <save.h>
#include <iostream>
#include <cstdarg>

_Log Log;

// Initializes the log system
int _Log::Init() {

	// Open log
	std::string FilePath = Save.SavePath + "irrlamb.log";
	FileStream.open(FilePath.c_str());

	return 1;
}

// Closes the log system
int _Log::Close() {

	// Close file
	FileStream.close();

	return 1;
}

// Writes a string to the log file
void _Log::Write(const char *Line, ...) {
	va_list ArgumentList;
	char Buffer[1024];

	// Get output
	va_start(ArgumentList, Line);
	vsnprintf(Buffer, 1024, Line, ArgumentList);
	va_end(ArgumentList);

	// Write line
	std::cout << Buffer << std::endl;
	FileStream << Buffer << std::endl;
}
