#include "VisualDebugger.h"
#include <vector>
#include "Extras\Camera.h"
#include "Extras\Renderer.h"
#include "Extras\HUD.h"

namespace VisualDebugger
{
	using namespace physx;

	enum RenderMode
	{
		DEBUG,
		NORMAL,
		BOTH
	};

	enum HUDState
	{
		EMPTY = 0,
		HELP = 1,
		PAUSE = 2
	};

	//function declarations
	void KeyHold();
	void KeySpecial(int key, int x, int y);
	void KeyRelease(unsigned char key, int x, int y);
	void KeyPress(unsigned char key, int x, int y);

	void motionCallback(int x, int y);
	void mouseCallback(int button, int state, int x, int y);
	void exitCallback(void);

	void RenderScene();
	void ToggleRenderMode();
	void HUDInit();

	///simulation objects
	Camera* camera;
	PhysicsEngine::MyScene* scene;
	PxReal delta_time = 1.f/60.f;
	PxReal gForceStrength = 20;
	RenderMode render_mode = NORMAL;
	const int MAX_KEYS = 256;
	bool key_state[MAX_KEYS];
	bool hud_show = true;
	HUD hud;

	PxReal swingPower = 2000;

	//Init the debugger
	void Init(const char *window_name, int width, int height)
	{
		///Init PhysX
		PhysicsEngine::PxInit();
		scene = new PhysicsEngine::MyScene();
		scene->Init();

		///Init renderer
		Renderer::BackgroundColor(PxVec3(150.f/255.f,150.f/255.f,150.f/255.f));
		Renderer::SetRenderDetail(40);
		Renderer::InitWindow(window_name, width, height);
		Renderer::Init();


		//changed to give better view from start
		camera = new Camera(PxVec3(0.0f, 5.0f, 25.0f), PxVec3(0.f,-.1f,-1.f), 5.f);

		//initialise HUD
		HUDInit();

		///Assign callbacks
		//render
		glutDisplayFunc(RenderScene);

		//keyboard
		glutKeyboardFunc(KeyPress);
		glutSpecialFunc(KeySpecial);
		glutKeyboardUpFunc(KeyRelease);

		//mouse
		glutMouseFunc(mouseCallback);
		glutMotionFunc(motionCallback);

		//exit
		atexit(exitCallback);

		//init motion callback
		motionCallback(0,0);
	}

