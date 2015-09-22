/*************************************************************************************
*	irrlamb - https://github.com/jazztickets/irrlamb
*	Copyright (C) 2015  Alan Witkowski
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************************/
#pragma once

// Libraries
#include <fstream>
#include <stddef.h>

// Classes
class _File {

	public:

		int OpenForWrite(const char *Filename);
		int OpenForRead(const char *Filename);
		void Clear() { File.clear(); }
		void Close() { File.close(); File.clear(); }
		bool Eof() { return File.eof(); }
		void Flush() { File.flush(); }
		void Ignore(size_t Size) { File.ignore(Size); }

		void WriteChar(unsigned char Data) { File.put(Data); }
		void WriteInt(int Data) { File.write(reinterpret_cast<char *>(&Data), sizeof(Data)); }
		void WriteShortInt(short int Data) { File.write(reinterpret_cast<char *>(&Data), sizeof(Data)); }
		void WriteFloat(float Data) { File.write(reinterpret_cast<char *>(&Data), sizeof(Data)); }
		void WriteData(void *Data, unsigned int Size) { File.write(reinterpret_cast<char *>(Data), Size); }
		void WriteString(const char *Data);
		void WriteString(const char *Data, unsigned int Size) { File.write(Data, Size); }

		unsigned char ReadChar() { return File.get(); }
		int ReadInt();
		short int ReadShortInt();
		float ReadFloat();
		void ReadData(void *Data, unsigned int Size) { File.read(reinterpret_cast<char *>(Data), Size); }
		void ReadString(char *Data);
		void ReadString(char *Data, unsigned int Size) { File.read(reinterpret_cast<char *>(Data), Size); }

	private:

		std::fstream File;

};
