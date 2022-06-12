#pragma once 
#include "net_entity.h"
#include <memory>
#include <array>

class Universe
{
    public:
	std::shared_ptr<std::array<std::shared_ptr<x3::net::Entity>, 65535>> entities;
	std::shared_ptr<std::array<std::shared_ptr<x3::net::Entity>, 65535>> stars;

    Universe();
};