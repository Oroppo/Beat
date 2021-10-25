#pragma once
using Entity = uint32_t; 
using Entity = std::uint32_t;
using ComponentType = std::uint8_t;


class IComponentArray
{
public:
	virtual ~IComponentArray();
	virtual void EntityDestroyed(Entity entity)=0;
};