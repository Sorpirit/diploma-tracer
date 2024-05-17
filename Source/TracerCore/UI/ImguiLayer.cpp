#include "ImguiLayer.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <stdexcept>

namespace TracerCore {
namespace UI
{
    ImguiLayer::ImguiLayer(VulkanDevice& device)
        : UILayer(device), _showDemoWindow(true)
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
        //io.ConfigViewportsNoAutoMerge = true;
        //io.ConfigViewportsNoTaskBarIcon = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
        ImGuiStyle& style = ImGui::GetStyle();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }
    }

    ImguiLayer::~ImguiLayer()
    {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        vkDestroyDescriptorPool(_device.GetVkDevice(), _descriptorPool, nullptr);
        //TODO:
        //ImGui::DestroyContext();
    }

    void ImguiLayer::Init(Window* window, SwapChain* swapchain)
    {
        auto queueFamilyIndices = _device.FindPhysicalQueueFamilies();

        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1;
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        if(vkCreateDescriptorPool(_device.GetVkDevice(), &pool_info, nullptr, &_descriptorPool) != VK_SUCCESS){
            throw std::runtime_error("Failed to create descriptor pool");
        }

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(window->GetWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = _instance;
        init_info.PhysicalDevice = _physicalDevice;
        init_info.Device = _device.GetVkDevice();
        init_info.QueueFamily = queueFamilyIndices.GraphicsFamily;
        init_info.Queue = _device.GetGraphicsQueue();
        //init_info.PipelineCache = g_PipelineCache;
        init_info.DescriptorPool = _descriptorPool;
        init_info.RenderPass = swapchain->GetGraphicsRenderPass();
        init_info.Subpass = 0;
        init_info.MinImageCount = swapchain->GetImageCount();
        init_info.ImageCount = swapchain->GetImageCount();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        // init_info.Allocator = g_Allocator;
        // init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);
    }

    void ImguiLayer::Render(VkCommandBuffer comandBuffer)
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //UI pass
        //ImGui::ShowExampleAppDockSpace(&_showDemoWindow);
        ImGui::ShowDemoWindow(&_showDemoWindow);
        //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        // Rendering
        ImGui::Render();
        ImDrawData* main_draw_data = ImGui::GetDrawData();
        ImGui_ImplVulkan_RenderDrawData(main_draw_data, comandBuffer);

        // Update and Render additional Platform Windows
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}}