#pragma once

#include "Entity.hpp"
#include <map>

using EntityVec = std::vector<std::shared_ptr<Entity>>;
using EntityMap = std::map<std::string, EntityVec>;

class EntityManager
{
    EntityVec   m_entities;
    EntityVec   m_entitiesToAdd;
    EntityMap   m_entityMap;
    size_t      m_totalEntities = 0;

    void removeDeadEntities(EntityVec& vec)
    {
        // TODO 
        // remove all entities from vec that are not alive
    }

public:

    EntityManager() = default;

    void update()
    {
        // add all the entities that we want to add
        for (auto e : m_entitiesToAdd)
        {
            m_entities.push_back(e);
            m_entityMap[e->tag()].push_back(e);
        }

        m_entitiesToAdd.clear();

        // remove dead entities from the vector of all entities
        removeDeadEntities(m_entities);
        
        // remove dead entities from each vector in the entity map
        // C++20 way of iterating through [key,value] pairs in a map
        for (auto& [tag, entityVec] : m_entityMap)
        {
            removeDeadEntities(entityVec);
        }
    }

    std::shared_ptr<Entity> addEntity(const std::string& tag)
    {
        auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
        m_entitiesToAdd.push_back(entity);
        return entity;
    }

    const EntityVec& getEntities() const
    {
        return m_entities;
    }

    const EntityVec& getEntities(const std::string& tag)
    {
        return m_entityMap[tag];
    }

    const EntityMap& getEntityMap()
    {
        return m_entityMap;
    }
};