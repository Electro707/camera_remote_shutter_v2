# uses https://github.com/ObKo/stm32-cmake

cmake_minimum_required(VERSION 3.12)

set(CMAKE_TOOLCHAIN_FILE "/home/electro/STM32Cube/Repository/stm32-cmake/cmake/stm32_gcc.cmake")

project(CameraRemoteShutterV2 C ASM)

set(LV_CONF_BUILD_DISABLE_EXAMPLES TRUE)
set(LV_CONF_BUILD_DISABLE_DEMOS TRUE)
set(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL TRUE)

set(STM32_CUBE_G0_PATH "/home/electro/STM32Cube/Repository/STM32CubeG0")
set(STM32_TOOLCHAIN_PATH "/opt/stm32cubeide/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.12.3.rel1.linux64_1.1.0.202410170702/tools/bin/")

find_package(CMSIS COMPONENTS STM32G071K REQUIRED)
find_package(HAL COMPONENTS STM32G071K REQUIRED)

add_executable(CameraRemoteShutterV2 main.c)

# file(GLOB_RECURSE LVGL_SOURCES ${CMAKE_SOURCE_DIR}/lvgl/src/*.c ${CMAKE_SOURCE_DIR}/lvgl/src/*.S)

target_include_directories(CameraRemoteShutterV2 PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/gc9a01
    ${CMAKE_SOURCE_DIR}/lvgl
)

# target_include_directories(lvgl PUBLIC
#     ${CMAKE_SOURCE_DIR}
# )

target_sources(CameraRemoteShutterV2 PRIVATE
    syscalls.c
    main.c
    gc9a01/gc9a01.c
    # ${LVGL_SOURCES}
)

# stm32_add_linker_script(CameraRemoteShutterV2 PRIVATE STM32G071K8TX_FLASH.ld)


target_link_libraries(CameraRemoteShutterV2 PRIVATE
    HAL::STM32::G0::LL_RCC
    HAL::STM32::G0::LL_UTILS
    HAL::STM32::G0::LL_GPIO
    CMSIS::STM32::G071KB
    STM32::NoSys
    )

target_compile_options(CameraRemoteShutterV2 PRIVATE 
    -O0
)

target_link_options(CameraRemoteShutterV2 PRIVATE 
    LINKER:-Map=foo.map
)

stm32_print_size_of_target(CameraRemoteShutterV2)
stm32_generate_binary_file(CameraRemoteShutterV2)

add_definitions(-fdiagnostics-color=always)
