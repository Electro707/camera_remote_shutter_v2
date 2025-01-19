# uses https://github.com/ObKo/stm32-cmake

cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE "/home/electro/STM32Cube/Repository/stm32-cmake/cmake/stm32_gcc.cmake")

project(CameraRemoteShutterV2 C ASM)

set(STM32_CUBE_G0_PATH "/home/electro/STM32Cube/Repository/STM32CubeG0")
set(STM32_TOOLCHAIN_PATH "/opt/stm32cubeide/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.linux64_1.1.0.202410170702/tools/bin/")

find_package(CMSIS COMPONENTS STM32G071K REQUIRED)
find_package(HAL COMPONENTS STM32G071K REQUIRED)

set(PROJECT_SOURCES
    syscalls.c
    main.c
)

add_executable(CameraRemoteShutterV2 main.c)

target_link_libraries(CameraRemoteShutterV2
    HAL::STM32::G0::LL_RCC
    HAL::STM32::G0::LL_UTILS
    CMSIS::STM32::G071K8
    STM32::NoSys)

stm32_print_size_of_target(CameraRemoteShutterV2)
stm32_generate_binary_file(CameraRemoteShutterV2)

add_definitions(-fdiagnostics-color=always)