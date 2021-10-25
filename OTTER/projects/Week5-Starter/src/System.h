#pragma once
#include <iostream>
#include <set>
using Entity = uint32_t;

class System
{
public:
	std::set<Entity> mEntities;
};

