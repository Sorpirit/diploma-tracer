#include "CamerUIControl.hpp"

#include <imgui.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "../Window.hpp"

namespace TracerCore {
namespace UI
{

    CamerUIControl::CamerUIControl(TracerCamera &camera, Window& window) : _camera(camera), _window(window)
    {
        auto mousePos = ImGui::GetMousePos();
        glm::vec2 mousePosGlm = glm::vec2(mousePos.x, mousePos.y);
        _lastMousePos = mousePosGlm;
    }

    CamerUIControl::~CamerUIControl()
    {
        printf("Boom");
    }

    void CamerUIControl::Render()
    {
        auto mousePos = ImGui::GetMousePos();
        glm::vec2 mousePosGlm = glm::vec2(mousePos.x, mousePos.y);
        glm::vec2 delta = (mousePosGlm - _lastMousePos) * 0.002f;
        _lastMousePos = mousePosGlm;

        bool allowMoving = ImGui::IsMouseDown(ImGuiMouseButton_Right);

        if(_allowMoving != allowMoving)
        {
            _allowMoving = allowMoving;
            if(allowMoving)
            {
                _window.LockCursor();
                _camera.SetStatic(false);
            }
            else
            {
                _window.UnlockCursor();
                _camera.SetStatic(true);
            }
        }

        // if(allowMoving)
        // {
        //     _window.LockCursor();
        //     _camera.SetStatic(false);
        // }
        // else
        // {
        //     _window.UnlockCursor();
        //     _camera.SetStatic(true);
        // }

        if (!ImGui::IsItemFocused() && allowMoving) 
        {
            _window.LockCursor();
            float speed = 0.5f;
            float rotationSpeed = 2.0f;
            glm::vec3 move = glm::vec3(0.0f);
            glm::vec3 newForward = _camera.GetForward();
            bool changed = false;

            glm::vec3 right = glm::cross(_camera.GetForward(), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

            if (ImGui::IsKeyDown(ImGuiKey_W) || ImGui::IsKeyDown(ImGuiKey_UpArrow))
            {
                move += _camera.GetForward();
                changed = true;
            }

            if (ImGui::IsKeyDown(ImGuiKey_S) || ImGui::IsKeyDown(ImGuiKey_DownArrow))
            {
                move += -_camera.GetForward();
                changed = true;
            }

            if (ImGui::IsKeyDown(ImGuiKey_A) || ImGui::IsKeyDown(ImGuiKey_LeftArrow))
            {
                move += -right;
                changed = true;
            }

            if (ImGui::IsKeyDown(ImGuiKey_D) || ImGui::IsKeyDown(ImGuiKey_RightArrow))
            {
                move += right;
                changed = true;
            }

            if (ImGui::IsKeyDown(ImGuiKey_D) || ImGui::IsKeyDown(ImGuiKey_RightArrow))
            {
                move += right;
                changed = true;
            }

            if (ImGui::IsKeyDown(ImGuiKey_Space))
            {
                move += up;
                
                changed = true;
            }

            if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
            {
                move -= up;
                changed = true;
            }


            if (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))
            {
                speed *= 3.0f;
            }

            if (delta.x != 0.0f || delta.y != 0.0f)
            {
                float pitchDelta = delta.y * rotationSpeed;
                float yawDelta = delta.x * rotationSpeed;

                glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, right),
                    glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
                
                newForward = glm::rotate(q, newForward);
                changed = true;
            }

            if(changed)
            {
                glm::vec3 newPosition =_camera.GetPosition();
                if(glm::dot(move, move) > 0.0f)
                    newPosition += glm::normalize(move) * (speed * ImGui::GetIO().DeltaTime);

                _camera.SetParameters(newPosition, newForward);
            }
        }
    }
}
}