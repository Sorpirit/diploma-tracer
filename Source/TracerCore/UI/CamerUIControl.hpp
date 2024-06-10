#pragma once

#include "UILayer.hpp"

#include "../TracerCamera.hpp"

namespace TracerCore {
namespace UI
{

  class CamerUIControl : public RenderUILayer
  {
    public:
        CamerUIControl(TracerCamera& camera, Window& window);
        ~CamerUIControl();

        void Render() override;
    private:
        TracerCamera& _camera;
        Window& _window;

        glm::vec2 _lastMousePos;
        bool _allowMoving = false;
  };
  

}}
