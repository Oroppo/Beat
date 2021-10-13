#pragma once
#include <iostream>
#include <cassert>
#include <unordered_map>
#include "IComponentArray.h"

// This class makes use of Unordered Maps which comes with a performance penalty, if we're having issues with 
// Framerates, we need to replace them with Arrays, although it'll be more cumbersome to code that way.

// If virtual Functions prove to be cumbersome to use, we'll have to use an Event System to call them whenever EntityDestroyed() is called
// instead. This will make implementation easier but is more obfuscated and hard to read. Note virtual functions also come with a performance hit

using Entity = std::uint32_t;

template<typename T>

class ComponentArray : public IComponentArray
{
public:
	void InsertData(Entity entity, T component);

	void RemoveData(Entity entity);

	T& GetData(Entity entity);

	void EntityDestroyed(Entity entity) override;

private:
	// The packed array of components (of generic type T),
	// set to a specified maximum amount, matching the maximum number
	// of entities allowed to exist simultaneously, so that each entity
	// has a unique spot.
	std::array<T, MAX_ENTITIES> mComponentArray;

	// Map from an entity ID to an array index.
	std::unordered_map<Entity, size_t> mEntityToIndexMap;

	// Map from an array index to an entity ID.
	std::unordered_map<size_t, Entity> mIndexToEntityMap;

	// Total size of valid entries in the array.
	size_t mSize;
};