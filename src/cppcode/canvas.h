#include "imgui.h"
#include <cstddef>
// #include <glm/trigonometric.hpp>
#include <vector>
#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
//
// #include "glad/gl.h"

// #include "shader.h"
// static std::vector<ImVec2> points;

struct point {
  float x;
  float y;
};

struct keyframe {
  size_t index_of_max_y;
  float max_y;
  long double perimeter;
  std::vector<point> points;
};

// extern Shader *shdr;
extern std::vector<keyframe> keyframes;
// keyframe 0 is always currentkeyframe

void initOpengl();
void RenderCanvas();
void resample(size_t kf_index, size_t n);
void RenderCanvasLerp(float t);
void Render3D(double yaw, double pitch, double roll);
