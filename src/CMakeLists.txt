
set(SOURCE_DIR "Source")

set(SOURCES
    cppcode/main.cpp
    cppcode/shader.h
    cppcode/shader.cpp
    cppcode/canvas.h
    cppcode/canvas.cpp
    # cppcode/Renderer.h
    # cppcode/Renderer.cpp
    outside/stb_image.h
    outside/stb_image.cpp
)

# Add the executable target
add_executable(teddyimpl ${SOURCES})

set_target_properties(teddyimpl PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/
)

target_link_libraries(teddyimpl GL)
target_link_libraries(teddyimpl glfw)
target_link_libraries(teddyimpl glad)
target_link_libraries(teddyimpl glm)
target_link_libraries(teddyimpl imgui)

target_include_directories(teddyimpl PRIVATE outside)
