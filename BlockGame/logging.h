#pragma once
#include <stdio.h>

/*
 * A basic macro that makes it so that logging doesn't
 * occur when the game is built in release mode.
 *
 * The macro is simply a wrapper around the printf function
 * available in the c standard library.
 */
#ifdef _DEBUG
	#define LOG(str, ...) printf(str, __VA_ARGS__)
#else
	#define LOG(str, ...)
#endif