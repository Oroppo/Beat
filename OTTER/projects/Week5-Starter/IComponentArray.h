#pragma once
class IComponentArray
{
public:
	virtual ~IComponentArray();
	virtual void EntityDestroyed(Entity entity)=0;
};