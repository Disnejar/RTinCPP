#include "Renderer.h"

namespace Utils
{
	static uint32_t Vec4ToABGR(const glm::vec4& color)
	{
		return
			(uint8_t)(0xff * color.a) << 24 |
			(uint8_t)(0xff * color.b) << 16 |
			(uint8_t)(0xff * color.g) << 8 |
			(uint8_t)(0xff * color.r);
	}

	
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (!_finalImage)
	{
		_finalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}
	else
	{
		if (_finalImage->GetWidth() == width && _finalImage->GetHeight() == height)
			return;

		_finalImage->Resize(width, height);
	}

	delete[] _imageData;
	_imageData = new uint32_t[width * height];

	delete[] _accumulationData;
	_accumulationData = new glm::vec4[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	_activeScene = &scene;
	_activeCamera = &camera;

	if (_frameIndex == 1)
		memset(_accumulationData, 0, _finalImage->GetHeight() * _finalImage->GetWidth() * sizeof(glm::vec4));

	for (uint32_t y = 0; y < _finalImage->GetHeight(); y++)
	{
		for (uint32_t x = 0; x < _finalImage->GetWidth(); x++)
		{
			glm::vec4 color = PerPixel(x, y);

			_accumulationData[x + y * _finalImage->GetWidth()] += color;

			glm::vec4 accumulatedColor = _accumulationData[x + y * _finalImage->GetWidth()] / (float)_frameIndex;

			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			_imageData[x + y * _finalImage->GetWidth()] = Utils::Vec4ToABGR(accumulatedColor);
		}
	}

	if (_settings.AccumulateFrames)
		_frameIndex++;
	else
		_frameIndex = 1;

	_finalImage->SetData(_imageData);
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	glm::vec3 color (0.0f);

	Ray ray;
	ray.Origin = _activeCamera->GetPosition();
	ray.Direction = _activeCamera->GetRayDirections()[x + y * _finalImage->GetWidth()];

	glm::vec3 lightDir = glm::normalize(_activeScene->MainSun.Direction);

	float multiplier = 1.0f;

	int bounces = 5;
	for (int i = 0; i < bounces; i++)
	{
		HitInfo hitInfo = TraceRay(ray);

		if (hitInfo.HitDistance < 0)
		{
			glm::vec3 skyColor = glm::vec3{ 0.6f, 0.7f, 0.9f };
			color += skyColor * multiplier;
			break; // Set Sky Color Here
		}
		float lightIntesity = glm::max(0.0f, glm::dot(lightDir, hitInfo.WorldNormal));

		const Sphere& sphere = _activeScene->Spheres[hitInfo.ObjectIndex];
		const Material& material = _activeScene->Materials[sphere.MaterialIndex];

		color += material.Albedo * lightIntesity * multiplier;

		multiplier *= 0.7f;
		ray.Origin = hitInfo.WorldPosition + hitInfo.WorldNormal * 0.0001f;
		ray.Direction = glm::reflect(ray.Direction, 
			hitInfo.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));
	}

	return glm::vec4(color, 1.0f);

}

HitInfo Renderer::TraceRay(const Ray& ray)
{
	int closestSphere = -1;
	float closestHit = FLT_MAX;

	for (size_t i = 0; i < _activeScene->Spheres.size(); i++)
	{
		const Sphere& sphere = _activeScene->Spheres[i];

		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		float discriminant = b * b - 4.0f * a * c;

		if (discriminant > 0.0f)
		{
			float hitDist = (-b - glm::sqrt(discriminant)) / 2.0f * a;

			if (hitDist < closestHit && hitDist > 0)
			{
				closestHit = hitDist;
				closestSphere = i;
			}
		}

	}

	if (closestSphere == -1)
	{
		return Miss(ray);
	}

	return ClosestHit(ray, closestHit, closestSphere);
}

HitInfo Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	HitInfo hitInfo;
	hitInfo.HitDistance = hitDistance;
	hitInfo.ObjectIndex = objectIndex;

	const Sphere* closestSphere = &_activeScene->Spheres[objectIndex];

	glm::vec3 origin = ray.Origin - closestSphere->Position;

	hitInfo.WorldPosition = origin + ray.Direction * hitDistance;
	hitInfo.WorldNormal = glm::normalize(hitInfo.WorldPosition);

	hitInfo.WorldPosition += closestSphere->Position;

	return hitInfo;
}

HitInfo Renderer::Miss(const Ray& ray)
{
	HitInfo hitInfo;
	hitInfo.HitDistance = -1;
	return hitInfo;
}

std::shared_ptr<Walnut::Image> Renderer::GetFinalImage()
{
	return _finalImage;
}
