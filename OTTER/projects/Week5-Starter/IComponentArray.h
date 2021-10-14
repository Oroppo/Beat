#pragma once
using Entity = uint32_t; 
using Entity = std::uint32_t;
using ComponentType = std::uint8_t;
const Entity MAX_ENTITIES = 5000;
const ComponentType MAX_COMPONENTS = 32;

class IComponentArray
{
public:
	virtual ~IComponentArray();
	virtual void EntityDestroyed(Entity entity)=0;
};