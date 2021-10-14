#pragma once
#include <iostream>
#include <bitset>
#include <queue>
#include <array>
#include <cassert>

using Entity = std::uint32_t;
using ComponentType = std::uint8_t;
const Entity MAX_ENTITIES = 5000;
const ComponentType MAX_COMPONENTS = 32;
using Signature = std::bitset<MAX_COMPONENTS>;

class EntityManager
{
public:
	EntityManager()
	{
		//initializes the queue with all possible entity ID's
		for (Entity entity = 0; entity < MAX_ENTITIES, ++entity;)
		{
			mAvailableEntities.push(entity);
		}
	}

	Entity CreateEntity()
	{
		assert(mLivingEntityCount < MAX_ENTITIES && "There are too many entities in Existence.");

		//Takes an ID from the front of the Queue then pops it to the back of the queue
		Entity id = mAvailableEntities.front();
		mAvailableEntities.pop();
		++mLivingEntityCount;

		return id;
	}
	void DestroyEntity(Entity entity)
	{
		assert(entity < MAX_ENTITIES && "That Entity is out of range and doesn't exist.");

		//Invalidates the signature of the destroyed entity
		mSignatures[entity].reset();

		//put the destroyed ID at the back of the queue
		mAvailableEntities.push(entity);
		--mLivingEntityCount;
	}
	void SetSignature(Entity entity, Signature signature)
	{
		assert(entity < MAX_ENTITIES && "Entity out of range.");

		// Put this entity's signature into the array
		mSignatures[entity] = signature;
	}

	Signature GetSignature(Entity entity)
	{
		assert(entity < MAX_ENTITIES && "Entity out of range.");

		// Get this entity's signature from the array
		return mSignatures[entity];
	}
private:
	// Queue of unused entity IDs
	std::queue<Entity> mAvailableEntities{};

	// Array of signatures where the index corresponds to the entity ID
	std::array<Signature, MAX_ENTITIES> mSignatures{};

	// Total living entities - used to keep limits on how many exist
	uint32_t mLivingEntityCount{};
};

