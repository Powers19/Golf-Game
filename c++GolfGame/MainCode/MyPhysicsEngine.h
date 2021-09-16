
#pragma once

#include "BasicActors.h"
#include "VisualDebugger.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	using namespace std;
	

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = { PxVec3(46.f / 255.f,9.f / 255.f,39.f / 255.f),PxVec3(217.f / 255.f,0.f / 255.f,0.f / 255.f),
		PxVec3(255.f / 255.f,45.f / 255.f,0.f / 255.f),PxVec3(255.f / 255.f,140.f / 255.f,54.f / 255.f),PxVec3(4.f / 255.f,117.f / 255.f,111.f / 255.f), PxVec3(0.f, 255.0f, 0.f) };


	//struct FilterGroup
	//{
	//	enum Enum
	//	{
	//		BALL = (1 << 3),
	//		FLAGPOLE = (1 << 4),
	//		GOLFCLUB = (1 << 5)
	//		//add more if you need
	//	};
	//};

	

	///A customised collision class, implementting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//an example variable that will be checked in the main simulation loop
		bool trigger;
		//controls if player wins - used to organise code more
		bool playerWins = false;

		MySimulationEventCallback() : trigger(false) {}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count)
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						//cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << endl;

						trigger = true;
						if (pairs[i].otherActor->getName(), "Hole")
						{
							//then you have putted the ball into the hole which is the main objective - you will win. 
							playerWins = true;
						}
					}
					
				}
			}
		}

		

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
		{
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					cerr << "OnContact::eNOTIFY_TOUCH_FOUND" << endl;
				}
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					cerr << "OnContact::eNOTIFY_TOUCH_LOST" << endl;
				}
				/*if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_CCD)
				{
					cerr << "OnContact::eNOTIFY_TOUCH_CCD" << endl;
				}*/
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) {}
		virtual void onWake(PxActor** actors, PxU32 count) {}
		virtual void onSleep(PxActor** actors, PxU32 count) {}
#if PX_PHYSICS_VERSION >= 0x304000
		virtual void onAdvance(const PxRigidBody* const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) {}
