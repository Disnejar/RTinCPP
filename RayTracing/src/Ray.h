#pragma once

#include <glm/glm.hpp>

struct HitInfo
{
	float HitDistance;
	glm::vec3 WorldPosition;
	glm::vec3 WorldNormal;

	uint32_t ObjectIndex;
};

struct Ray
{
	glm::vec3 Origin;
	glm::vec3 Direction;
};
