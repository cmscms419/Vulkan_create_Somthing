#include "VKengine.h"
#include "VKkey.h"
#include "Debug.h"
#include "helper.h"

using vkengine::input::key_callback;
using vkengine::VKDevice_;
using vkengine::VKSwapChain;

using namespace vkengine::debug;

namespace vkengine
{
    constexpr bool bUseValidationLayers = false;
    VulkanEngine* loadedEngine = nullptr;

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<VulkanEngine*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    VulkanEngine::~VulkanEngine()
    {
        this->cleanup();
    }

    VulkanEngine& VulkanEngine::Get() { return *loadedEngine; }

    void VulkanEngine::init()
    {
        assert(loadedEngine == nullptr);
        loadedEngine = this;

        this->initWindow();
        this->init_vulkan();

    }

    void VulkanEngine::cleanup()
    {
        if (this->_isInitialized)
        {
            this->VKswapChain->~VKSwapChain();
            
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroyCommandPool(this->VKdevice->VKdevice, this->VKframeData[i].commandPool, nullptr);
            }

            vkDestroyDevice(this->VKdevice->VKdevice, nullptr);

            
            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(this->VKinstance, this->VKdebugUtilsMessenger, nullptr);
            }

            vkDestroySurfaceKHR(this->VKinstance, this->VKsurface, nullptr);
            vkDestroyInstance(this->VKinstance, nullptr);

