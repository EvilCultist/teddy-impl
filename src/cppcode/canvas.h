#include "imgui.h"
#include <cstddef>
#include <vector>
#define GL_SILENCE_DEPRECATION
#include <GL/gl.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
                        //

// static std::vector<ImVec2> points;

struct keyframe {
  size_t index_of_max_y;
  float max_y;
  long double perimeter;
  std::vector<ImVec2> points;
};

extern std::vector<keyframe> keyframes;
// keyframe 0 is always currentkeyframe

void RenderCanvas();
void resample(size_t kf_index, size_t n);
void RenderCanvasLerp(float t);
