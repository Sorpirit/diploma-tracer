#pragma once

#include <glm/glm.hpp>

namespace TracerCore
{
    struct TracerCameraData
    {
        glm::mat4x4 _projection;
        glm::mat4x4 _view;
        glm::mat4x4 _invProjection;
        glm::mat4x4 _invView;  
    };

    class TracerCamera
    {
    public:
        TracerCamera();
        ~TracerCamera();

        TracerCamera(const TracerCamera&) = delete;
        TracerCamera &operator=(const TracerCamera&) = delete;

        inline const glm::vec3& GetPosition() const { return _position; }
        inline const glm::vec3& GetForward() const { return _forward; }
        inline const glm::mat4x4& GetProjection() const { return _projection; }
        inline const glm::mat4x4& GetView() const { return _view; }
        inline const glm::mat4x4& GetInvProjection() const { return _invProjection; }
        inline const glm::mat4x4& GetInvView() const { return _invView; }
        inline bool IsStatic() const {return _staticFlag; }
        inline void SetStatic(bool isStatic) { _staticFlag = isStatic; }

        void SetParameters(const glm::vec3& position, const glm::vec3& forward);
        void SetProjection(float fov, float aspect, float near, float far);
    private:
        glm::vec3 _position;
        glm::vec3 _forward;
        float _fov;
        float _aspect;
        float _near;
        float _far;

        glm::mat4x4 _projection;
        glm::mat4x4 _view;
        glm::mat4x4 _invProjection;
        glm::mat4x4 _invView;   

        bool _staticFlag = false; 

        void UpdateProjection();
        void UpdateView();     
    };
}