            glfwDestroyWindow(this->VKwindow);
            glfwTerminate();
        }

        loadedEngine = nullptr;
    }

    bool VulkanEngine::mainLoop()
    {
        while (!glfwWindowShouldClose(this->VKwindow)) {
            glfwPollEvents();
            drawFrame();
#ifdef DEBUG_
            //printf("update\n");
#endif // DEBUG_

        }

        //vkDeviceWaitIdle(this->VKdevice);
        state = false;

        return state;
    }

    void VulkanEngine::drawFrame()
    {
    }

    void VulkanEngine::initWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        int WIDTH_ = WIDTH;
        int HEIGHT_ = HEIGHT;

        this->VKwindow = glfwCreateWindow(WIDTH_, HEIGHT_, "Vulkan Test", nullptr, nullptr);

        glfwSetWindowUserPointer(this->VKwindow, this);
        glfwSetFramebufferSizeCallback(this->VKwindow, framebufferResizeCallback);
        glfwSetKeyCallback(this->VKwindow, vkengine::input::key_callback);  // 키 입력 콜백 설정

        this->_isInitialized = true;
    }

    void VulkanEngine::init_vulkan()
    {
        if (glfwVulkanSupported() == GLFW_FALSE) {
            throw std::runtime_error("Vulkan is not supported");
        }

        this->createInstance();
        
        if (enableValidationLayers)
        {
            this->setupDebugCallback();
        }

        this->createSurface();
        this->createDevice();
        this->init_swapchain();
        this->init_commands();
        this->init_sync_structures();
    }

    void VulkanEngine::init_swapchain()
    {
        this->VKswapChain = new VKSwapChain(this->VKdevice->VKphysicalDevice, this->VKdevice->VKdevice, this->VKsurface, &this->VKinstance);
        this->VKswapChain->createSwapChain(&this->VKdevice->queueFamilyIndices);
    }

    void VulkanEngine::init_commands()
    {
        // command pool create and command buffer create
        QueueFamilyIndices queueFamilyIndices = this->VKdevice->queueFamilyIndices;
        
        // 커맨드 풀 생성 정보 구조체를 초기화합니다.
        VkCommandPoolCreateInfo poolInfo = helper::commandPoolCreateInfo(queueFamilyIndices.graphicsAndComputeFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        for (auto& framedata : this->VKframeData)
        {
            VkResult result = vkCreateCommandPool(this->VKdevice->VKdevice, &poolInfo, nullptr, &framedata.commandPool);
            
            if (result != VK_SUCCESS) {
                return;
            }

            VkCommandBufferAllocateInfo allocInfo = helper::commandBufferAllocateInfo(framedata.commandPool, 1, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

            result = vkAllocateCommandBuffers(this->VKdevice->VKdevice, &allocInfo, &framedata.mainCommandBuffer);
            
            if (result != VK_SUCCESS) {
                return;
            }

        }
    }

    bool VulkanEngine::init_sync_structures()
    {
        VkSemaphoreCreateInfo semaphoreInfo = helper::semaphoreCreateInfo(0);
        VkFenceCreateInfo fenceInfo = helper::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);

        for (auto frameData : this->VKframeData)
        {
            VkResult result0 = vkCreateSemaphore(this->VKdevice->VKdevice, &semaphoreInfo, nullptr, &frameData.VkimageavailableSemaphore);
            VkResult result1 = vkCreateSemaphore(this->VKdevice->VKdevice, &semaphoreInfo, nullptr, &frameData.VkrenderFinishedSemaphore);
            VkResult result2 = vkCreateFence(this->VKdevice->VKdevice, &fenceInfo, nullptr, &frameData.VkinFlightFences);

            if (result0 != VK_SUCCESS || result1 != VK_SUCCESS || result2 != VK_SUCCESS) {
                return false;
            }
        }
    }

    void VulkanEngine::createInstance()
    {
        if (enableValidationLayers && !checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        // VkApplicationInfo 구조체는 애플리케이션에 대한 정보를 Vulkan에게 제공하기 위해 사용됩니다.
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;     // 구조체 타입을 지정합니다.
        appInfo.pApplicationName = "Vulkan egine";              // 애플리케이션 이름을 지정합니다.
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);  // 애플리케이션 버전을 지정합니다.
        appInfo.pEngineName = "vulkanEngine";                   // 엔진 이름을 지정합니다.
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);       // 엔진 버전을 지정합니다.
        appInfo.apiVersion = VK_API_VERSION_1_0;                // 사용할 Vulkan API 버전을 지정합니다.

        // VkInstanceCreateInfo 구조체는 Vulkan 인스턴스를 생성하기 위한 정보를 제공합니다.
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; // 구조체 타입을 지정합니다.
        createInfo.pApplicationInfo = &appInfo;                    // VkApplicationInfo 구조체를 참조합니다.

        uint32_t glfwExtensionCount = 0;
        std::vector<const char*> extensions = this->getRequiredExtensions();     // 확장 목록을 저장할 변수를 선언합니다.
        glfwExtensionCount = static_cast<uint32_t>(extensions.size());           // 확장 개수를 저장합니다.
        createInfo.enabledExtensionCount = glfwExtensionCount;                   // 활성화할 확장 개수를 지정합니다.
        createInfo.ppEnabledExtensionNames = extensions.data();                  // 활성화할 확장 목록을 지정합니다.

        if (enableValidationLayers) {

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // Vulkan 인스턴스를 생성합니다.
        if (vkCreateInstance(&createInfo, nullptr, &this->VKinstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!"); // 인스턴스 생성 실패 시 예외를 발생시킵니다.
        }
        else
        {
            printf("create instance\n");
        }
    }

    void VulkanEngine::setupDebugCallback()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(this->VKinstance, &createInfo, nullptr, &this->VKdebugUtilsMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void VulkanEngine::createSurface()
    {
#if CREATESURFACE_VKWIN32SURFACECREATEINFOKHR == 0
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hwnd = glfwGetWin32Window(this->VKwindow);
        createInfo.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(this->VKinstance, &createInfo, nullptr, &this->VKsurface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
#else
        if (glfwCreateWindowSurface(this->VKinstance, this->VKwindow, nullptr, &this->VKsurface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
#endif
        //this->VKswapChain = new vkengine::VKSwapChain(this->VKdevice->VKphysicalDevice, this->VKdevice->VKdevice, this->VKsurface, &this->VKinstance);
    }

    void VulkanEngine::createDevice()
    {
        // 물리 디바이스 목록을 가져옵니다.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(this->VKinstance, &deviceCount, devices.data());

        // 물리 디바이스를 선택한다.
        // 먼저 물리 디바이스를 선택하기 위한 함수를 정의한다.
        // Score를 기준으로 가장 높은 점수의 물리 디바이스를 선택한다.
        std::multimap<int, VkPhysicalDevice> candidates;
        std::vector<QueueFamilyIndices> indices(deviceCount);

        uint16_t selectQueueFamilyIndeices = 0;
        for (const auto& device : devices) {
            if (helper::isDeviceSuitable(device, this->VKsurface, &indices[selectQueueFamilyIndeices]))
            {
                int score = helper::rateDeviceSuitability(device);
                candidates.insert(std::make_pair(score, device));
                selectQueueFamilyIndeices++;
            }
        }

        int Score = 0;
        VkPhysicalDevice pdevice = VK_NULL_HANDLE;
        
        int i = 0;
        for (const auto& candidate : candidates) {

            if (candidate.first > Score) {
                Score = candidate.first;
                pdevice = candidate.second;
                selectQueueFamilyIndeices = i;
            }
            i++;
        }

        //this->VKmsaaSamples = helper::getMaxUsableSampleCount(this->VKphysicalDevice);

        if (Score == 0)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

        this->VKdevice = new vkengine::VKDevice_(pdevice, indices[selectQueueFamilyIndeices]);

        // 파생된 예제는 물리적 장치에서 읽은 지원되는 확장 목록에 따라 확장 기능을 활성화할 수 있습니다.
        // 필요할 때 코드 생성

        // 물리 장치 기능 구조체를 초기화합니다.
        this->VKdevice->features.samplerAnisotropy = VK_TRUE; // 샘플러를 사용하여 텍스처를 보간합니다.
        this->VKdevice->features.sampleRateShading = VK_TRUE; // 샘플 레이트 쉐이딩을 사용하여 픽셀을 그립니다.

        // 논리 디바이스를 생성합니다.
        VkResult result = this->VKdevice->createLogicalDevice();

        // depth format을 가져옵니다.
        this->VKdepthStencill.depthFormat = helper::findDepthFormat(this->VKdevice->VKphysicalDevice);
    }

    void VulkanEngine::createDepthStencilResources()
    {
        // 깊이 이미지 생성 정보 구조체를 초기화합니다.
        this->VKdevice->createimageview(
            this->VKswapChain->getSwapChainExtent().width,
            this->VKswapChain->getSwapChainExtent().height,
            1,
            this->VKmsaaSamples,
            this->VKdepthStencill.depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            this->VKdepthStencill.depthImage,
            this->VKdepthStencill.depthImageMemory
        );

        // 깊이 이미지 뷰를 생성합니다.
        this->VKdepthStencill.depthImageView = helper::createImageView(
            this->VKdevice->VKdevice,
            this->VKdepthStencill.depthImage,
            this->VKdepthStencill.depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            1
        );

        // 깊이 이미지 레이아웃을 설정합니다.
        helper::transitionImageLayout(
            this->VKdevice->VKdevice,
            this->VKdevice->VKcommandPool,
            this->VKdevice->graphicsVKQueue,
            this->VKdepthStencill.depthImage,
            this->VKdepthStencill.depthFormat,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            1
        );
    }

    void VulkanEngine::cleanupSwapChain()
    {
        //for (auto swapChainView : this->VKswapChainImageViews)
        //{
        //    vkDestroyImageView(this->VKdevice->VKdevice, swapChainView, nullptr);
        //}
        //
        //vkDestroySwapchainKHR(this->VKdevice->VKdevice, this->VKswapChain, nullptr);
    }

    bool VulkanEngine::checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> VulkanEngine::getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

}


