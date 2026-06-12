cmake_minimum_required(VERSION 3.30)
project(GreenhouseOptimalCropPlacement)

set(CMAKE_CXX_STANDARD 20)

add_executable(GreenhouseOptimalCropPlacement main.cpp)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
    
   
    if(TARGET GreenhouseOptimalCropPlacement)
        target_link_options(GreenhouseOptimalCropPlacement PRIVATE "-static-libgcc" "-static-libstdc++" "-static")
    endif()
endif()
