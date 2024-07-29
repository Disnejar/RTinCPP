#include "Camera.h"
#include "Walnut/Input/Input.h"


Camera::Camera(float verticalFOV, float nearClip, float farClip)
	: _verticalFOV(verticalFOV), _nearClip(nearClip), _farClip(farClip)
{
	_forwardDirection = glm::vec3(0, 0, -1);
	_position = glm::vec3(0, 0, 3);

	RecalculateView();
	RecalculateRayDirections();
}

bool Camera::OnUpdate(float ts) {
	glm::vec2 mousePos = Walnut::Input::GetMousePosition();
	glm::vec2 mouseDelta = (mousePos - _lastMousePosition) * 0.002f;
	_lastMousePosition = mousePos;

	if (!Walnut::Input::IsMouseButtonDown(Walnut::MouseButton::Right))
	{
		Walnut::Input::SetCursorMode(Walnut::CursorMode::Normal);
		return false;
	}

	Walnut::Input::SetCursorMode(Walnut::CursorMode::Locked);

	bool moved = false;

	glm::vec3 rightDirection = GetRightDirection();

	glm::vec3 desiredMovement = glm::vec3(0.0f);

	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::W))
		desiredMovement += _forwardDirection;
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::S))
		desiredMovement -= _forwardDirection;
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::A))
		desiredMovement -= rightDirection;
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::D))
		desiredMovement += rightDirection;
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::Space))
		desiredMovement += _upDirection;
	if (Walnut::Input::IsKeyDown(Walnut::KeyCode::LeftShift))
		desiredMovement -= _upDirection;

	if (desiredMovement != glm::vec3(0.0f))
	{
		_position += desiredMovement * ts * _movementSpeed;
		moved = true;
	}

	if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f)
	{
		float pitchDelta = mouseDelta.y * _rotationSpeed;
		float yawDelta = mouseDelta.x * _rotationSpeed;

		glm::quat r = glm::normalize(glm::cross(
			glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, _upDirection)));
		_forwardDirection = glm::rotate(r, _forwardDirection);

		moved = true;
	}

	if (moved)
	{
		RecalculateView();
		RecalculateRayDirections();
	}

	return moved;
}

void Camera::OnResize(uint32_t width, uint32_t height){
	if (width == _viewPortWidth && height == _viewPortHeight)
		return;

	_viewPortWidth = width;
	_viewPortHeight = height;

	RecalculateProjection();
	RecalculateRayDirections();
}

float* Camera::GetRotationSpeed(){
	return &_rotationSpeed;
}

float* Camera::GetMovementSpeed(){
	return &_movementSpeed;
}

void Camera::RecalculateProjection()
{
	_projection = glm::perspectiveFov(glm::radians(_verticalFOV), (float)_viewPortWidth, (float)_viewPortHeight, _nearClip, _farClip);
	_inverseProjection = glm::inverse(_projection);
}

void Camera::RecalculateView()
{
	_view = glm::lookAt(_position, _position + _forwardDirection, _upDirection);
	_inverseView = glm::inverse(_view);
}

void Camera::RecalculateRayDirections()
{
	_rayDirections.resize(_viewPortWidth * _viewPortHeight);

	for (uint32_t y = 0; y < _viewPortHeight; y++)
	{
		for (uint32_t x = 0; x < _viewPortWidth; x++)
		{
			glm::vec2 coord = { (float)x / _viewPortWidth, (float)y / _viewPortHeight };
			coord = coord * 2.0f - 1.0f;

			glm::vec4 target = _inverseProjection * glm::vec4(coord.x, coord.y, 1, 1);
			glm::vec3 rayDirection = glm::vec3(_inverseView * glm::vec4(glm::normalize(glm::vec3(target) / target.w), 0)); // World Space
			_rayDirections[x + y * _viewPortWidth] = rayDirection;
		}
	}
}
