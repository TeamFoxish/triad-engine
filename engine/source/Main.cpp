#include "core/Game.h"

#if defined(_WIN32) && defined(WIN_MAIN)

#include <windows.h>

int CALLBACK WinMain(
	HINSTANCE   hInstance,
	HINSTANCE   hPrevInstance,
	LPSTR       lpCmdLine,
	int         nCmdShow
) 
{

#else

// TEMP: use main to show console window
int main(int argc, char* argv[])
{

#endif // _WIN32

	Game game;
	bool success = game.Initialize("TriadEngine", 800, 800);
	if (success)
	{
		game.RunLoop();
	}
	game.Shutdown();

	return 0;
}
