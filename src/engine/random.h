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

typedef unsigned long long u64;
typedef unsigned int u32;

class _Random {

    public:

        _Random();
        _Random(u32 TSeed);

        void SetSeed(u32 TSeed);

        double Generate();
        u32 Generate(u32 TMax);
        int GenerateRange(int TMin, int TMax);
        u32 GenerateRange(u32 TMin, u32 TMax);
        double GenerateRange(double TMin, double TMax);

	private:

		// Base function
		u32 GenerateRandomInteger();

		// Seeds
		u32 Q[1024];
};

// Constructor
inline _Random::_Random() {

    SetSeed(0);
}

// Constructor
inline _Random::_Random(u32 TSeed) {

    SetSeed(TSeed);
}

// Sets the seed for the generator
inline void _Random::SetSeed(u32 TSeed) {

    for(u32 i = 0; i < 1024; i++){
		TSeed ^= TSeed << 13;
		TSeed ^= TSeed >> 17;
		TSeed ^= TSeed << 5;
		Q[i] = TSeed;
	}
}

// Generates a random integer
inline u32 _Random::GenerateRandomInteger() {
   static u32 c = 8471623, i = 1023;
	u64 t, a = 123471786LL;
	u32 x, r = 0xfffffffe;

	i = (i + 1) & 1023;
	t = a * Q[i] + c;
	c = t >> 32;
	x = (u32)(t + c);
	if(x < c) {
		x++;
		c++;
	}

	return Q[i] = r - x;
}

// Generates a random number [0, 1)
inline double _Random::Generate() {

	return GenerateRandomInteger() / 4294967296.0;
}

// Generates a random number [0, TMax-1]
inline u32 _Random::Generate(u32 TMax) {

    return (u32)(Generate() * TMax);
}

// Generates a random number [TMin, TMax]
inline int _Random::GenerateRange(int TMin, int TMax) {

    return (int)(Generate() * (TMax - TMin + 1)) + TMin;
}

// Generates a random number [TMin, TMax]
inline u32 _Random::GenerateRange(u32 TMin, u32 TMax) {

    return (u32)(Generate() * (TMax - TMin + 1)) + TMin;
}

// Generates a random number [TMin, TMax]
inline double _Random::GenerateRange(double TMin, double TMax) {

    return (GenerateRandomInteger() / 4294967295.0) * (TMax - TMin) + TMin;
}

extern _Random Random;
