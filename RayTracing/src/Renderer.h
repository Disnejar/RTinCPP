#pragma once

#include "Walnut/Image.h"
#include "Walnut/Random.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"

#include <memory>
#include <glm/glm.hpp>

class Renderer
{
public:
	struct Settings
	{
		bool AccumulateFrames = true;
	};

public:

	Renderer() = default;

	void OnResize(uint32_t width, uint32_t height);

	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage();

	Settings& GetSettings() { return _settings; };

	void ResetFrameIndex() { _frameIndex = 1; }
private:
	glm::vec4 PerPixel(uint32_t x, uint32_t y);

	HitInfo TraceRay(const Ray& ray);
	HitInfo ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitInfo Miss(const Ray& ray);

private:
	std::shared_ptr<Walnut::Image> _finalImage;
	Settings _settings;

	const Scene* _activeScene = nullptr;
	const Camera* _activeCamera = nullptr;

	uint32_t* _imageData = nullptr;
	glm::vec4* _accumulationData = nullptr;

	uint32_t _frameIndex = 1;
};

