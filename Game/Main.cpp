#include <OpenGL3D/Game/OglGame.h>
#include <iostream>

/// <summary>
/// that is the main starting point of the engine
/// </summary>
/// <returns></returns>
int main() 
{
	try
	{
		//inherit of OglGame to "game" 
		OglGame game;

		//run the method "run"
		game.run();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return 1;
	}
	
	return 0;
}