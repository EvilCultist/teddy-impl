#include <algorithm>
#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <imgui.h>
#include <iostream>
#include <ostream>
#include <stdio.h>
#define IMGUI_IMPL_OPENGL_ES3
#define IM_VEC2_CLASS_EXTRA
#include "canvas.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define GL_SILENCE_DEPRECATION
#include <GL/gl.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

std::ostream &operator<<(std::ostream &out, ImVec2 point) {
  out << point.x << ", " << point.y;
  return out;
}

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

struct Line {
  ImVec2 p1, p2;
};

// Main code
int main(int, char **) {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  const char *glsl_version = "#version 300 es";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(
      1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
  if (window == nullptr)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);
  // Our state
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  keyframes.push_back({0, 0, 0, {}});
  int n_points = 8;
  bool editslice = true;
  float lerp_anim_t = 0.0f;
  bool show_3d_render = false;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10); // do not run every frame if minimised
      continue;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
      ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::Begin("canvas", nullptr,
                   ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoBringToFrontOnFocus);
      if (editslice) {
        RenderCanvas();
      } else {
        RenderCanvasLerp(lerp_anim_t);
      }
      ImGui::End();
      ImGui::PopStyleVar(1);
    }

    if (show_3d_render) {
      ImGui::Begin("3d render");
      ImGui::End();
    }

    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("controls"); // Create a window called "Hello, world!"
                                // and append into it.
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / io.Framerate, io.Framerate);
      if (ImGui::Checkbox("Edit settings", &editslice)) {
        if (!editslice) {
          uint64_t total_no_vertices = 0;
          for (auto frame : keyframes) {
            total_no_vertices += frame.points.size();
          }
          n_points = (long double)(total_no_vertices) / keyframes.size();
          // n_points = std::min(n_points, 3000);
          std::cout << "resampling to : " << n_points << " points" << std::endl;
          for (int i = 0; i < keyframes.size(); i++) {
            resample(i, n_points);
          }
        }
      }
      if (editslice) {
        if (ImGui::Button("Save Frame")) {
          std::rotate(keyframes[0].points.begin(),
                      keyframes[0].points.begin() + keyframes[0].index_of_max_y,
                      keyframes[0].points.end());
          keyframes[0].index_of_max_y = 0;
          keyframes.insert(keyframes.begin(), {0, 0, 0, {}});
        }
        ImGui::SliderInt("n_points", &n_points, 4, 16);
        ImGui::Text("%lu", (long unsigned int)1 << n_points);
        if (ImGui::Button("Resample")) {
          std::rotate(keyframes[0].points.begin(),
                      keyframes[0].points.begin() + keyframes[0].index_of_max_y,
                      keyframes[0].points.end());
          keyframes[0].index_of_max_y = 0;
          resample(0, 1 << n_points);
        }
      } else {
        ImGui::SliderFloat("t", &lerp_anim_t, 0.0, 0.999);
        // std::cout << lerp_anim_t << std::endl;
      }
      ImGui::Checkbox("Show Render", &show_3d_render);
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);

    if (ImGui::IsKeyDown(ImGuiKey_Escape))
      break;
  }

  // std::cout << keyframes.size() << std::endl;
  // std::cout << points[0] << " - " << points[points.size() - 1] << std::endl;

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
