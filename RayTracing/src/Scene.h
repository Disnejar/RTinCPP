#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Sun
{
	glm::vec3 Direction{ 1.0f };
};

struct Material
{
	glm::vec3 Albedo{ 0.8f, 0.8f, 0.8f };
	float Roughness = 1.0f;
	float Metallic = 0.0f;
};

struct Sphere
{
	glm::vec3 Position { 0.0f, 0.0f, 0.0f };
	float Radius = 0.5f;

	int MaterialIndex = 0;
};

struct Scene
{
	Sun MainSun;
	std::vector<Sphere> Spheres;
	std::vector<Material> Materials;
};
