#include "X3Util.h"
#include "X3Functions.h"
#include <iostream>

namespace x3
{
    namespace util
    {
        std::vector<x3::Entity*> entitiesToDelete;

        void hook_DeleteEntityLoop(void* param_1)
        {
            for (auto& deletion : entitiesToDelete)
            {
                x3::DeleteEntity(deletion);
                // Entity deletion is handled by X3's built-in deletion system
            }
            entitiesToDelete.clear();
            x3::DeleteEntityLoop(param_1); // Call original game function
        }

        void DeleteEntity(x3::Entity* entity)
        {
            entitiesToDelete.push_back(entity);
        }

        const bool CheckShipPointer(x3::Entity* ptr, std::array<Entity*, MAX_ENTITIES>& entities)
        {
            return std::find(std::begin(entities), std::end(entities), ptr) != std::end(entities);
        }
    }
}