// CUDA 공식 예제를 변경하였습니다.

#include"include/app/VulkanCudaInterop.h"
#include "helper_cuda.h"

int main(int argc, char **argv) {

    VulkanCudaInterop app;

    std::string image_filename = "source/image.jpg";

    try {
        // This app only works on ppm images
        app.LoadImageData(image_filename);
        app.Run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