#endif
	};

	//A simple filter shader based on PxDefaultSimulationFilterShader - without group filtering
	static PxFilterFlags CustomFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		// let triggers through
		if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;
		//enable continous collision detection
		//pairFlags |= PxPairFlag::eCCD_LINEAR;


		

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			//trigger onContact callback for this pair of objects
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
			//pairFlags |= PxPairFlag::eNOTIFY_TOUCH_CCD;
			//pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;

			//return PxFilterFlag::eKILL | PxFilterFlag::eNOTIFY;
		}

		return PxFilterFlags();
	};

	///Custom scene class
	class MyScene : public Scene
	{

		MySimulationEventCallback* my_callback;

		Plane* plane;
		Box* box;
		Box* floorJoint;
		Box* putter;
		CompoundObject* Cbox;
		Edge* edge;
		Box* movingFloor;
		GolfClub* club;
		//GolfBall* golfBall;
		Sphere* circle;
		Cloth* flag;

		RevoluteJoint* clubJoint;
		RevoluteJoint* fJoint;
		Box* holeTrigger;
		StatBox* pole;
		StatBox* teeBox;

		PxTransform jointLocationClub;
		PxTransform floorJointLocation;

		PxReal hitPower = 0;

		bool swing = false;
		//timer for moving platforms - gives the fantasy element 
		
		float time;
		float countDown = 0.1f;
		
		//keeps track of player hits - use for score
		int hitCount = 0;
		

		PxMaterial* grassMat = GetPhysics()->createMaterial(0.3f, 0.2f, 0.2f);
		PxMaterial* wall;
		PxMaterial* ballMat;

		GolfBall* golfBall;
		PxReal golfBall_x = 0.5f;
		PxReal golfBall_y = .8f;
		PxReal golfBall_z = 11.0f;

	public:
		MyScene() : Scene(CustomFilterShader) {};
		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit() 
		{
			SetVisualisation();			

			GetMaterial()->setDynamicFriction(.2f);

			//call relevant classes for collision and trigger
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);

			//materials
			wall = GetPhysics()->createMaterial(0.4f, 0.6f, 0.4f);
			ballMat = GetPhysics()->createMaterial(.4f, .15f, .4f);
			ballMat->setFrictionCombineMode(PxCombineMode::eAVERAGE);

		

			plane = new Plane();
			plane->Color(color_palette[4]);
			//plane->Material(grassMat);
			Add(plane);

			//golf ball added to course 
			golfBall = new GolfBall(PxTransform(golfBall_x, golfBall_y, golfBall_z)), 10.0f, 1.0f;
			golfBall->Name("ball");
			golfBall->Color(color_palette[3]);
			((PxRigidDynamic*)golfBall->Get())->setMass(0.045f);
			//((PxRigidDynamic*)golfBall->Get())->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			//((PxRigidDynamic*)golfBall->Get())->setLinearDamping(.25f);
			golfBall->Material(ballMat);
			Add(golfBall);

			//tee box from the start
			teeBox = new StatBox(PxTransform(0.5f, -0.2f, 11.0f));
			teeBox->Color(color_palette[1]);
			Add(teeBox);

			//seperate functions for these to make it easier and for cleaner code. 
			AddEdges();
			AddCourse();

			putter = new Box(PxTransform(PxVec3(0.5f, 5.45f, 14.0f)));
			putter->SetKinematic(true);
			putter->Color(color_palette[1]);
			Add(putter);

			club = new GolfClub(PxTransform(PxVec3(0.5f, 4.0f, 15.5f)));
			club->Color(color_palette[2]);
			((PxRigidBody*)club->Get())->setMass(0.355f);
			Add(club);
#
			clubJoint = new RevoluteJoint(putter, PxTransform(PxVec3(0.0f, .0f, 0.0f), PxQuat(PxPi / 2, PxVec3(1.0f, 0.0f, 0.0f))), club, PxTransform(PxVec3(0.0f, 5.0f, 0.0f)));
			clubJoint->SetLimits(-PxPi / 1.4f, -PxPi / 3.5f);

			

			//fantasy element - moving floor
			movingFloor = new Box(PxTransform(PxVec3(.5f, 0.0f, -13.0f)), PxVec3(5.0f, 0.1f, 10.5f));
			movingFloor->Color(color_palette[5]);
			Add(movingFloor);

			//joint to give movement
			floorJoint = new Box(PxTransform(PxVec3(.5f, 5.0f, -13.0f)));
			floorJoint->SetKinematic(true);
			Add(floorJoint);

			fJoint = new RevoluteJoint(floorJoint, PxTransform(PxVec3(0.0f, -5.0f, 0.0f)), movingFloor, PxTransform(PxVec3(0.0f, 0.0f, 0.0f)));
			//fJoint->SetLimits(5.0f, 0.0f);
			
			
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LIMITS, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eJOINT_LOCAL_FRAMES, 1.0f);
		}

		//Custom udpate function
		virtual void CustomUpdate() 
		{
			jointLocationClub = ((PxRigidBody*)putter->Get())->getGlobalPose();

			
			floorJointLocation = ((PxRigidBody*)floorJoint->Get())->getGlobalPose();
			((PxRigidDynamic*)movingFloor->Get())->wakeUp();
			

			PxTransform ballPos = ((PxRigidBody*)golfBall->Get())->getGlobalPose();

			//if ball goes when the trap surface moves then restart 
			if (ballPos.p.y < 0.25)
			{
				printf("Touched floor, start again at the tee box");
				
				BallReset();
			}

			//sets a good height for the club to reset to if it gets too low 
			if (jointLocationClub.p.y <= 5.5f)
			{
				jointLocationClub.p.y = 5.5f;
			}

			
			//if player swings their club the add force to make it move 
			if (swing)
			{
				((PxRigidBody*)club->Get())->addForce(PxVec3(0.0f, 0.0f, 1.0f), PxForceMode::eIMPULSE);
				
			}

			//player win logic
			if (my_callback->playerWins)
			{
				cout << "YOU HAVE HIT THE HOLE. Your hit attempts: " + hitCount << endl;
				
				//reset ball to play again
				BallReset();
				my_callback->playerWins = false;
			}
			
			//controls logic for moving platform up and down - gives a twist to the course, also extra challenge
			//so the platform can stay up or down for a certain amount of time, ensuring the player has to time their swings and movement of the club. 
			if ((time > 400.f) && (time < 700.f))
			{
				//printf("Move platform down");
				MovePlatDown();
			}
			if ((time < 400.f) && (time > 100.f))
			{
				//printf("Move platform up");
				MovePlatup();
			}
			//gives timer for value if not 900
			if (time != 900.f)
			{
				time++;
			}
			//resets timer if met at max value 
			if (time == 900.f)
			{
				time = 0;
			}
			
		}

	

		virtual void AddEdges()
		{
			//left side
			edge = new Edge(PxTransform(PxVec3(-6.f, 1.f, -14.95f)) , PxVec3(.5f, 1.0f, 31.5f));
			edge->Color(PxVec3(60.f / 255.f, 60.f / 255.f, 60.f / 255.f));
			edge->Material(wall);
			Add(edge);

			//right side 
			edge = new Edge(PxTransform(PxVec3(7.f, 1.f, -14.95f)), PxVec3(.5f, 1.0f, 31.5f));
			edge->Color(PxVec3(60.f / 255.f, 60.f / 255.f, 60.f / 255.f));
			edge->Material(wall);
			Add(edge);

			//back wall next to hole part
			edge = new Edge(PxTransform(PxVec3(0.2f, 1.f, -46.f)), PxVec3(6.8f, 1.0f, 0.8f));
			edge->Color(PxVec3(60.f / 255.f, 60.f / 255.f, 60.f / 255.f));
			edge->Material(wall);
			Add(edge);

			//walls to add bit of confusion to current course
			//right hand side
			edge = new Edge(PxTransform(PxVec3(35.2f, 1.f, -25.f)), PxVec3(21.4f, 1.0f, .5f));
			edge->Color(PxVec3(60.f / 255.f, 60.f / 255.f, 60.f / 255.f));
			edge->Material(wall);
			Add(edge);
			
			//left side wall
			edge = new Edge(PxTransform(PxVec3(35.2f, 1.f, -13.f)), PxVec3(21.4f, 1.0f, .5f));
			edge->Color(PxVec3(60.f / 255.f, 60.f / 255.f, 60.f / 255.f));
			edge->Material(wall);
			Add(edge);

			//back wall at extra course
			edge = new Edge(PxTransform(PxVec3(56.0f, 1.f, -19.f)), PxVec3(0.8f, 1.0f, 6.0f));
			edge->Color(PxVec3(60.f / 255.f, 60.f / 255.f, 60.f / 255.f));
			edge->Material(wall);
			Add(edge);
		}

		virtual void AddCourse()
		{
			//grass for start of course, just before moving traps 
			edge = new Edge(PxTransform(PxVec3(.5f, 0.1f, 7.f)), PxVec3(6.0f, 0.1f, 9.5f));
			edge->Color(color_palette[5]);
			Add(edge);

			//grass turning right
			edge = new Edge(PxTransform(PxVec3(35.0f, 0.1f, -19.f)), PxVec3(21.5f, 0.1f, 6.5f));
			edge->Color(color_palette[5]);
			Add(edge);

			//grass for actual hole
			edge = new Edge(PxTransform(PxVec3(.5f, 0.1f, -35.f)), PxVec3(6.0f, 0.1f, 11.5f));
			edge->Color(color_palette[5]);
			Add(edge);

			//core golf elements
			//golf hole 
			holeTrigger = new Box(PxTransform(PxVec3(0.5f, 0.3f, -35.f)), PxVec3(1.0f, 0.1f, 1.0f));
			holeTrigger->SetKinematic(true);
			holeTrigger->GetShape(0)->setFlag(PxShapeFlag::eSIMULATION_SHAPE, false);
			holeTrigger->GetShape(0)->setFlag(PxShapeFlag::eTRIGGER_SHAPE, true);
			holeTrigger->Color(color_palette[1]);
			holeTrigger->Name("Hole");
			Add(holeTrigger);


			//flag pole to show user where the hole is more clearly - gives clearer objectives 
			//pole used as box and tall and thin to show pole shape
			pole = new StatBox(PxTransform(PxVec3(0.5f, 4.5f, -35.f)), PxVec3(0.1f, 4.0f, 0.1f));
			pole->Color(color_palette[3]);
			Add(pole);

			//flag added using cloth class with material used.  
			flag = new Cloth(PxTransform(PxVec3(0.5f, 6.f, -35.f), PxQuat(PxPi / 2, PxVec3(0.f, 0.f, 1.f))), PxVec2(4.f, 4.f), 40, 40, true);
			((PxCloth*)flag->Get())->setExternalAcceleration(PxVec3(5.f, 8.f, 5.f));
			flag->Color(color_palette[0]);
			Add(flag);
		}

		


		//resets ball after hit
		void BallReset()
		{
			((PxRigidBody*)golfBall->Get())->setGlobalPose(PxTransform(golfBall_x, golfBall_y, golfBall_z));
			((PxRigidBody*)golfBall->Get())->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
			((PxRigidBody*)golfBall->Get())->setAngularVelocity(PxVec3(0.0f, 0.0f, 0.0f));

		}

		//ensures the club is not swinging again once hit has been made - also hit count incremented to reflect potential final score 
		void Hit()
		{
			swing = false;
			((PxRigidBody*)club->Get())->addForce(PxVec3(0.0f, 0.0f, -hitPower), PxForceMode::eIMPULSE);
			hitCount += 1;
			cout << hitCount << endl;
		}

		void Swing()
		{
			((PxRigidBody*)club->Get())->addForce(PxVec3(0.0f, 0.0f, 1.0f), PxForceMode::eIMPULSE);
			
			swing = true;
			hitPower = 0;
		}

		void SetPower()
		{
			//lets players know of what power they have ready 
			cout << hitPower << endl;
			//player can change this when they press p key, ensures it doesn't go over 25 
			if (hitPower <= 25.f)
			{
				hitPower+= 1.0f;
				
			}
			//sets default 
			else
			{
				hitPower = 5;
			}
		}

		//functions for moving the club, uses the joint location values of the club, which gets the position and then adds value in particular axis depending on action performed. 
		void HitForward() const
		{
			((PxRigidBody*)putter->Get())->setGlobalPose(PxTransform(PxVec3(jointLocationClub.p.x, jointLocationClub.p.y, jointLocationClub.p.z - .05f), PxQuat(jointLocationClub.q.x, jointLocationClub.q.y, jointLocationClub.q.z, jointLocationClub.q.w)));
		}

		void HitBack() const
		{
			((PxRigidBody*)putter->Get())->setGlobalPose(PxTransform(PxVec3(jointLocationClub.p.x, jointLocationClub.p.y, jointLocationClub.p.z + .05f), PxQuat(jointLocationClub.q.x, jointLocationClub.q.y, jointLocationClub.q.z, jointLocationClub.q.w)));
		}

		void HitLeft() const
		{
			((PxRigidBody*)putter->Get())->setGlobalPose(PxTransform(PxVec3(jointLocationClub.p.x - .05f, jointLocationClub.p.y, jointLocationClub.p.z), PxQuat(jointLocationClub.q.x, jointLocationClub.q.y, jointLocationClub.q.z, jointLocationClub.q.w)));
		}

		void HitRight() const
		{
			((PxRigidBody*)putter->Get())->setGlobalPose(PxTransform(PxVec3(jointLocationClub.p.x + .05f, jointLocationClub.p.y, jointLocationClub.p.z), PxQuat(jointLocationClub.q.x, jointLocationClub.q.y, jointLocationClub.q.z, jointLocationClub.q.w)));
		}

		void MoveUp() const
		{
			((PxRigidBody*)putter->Get())->setGlobalPose(PxTransform(PxVec3(jointLocationClub.p.x, jointLocationClub.p.y + .005f, jointLocationClub.p.z), PxQuat(jointLocationClub.q.x, jointLocationClub.q.y, jointLocationClub.q.z, jointLocationClub.q.w)));
		}

		void MoveDown() const
		{
			((PxRigidBody*)putter->Get())->setGlobalPose(PxTransform(PxVec3(jointLocationClub.p.x, jointLocationClub.p.y - .005f, jointLocationClub.p.z), PxQuat(jointLocationClub.q.x, jointLocationClub.q.y, jointLocationClub.q.z, jointLocationClub.q.w)));
		}

		void RotateLeft() const
		{
			((PxRigidBody*)putter->Get())->setGlobalPose(PxTransform(PxVec3(jointLocationClub.p.x, jointLocationClub.p.y, jointLocationClub.p.z), PxQuat(jointLocationClub.q.y + .45f, PxVec3(0.f, 1.f, .0f))));
		}

		void RotateRight() const
		{
			((PxRigidBody*)putter->Get())->setGlobalPose(PxTransform(PxVec3(jointLocationClub.p.x, jointLocationClub.p.y, jointLocationClub.p.z), PxQuat(jointLocationClub.q.y + -.45f, PxVec3(0.f, 1.f, .0f))));
		}

		void ResetRot()
		{
			((PxRigidBody*)putter->Get())->setGlobalPose(PxTransform(PxVec3(jointLocationClub.p.x, jointLocationClub.p.y, jointLocationClub.p.z), PxQuat(0.f, PxVec3(0.f, 1.f, .0f))));
		}

		//platform controls
		//platform go up
		void MovePlatup()
		{
			((PxRigidBody*)floorJoint->Get())->setGlobalPose(PxTransform(PxVec3(floorJointLocation.p.x, floorJointLocation.p.y + 0.01f, floorJointLocation.p.z))); 
		}	
		//platform goes back down
		void MovePlatDown()
		{
			((PxRigidBody*)floorJoint->Get())->setGlobalPose(PxTransform(PxVec3(floorJointLocation.p.x, floorJointLocation.p.y - 0.01f, floorJointLocation.p.z))); 
		}
	};
}
