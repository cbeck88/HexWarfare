/*
   Copyright 2014 Kristina Simpson <sweet.kristas@gmail.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

#ifndef SERVER_BUILD

#include "SDL.h"
#include <iostream>
#include <string>

namespace profile 
{
	struct manager
	{
		Uint64 frequency;
		Uint64 t1, t2;
		double elapsedTime;
		const char* name;

		manager(const char* const str) : name(str)
		{
			frequency = SDL_GetPerformanceFrequency();
			t1 = SDL_GetPerformanceCounter();
		}

		~manager()
		{
			t2 = SDL_GetPerformanceCounter();
			elapsedTime = (t2 - t1) * 1000.0 / frequency;
			std::cerr << name << ": " << elapsedTime << " milliseconds" << std::endl;
		}
	};

	struct timer
	{
		Uint64 frequency;
		Uint64 t1, t2;

		timer()
		{
			frequency = SDL_GetPerformanceFrequency();
			t1 = SDL_GetPerformanceCounter();
		}

		// Elapsed time in milliseconds
		double get_time()
		{
			t2 = SDL_GetPerformanceCounter();
			return (t2 - t1) * 1000000.0 / frequency;
		}
	};

	inline void sleep(double t) 
	{
		SDL_Delay(static_cast<Uint32>(t * 1000.0));
	}
}

#else

#include <chrono>
#include <thread>

namespace profile
{
	struct manager
	{
		manager(const char* const str) : name(str) {}
		const char* name;
	};

	struct timer
	{
		timer() {}
		double get_time() { return 0; }
	};

	inline void sleep(double t) 
	{
		std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(t*1000000.0)));
	}
}
#endif
