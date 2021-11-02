#pragma once
#include "bullet/btBulletDynamicsCommon.h"
#include <memory>

class Physics {

private:
	btDefaultCollisionConfiguration* collisionConfiguration;
	btCollisionDispatcher* dispatcher;
	btBroadphaseInterface* overlappingPairCache;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

public:

	typedef std::shared_ptr<Physics> Sptr;

	static inline Sptr Create() {
		return std::make_shared<Physics>();
	}

	void Init() {
		collisionConfiguration = new btDefaultCollisionConfiguration;
		dispatcher = new btCollisionDispatcher(collisionConfiguration);
		overlappingPairCache = new btDbvtBroadphase();
		solver = new btSequentialImpulseConstraintSolver;
		dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

	}

	void cleanup() {
		// remove the rigidbodies from the dynamics world and delete them
		for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i --)
		{
		btCollisionObject * obj = dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody * body = btRigidBody::upcast(obj);
		if (body && body -> getMotionState())
		 {
		 delete body -> getMotionState();
		 }
		dynamicsWorld -> removeCollisionObject(obj);
		delete obj;
		}
		
		//// delete collision shapes
		//for (int j = 0; j < collisionShapes.size(); j++)
		//{
		//btCollisionShape * shape = collisionShapes[j];
		//collisionShapes[j] = 0;
		//delete shape;
		//}
		
		// delete dynamics world
		delete dynamicsWorld;
		
		// delete solver
		delete solver;
		
		// delete broadphase
		delete overlappingPairCache;
		
		// delete dispatcher
		delete dispatcher;

    }

	void timeStep() {
		for (int i = 0; i < 100; i++) {
			dynamicsWorld->stepSimulation(1.f / 60.f, 10);
			for (int j = dynamicsWorld->getNumCollisionObjects() - 1; j >= 0; j--) {
				btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
				btRigidBody* body = btRigidBody::upcast(obj);
				btTransform trans;
				if (body && body->getMotionState())
				{
					body->getMotionState()->getWorldTransform(trans);
				}
				else
				{
					trans = obj->getWorldTransform();
				}
				printf(" world pos object %d = %f ,%f ,%f\n", j, float(trans.getOrigin().getX()),
					float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
			}
		}
	}

};