	void HUDInit()
	{
		//initialise HUD
		//add an empty screen
		hud.AddLine(EMPTY, "");
		//add a help screen
		hud.AddLine(HELP, " Fun golf game - once the ball hits the hole - you win!");
		hud.AddLine(HELP, "    F10 - pause");
		hud.AddLine(HELP, "    F12 - reset");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, " Display");
		hud.AddLine(HELP, "    F5 - instructions on/off");
		hud.AddLine(HELP, "    F6 - shadows on/off");
		hud.AddLine(HELP, "    F7 - render mode");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, " Camera");
		hud.AddLine(HELP, "    1,2,3,4,5,6 - forward,backward,left,right,up,down");
		hud.AddLine(HELP, "    mouse + click - change orientation");
		hud.AddLine(HELP, "    F8 - reset view");
		hud.AddLine(HELP, "");
		hud.AddLine(HELP, " Hit the golf ball and move the club");
		hud.AddLine(HELP, "    W,A,S - Club forward, Club left, Club backward");
		hud.AddLine(HELP, "D,Q,R,O,P - Club right, Club up, Club down, Swing, Change power");
		hud.AddLine(HELP, "Space, - Hit the golf ball after swing");	
		hud.AddLine(HELP, "Z,X,C, - Rotate left, Reset, Rotate Right");
		//add a pause screen
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "");
		hud.AddLine(PAUSE, "   Simulation paused. Press F10 to continue.");
		//set font size for all screens
		hud.FontSize(0.018f);
		//set font color for all screens
		hud.Color(PxVec3(0.f,0.f,0.f));
	}

	//Start the main loop
	void Start()
	{ 
		glutMainLoop(); 
	}

	//Render the scene and perform a single simulation step
	void RenderScene()
	{
		//handle pressed keys
		KeyHold();

		//start rendering
		Renderer::Start(camera->getEye(), camera->getDir());

		if ((render_mode == DEBUG) || (render_mode == BOTH))
		{
			Renderer::Render(scene->Get()->getRenderBuffer());
		}

		if ((render_mode == NORMAL) || (render_mode == BOTH))
		{
			std::vector<PxActor*> actors = scene->GetAllActors();
			if (actors.size())
				Renderer::Render(&actors[0], (PxU32)actors.size());
		}

		//adjust the HUD state
		if (hud_show)
		{
			if (scene->Pause())
				hud.ActiveScreen(PAUSE);
			else
				hud.ActiveScreen(HELP);
		}
		else
			hud.ActiveScreen(EMPTY);

		//render HUD
		hud.Render();

		//finish rendering
		Renderer::Finish();

		//perform a single simulation step
		scene->Update(delta_time);
	}

	//user defined keyboard handlers
	void UserKeyPress(int key)
	{
		switch (toupper(key))
		{
		//implement your own
		case 'Z': 
			scene->RotateLeft();
			break;
		case 'C':
			scene->RotateRight();
			break;
		case 'X':
			scene->ResetRot();
			break;
		default:
			break;
		}
	}

	void UserKeyRelease(int key)
	{
		switch (toupper(key))
		{
		//spacebar to hit would suit this, most games utilise this 
		case ' ':
			scene->Hit();
			break;
		default:
			break;
		}
	}

	void UserKeyHold(int key)
	{
		//keys to hit golf ball with club
		switch (toupper(key))
		{
		case 'W': //forward
		//scene->GolfBall->Get()->isRigidBody()->addTorque(PxVec3(-1, 0, 0)*(swingPower * gForceStrength));
			scene->HitForward();
			break;
		case 'S': //backward
			scene->HitBack();
			break;
		case 'A': //left
			scene->HitLeft();
			break;
		case 'D': //right
			scene->HitRight();
			break;
		case 'Q': //up
			scene->MoveUp();
			break;
		case 'R': //down
			scene->MoveDown();
			break;
		case 'O': //Swing back
			scene->Swing();
			break;
		case 'P': //Speed control
			scene->SetPower();
			break;
		default:
			break;
		}
	}

	//handle camera control keys
	void CameraInput(int key)
	{
		switch (key)
		{
		case '1':
			camera->MoveForward(delta_time);
			break;
		case '2':
			camera->MoveBackward(delta_time);
			break;
		case '3':
			camera->MoveLeft(delta_time);
			break;
		case '4':
			camera->MoveRight(delta_time);
			break;
		case '5':
			camera->MoveUp(delta_time);
			break;
		case '6':
			camera->MoveDown(delta_time);
			break;
		default:
			break;
		}
	}

	//handle force control keys
	void ForceInput(int key)
	{
		//if (!scene->GetSelectedActor())
		//	return;

		//switch (toupper(key))
		//{
		//	// Force controls on the selected actor
		//case 'I': //forward

		//	scene->GetSelectedActor()->addForce(PxVec3(0, 0, -1) * gForceStrength);
		//	break;
		//case 'K': //backward
		//	scene->GetSelectedActor()->addForce(PxVec3(0,0,1)*gForceStrength);
		//	break;
		//case 'J': //left
		//	scene->GetSelectedActor()->addForce(PxVec3(-1,0,0)*gForceStrength);
		//	break;
		//case 'L': //right
		//	scene->GetSelectedActor()->addForce(PxVec3(1,0,0)*gForceStrength);
		//	break;
		//case 'U': //up
		//	scene->GetSelectedActor()->addForce(PxVec3(0,1,0)*gForceStrength);
		//	break;
		//case 'M': //down
		//	scene->GetSelectedActor()->addForce(PxVec3(0,-1,0)*gForceStrength);
		//	break;
		//default:
		//	break;
		//}
	}

	///handle special keys
	void KeySpecial(int key, int x, int y)
	{
		//simulation control
		switch (key)
		{
			//display control
		case GLUT_KEY_F5:
			//hud on/off
			hud_show = !hud_show;
			break;
		case GLUT_KEY_F6:
			//shadows on/off
			Renderer::ShowShadows(!Renderer::ShowShadows());
			break;
		case GLUT_KEY_F7:
			//toggle render mode
			ToggleRenderMode();
			break;
		case GLUT_KEY_F8:
			//reset camera view
			camera->Reset();
			break;

		//	//simulation control
		//case GLUT_KEY_F9:
		//	//select next actor
		//	scene->SelectNextActor();
		//	break;
		case GLUT_KEY_F10:
			//toggle scene pause
			scene->Pause(!scene->Pause());
			break;
		case GLUT_KEY_F12:
			//resect scene
			scene->Reset();
			break;
		default:
			break;
		}
	}

	//handle single key presses
	void KeyPress(unsigned char key, int x, int y)
	{

		
		//do it only once
		if (key_state[key] == true)
			return;

		key_state[key] = true;

		//exit
		if (key == 27)
			exit(0);

		UserKeyPress(key);
	}

	//handle key release
	void KeyRelease(unsigned char key, int x, int y)
	{
		key_state[key] = false;
		UserKeyRelease(key);
	}

	//handle holded keys
	void KeyHold()
	{
		for (int i = 0; i < MAX_KEYS; i++)
		{
			if (key_state[i]) // if key down
			{
				CameraInput(i);
				ForceInput(i);
				UserKeyHold(i);
			}
		}
	}

	///mouse handling
	int mMouseX = 0;
	int mMouseY = 0;

	void motionCallback(int x, int y)
	{
		int dx = mMouseX - x;
		int dy = mMouseY - y;

		camera->Motion(dx, dy, delta_time);

		mMouseX = x;
		mMouseY = y;
	}

	void mouseCallback(int button, int state, int x, int y)
	{
		mMouseX = x;
		mMouseY = y;
	}

	void ToggleRenderMode()
	{
		if (render_mode == NORMAL)
			render_mode = DEBUG;
		else if (render_mode == DEBUG)
			render_mode = BOTH;
		else if (render_mode == BOTH)
			render_mode = NORMAL;
	}

	///exit callback
	void exitCallback(void)
	{
		delete camera;
		delete scene;
		PhysicsEngine::PxRelease();
	}
}

