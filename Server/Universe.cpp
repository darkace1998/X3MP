#include "Universe.h"

Universe::Universe()
{
    entities = std::make_shared<std::array<std::shared_ptr<x3::net::Entity>, 65535>>();
    stars = std::make_shared<std::array<std::shared_ptr<x3::net::Entity>, 65535>>();
}