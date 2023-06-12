#include <iostream>
#include "logging.h"
#include "game.h"
#include <FastNoise/FastNoise.h>

/*
 * The entry point for the game.
 */
int main(int argc, char **argv)
{
	LOG("Launching the game!\n");
	Game game = Game();
	game.Run();
	return 0;
}