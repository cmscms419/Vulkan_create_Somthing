﻿#ifndef INCLUDE_SOURCE_APPLICATION_H
#define INCLUDE_SOURCE_APPLICATION_H

#include "cms491_common.h"
#include "DebugFunction.h"

namespace vkutil {

    class Application {
    public:
        Application(std::string root_path);
        ~Application();

        virtual void init();
        virtual void update();
        virtual bool mainLoop();
        virtual void setup();
        virtual void cleanup();

        bool framebufferResized = false;                    // 프레임 버퍼 크기 조정 여부
    private:

        // GLFW 윈도우 생성
        // 상속
        virtual void initWindow();
        virtual void initVulkan();
        virtual void drawFrame();
        virtual bool run(); // 임시
        
        // 기본
        // Vulkan 인스턴스 생성
        void createInstance();
        void setupDebugCallback();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createSurface();
        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createGraphicsPipeline();
        void createFramebuffers();
        void createCommandPool();
        void createCommandBuffers();
        void createSyncObjects();
        void cleanupSwapChain();
        void recreateSwapChain();
        void createVertexBuffer();


        // 도구

        // 주어진 물리 장치에서 큐 패밀리 속성을 찾는 함수
        // PROB : 큐 패밀리가 여러개인 경우에 필요한 처리가 있는 패밀리를 먼저 찾을 경우, 그 패밀리의 인덱스만 반환함
        // TODO ; 큐 패밀리가 여러개인 경우에 대한 처리가 필요함
        const QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        
        bool isDeviceSuitable(VkPhysicalDevice device);
        
        // 물리 디바이스의 확장 기능을 지원하는지 확인하는 함수
        // 검증 레이어 지원 여부를 확인하는 함수
        // 확장 기능을 열거하고 필요한 모든 확장 기능이 포함되어 있는지 확인
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        // 스왑 체인 지원 정보를 가져오는 함수
        // 스왑 체인 지원 정보를 저장할 구조체를 초기화
        // 물리 장치에서 서피스의 기능을 가져옴
        const SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        
        // 서피스 포맷을 선택하는 함수
        // SRGB 색상 공간을 지원하는 32비트 BGR 색상 구조를 선택
        // 이러한 형식이 지원되지 않으면 첫 번째 형식을 반환
        // 이러한 형식이 없으면 예외를 발생시킵니다.
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        // 프레젠테이션 모드를 선택하는 함수
        // VK_PRESENT_MODE_MAILBOX_KHR 프레젠테이션 모드를 지원하는지 확인
        // 지원되는 경우 VK_PRESENT_MODE_MAILBOX_KHR를 반환
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        
        // 스왑 체인 이미지의 해상도를 선택합니다.
        // 현재 해상도가 유효하면 반환하고, 그렇지 않으면 GLFW 크기를 기준으로 해상도를 설정합니다.
        // 해상도는 최소 및 최대 이미지 해상도 사이에서 클램프됩니다.
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
        // 코드를 읽어서 셰이더 모듈을 생성하는 함수
        const VkShaderModule createShaderModule(const std::vector<char>& code);

        // 커맨드 버퍼를 기록하는 함수
        // 실행하고자 하는 명령을 명령 버퍼에 기록
        void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
        
        GLFWwindow* VKwindow;                               // GLFW 윈도우 핸들 -> GLFW 윈도우 핸들을 저장
        VkInstance VKinstance;                              // Vulkan 인스턴스 -> Vulkan API를 사용하기 위한 인스턴스
        VkDebugUtilsMessengerEXT VKdebugUtilsMessenger;     // 디버그 메신저 -> 디버깅을 위한 메신저
        VkPhysicalDevice VKphysicalDevice;                  // 물리 디바이스 -> GPU Physical Handle
        QueueFamilyIndices VKqueueFamilyIndices;            // 큐 패밀리 인덱스 -> VKphysicalDevice에서 선택한 queue family index
        VkDevice VKdevice;                                  // 논리 디바이스 -> GPU Logical Handle
        VkQueue graphicsVKQueue;                            // 그래픽스 큐 -> 그래픽스 명령을 처리하는 큐
        VkQueue presentVKQueue;                             // 프레젠트 큐 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스
        VkSurfaceKHR VKsurface;                             // 서피스 -> 윈도우 시스템과 Vulkan을 연결하는 인터페이스
        VkSwapchainKHR VKswapChain;                         // 스왑 체인 -> 이미지를 프레임 버퍼로 전송하는 데 사용
        std::vector<VkImage> VKswapChainImages;             // 스왑 체인 이미지 -> 스왑 체인에 사용되는 이미지 Handle 배열
        std::vector<VkImageView> VKswapChainImageViews;     // 스왑 체인 이미지 뷰 -> 스왑 체인 이미지를 뷰로 변환 (이미지 뷰는 이미지를 읽고 쓰는 데 사용됨)
        VkFormat VKswapChainImageFormat;                    // 스왑 체인 이미지 포맷 -> 스왑 체인 이미지의 픽셀 형식
        VkExtent2D VKswapChainExtent;                       // 스왑 체인 이미지 해상도 -> 스왑 체인 이미지의 너비와 높이
        VkPipelineLayout VKpipelineLayout;                  // 파이프라인 레이아웃 -> 파이프라인에 사용되는 레이아웃
        VkRenderPass VKrenderPass;                          // 렌더 패스 -> 렌더링 작업을 정의하는 데 사용
        VkPipeline VKgraphicsPipeline;                      // 그래픽스 파이프라인 -> 그래픽스 파이프라인을 생성
        std::vector<VkFramebuffer> VKswapChainFramebuffers; // 스왑 체인 프레임 버퍼 -> 스왑 체인 이미지를 렌더링할 때 사용 (프레임 버퍼는 이미지를 렌더링하는 데 사용)
        VkCommandPool VKcommandPool;                        // 커맨드 풀 -> 커맨드 버퍼를 생성하는 데 사용 (커맨드 풀은 커맨드 버퍼를 생성하는 데 사용)
        std::vector<VkCommandBuffer> VKcommandBuffers;      // 커맨드 버퍼 -> 렌더링 명령을 저장하는 데 사용
        std::vector<VkSemaphore> VkimageavailableSemaphore; // 이미지 사용 가능 세마포어 -> 이미지를 가져오기 위해 사용
        std::vector<VkSemaphore> VkrenderFinishedSemaphore; // 렌더링 완료 세마포어 -> 렌더링이 완료되었음을 알리는 데 사용
        std::vector<VkFence> VkinFlightFences;              // 플라이트 펜스 -> 프레임이 완료되었음을 알리는 데 사용
        VkBuffer VKvertexBuffer;                            // 버텍스 버퍼 -> 버텍스 데이터를 저장하는 데 사용
        VkDeviceMemory VKvertexBufferMemory;                // 버텍스 버퍼 메모리 -> 버텍스 데이터를 저장하는 데 사용

        std::string RootPath = "";                          // 루트 경로
        size_t currentFrame = 0;                            // 현재 프레임 인덱스
        bool state;                                         // 프로그램 상태 
    };

    int rateDeviceSuitability(VkPhysicalDevice device);
    void getPyhsicalDeviceProperties(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
}

#endif // INCLUDE_SOURCE_APPLICATION_H