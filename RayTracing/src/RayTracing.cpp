#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class MainLayer : public Walnut::Layer
{
public:
	MainLayer()
		: _camera(60.0f, 0.1f, 100.0f)
	{
		_scene.MainSun = Sun{ {1.0f, 1.0f, 1.0f} };
		_scene.Materials.push_back(Material());
	}

	virtual void OnUpdate(float ts) override
	{
		if (_camera.OnUpdate(ts))
		{
			_renderer.ResetFrameIndex();
		}
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		
		ImGui::Text("Last render: %.3fms", _lastRenderElapsedMS);

		if (ImGui::Button("Render")) 
		{
			Render();
		}

		ImGui::Checkbox("Accumulate", &_renderer.GetSettings().AccumulateFrames);

		if (ImGui::Button("Reset"))
		{
			_renderer.ResetFrameIndex();
		}

		ImGui::BeginVertical("Camera");

		ImGui::Text("Camera Controls");
		ImGui::DragFloat("Movement Speed", _camera.GetMovementSpeed(), 0.2f, 0.5f);
		ImGui::DragFloat("Rotation Speed", _camera.GetRotationSpeed(), 0.05f, 0.1f);

		ImGui::EndVertical();
		
		ImGui::End();

		ImGui::Begin("Scene");

		ImGui::Text("Sphere Controls");
		
		if (ImGui::Button("Add Sphere"))
		{
			Sphere sphere;
			sphere.Position = { 1.0f, 0.0f, 0.0f };
			sphere.Radius = 1;
			sphere.MaterialIndex = 0;

			_scene.Spheres.push_back(sphere);
		}

		if(_scene.Spheres.size() > 0)
			ImGui::SliderInt("Sphere Index", &_sphereIndex, 0, _scene.Spheres.size() - 1);
		
		ImGui::Separator();

		if (_scene.Spheres.size() > _sphereIndex)
		{
			Sphere* sphere = &_scene.Spheres[_sphereIndex];

			ImGui::DragFloat3("Position", glm::value_ptr(sphere->Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere->Radius, 0.1f);
			ImGui::SliderInt("Material Index", &sphere->MaterialIndex, 0, _scene.Materials.size() - 1);
		}

		ImGui::Text("Material Controls");
		
		if (ImGui::Button("Add Material"))
		{
			Material material;
			material.Albedo = { 1.0f, 0.0f, 1.0f };
			material.Roughness = 1.0f;
			material.Metallic = 0.0f;

			_scene.Materials.push_back(material);
		}

		if(_scene.Materials.size() > 0)
			ImGui::SliderInt("Material Index", &_materialIndex, 0, _scene.Materials.size() - 1);
		
		ImGui::Separator();

		if (_scene.Materials.size() > _materialIndex)
		{
			Material* material = &_scene.Materials[_materialIndex];
			ImGui::ColorEdit3("Albedo", glm::value_ptr(material->Albedo));
			ImGui::SliderFloat("Roughness", &material->Roughness, 0.0f, 1.0f);
			ImGui::SliderFloat("Metallic", &material->Metallic, 0.0f, 1.0f);
		}

		ImGui::End();

		ImGui::Begin("Viewport");

		_viewportWidth = ImGui::GetContentRegionAvail().x;
		_viewportHeight = ImGui::GetContentRegionAvail().y;
		
		auto image = _renderer.GetFinalImage();
		
		if (image)
			ImGui::Image(image->GetDescriptorSet(), {(float)image->GetWidth(), (float)image->GetHeight()}, ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();

		Render();
	}

	void Render() {
		Timer timer;

		_renderer.OnResize(_viewportWidth, _viewportHeight);
		_camera.OnResize(_viewportWidth, _viewportHeight);
		_renderer.Render(_scene, _camera);

		_lastRenderElapsedMS = timer.ElapsedMillis();
	}

private:
	Renderer _renderer;
	Scene _scene;
	Camera _camera;
	uint32_t _viewportWidth = 0, _viewportHeight = 0;

	float _lastRenderElapsedMS = 0.0f;

	int _sphereIndex = 0;
	int _materialIndex = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing Studio";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<MainLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::MenuItem("Exit"))
		{
			app->Close();
		}
	});
	return app;
}