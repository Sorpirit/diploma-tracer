#include "TracerCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace TracerCore
{
    TracerCamera::TracerCamera()
    {
        SetParameters(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
        SetProjection(90.0f, 1.0f, 0.1f, 150.0f);
    }

    TracerCamera::~TracerCamera()
    {
    }

    void TracerCamera::SetParameters(const glm::vec3 &position, const glm::vec3 &forward)
    {
        _position = position;
        _forward = forward;
        UpdateView();
    }

    void TracerCamera::SetProjection(float fov, float aspect, float near, float far)
    {
        _fov = fov;
        _aspect = aspect;
        _near = near;
        _far = far;
        UpdateProjection();
    }

    void TracerCamera::UpdateProjection()
    {
        _projection = glm::perspective(_fov, _aspect, _near, _far);
        _invProjection = glm::inverse(_projection);
    }

    void TracerCamera::UpdateView()
    {
        _view = glm::lookAt(_position, _position + _forward, glm::vec3(0.0f, 1.0f, 0.0f));
        _invView = glm::inverse(_view);
    }

} // namespace TracerCore
