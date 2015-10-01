/******************************************************************************
* irrlamb - https://github.com/jazztickets/irrlamb
* Copyright (C) 2015  Alan Witkowski
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

// Libraries
#include <fstream>
#include <cstddef>

// Classes
class _File {

	public:

		int OpenForWrite(const char *Filename);
		int OpenForRead(const char *Filename);
		void Clear() { File.clear(); }
		void Close() { File.close(); File.clear(); }
		bool Eof() { return File.eof() || File.peek() == EOF; }
		void Flush() { File.flush(); }
		void Ignore(size_t Size) { File.ignore(Size); }

		void WriteChar(uint8_t Data) { File.put(Data); }
		void WriteInt32(int32_t Data) { File.write(reinterpret_cast<char *>(&Data), sizeof(Data)); }
		void WriteInt16(int16_t Data) { File.write(reinterpret_cast<char *>(&Data), sizeof(Data)); }
		void WriteFloat(float Data) { File.write(reinterpret_cast<char *>(&Data), sizeof(Data)); }
		void WriteData(void *Data, uint32_t Size) { File.write(reinterpret_cast<char *>(Data), Size); }
		void WriteString(const char *Data, uint32_t Size) { File.write(Data, Size); }

		uint8_t ReadChar() { return File.get(); }
		int32_t ReadInt32();
		int16_t ReadInt16();
		float ReadFloat();
		void ReadData(void *Data, uint32_t Size) { File.read(reinterpret_cast<char *>(Data), Size); }
		void ReadString(char *Data, uint32_t Size) { File.read(reinterpret_cast<char *>(Data), Size); }

	private:

		std::fstream File;

};
