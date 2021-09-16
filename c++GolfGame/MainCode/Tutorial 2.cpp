//CGP3012M Physics Simulation 
//Feargus Powers 16654184
//Extension Authorisation Code: 66CP30PZXXCBLDQR

//Report 
//This code shows creates a simple golf game where the player has a course which is a straight line, but a fantasy twist with a moving platform to get past. 
//The flag on the pole on the hole uses a material class to give the look like wind is moving it. This is positioned after the moving platform which is part of the 'fantasy twist'. This moves up and down through a timer that is set to control
// how often it does this. 
//This is created using a revolute joint and in a similar fashion to the way the club moves up and down, the actor (in this case a club or the moving platform) has a box above it which has to be positioned with
//the right values, so that when the revolute joint is created, it can move the actor appropriately. 
//Another feature of this to add some more challenge to the game, is when the ball misses the moving platform and falls onto the floor slightly below, its position is reset using a simple function that moves the ball
//back to its original starting position. 
//Materials are also used on the walls for the course, these all have the same to give consistency in the design and to make it look close to a real golf course, while the ground is also a green colour, to also
//reflect the true look of a golf course. 
//The camera values were tweaked in the 'visualdebugger.ccp' file so that the z axis pushed back to give the player a better view when they start the game. The controls were also changed, with the help text shown 
//on screen to reflect that. It is WASD to move the club, forward, left, backward and right. Q and R to go up and down, O to swing the golf club back and get ready for a hit, with space to actually swing and potentially hit the ball.
//Additionally, p is used to set the power/speed of the club, this can be seen shown in console when player presses it. Finally, to move the camera, 1,2,3,4,5, and 6 are used to go forward, backward, left, right, up and down. 
//Additional features are added such as the tee box and a int value for keeping score of how many hits the player attempts. This is incremented when the player tries to hit the ball and is shown in the console their score after they pot the ball.
//This reflects a score in golf where the higher your number the worse you played. Finally, a course is present to the right of the player, this just adds an element of potential confusion and some more depth to the scene. 
//Overall, all the elements are there for players to play a fun game of mini golf with a challenging twist. 

#include <iostream>
#include "VisualDebugger.h"

using namespace std;

int main()
{
	try 
	{ 
		VisualDebugger::Init("Physics Simulation Golf Game", 800, 800); 
	}
	catch (Exception exc) 
	{ 
		cerr << exc.what() << endl;
		return 0; 
	}

	VisualDebugger::Start();

	return 0;
}