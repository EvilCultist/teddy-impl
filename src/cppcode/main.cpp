#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <ostream>
#include <stdio.h>
#include <vector>
// #define IM_VEC2_CLASS_EXTRA
#define IMGUI_IMPL_OPENGL_ES3
#include "imgui.h"
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

static std::vector<Line> lines;
static std::vector<ImVec2> points;

void RenderCanvas() {
  static bool is_drawing = false;
  static ImVec2 last_pos = ImVec2(0, 0);

  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
  ImVec2 canvas_size = ImGui::GetContentRegionAvail();
  // if (canvas_size.x < 50.0f)
  //   canvas_size.x = 50.0f;
  // if (canvas_size.y < 50.0f)
  //   canvas_size.y = 50.0f;

  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  // draw_list->AddRectFilled(
  //     canvas_pos,
  //     ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
  //     IM_COL32(50, 50, 50, 255));
  // draw_list->AddRect(
  //     canvas_pos,
  //     ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y),
  //     IM_COL32(255, 255, 255, 255));

  ImGui::InvisibleButton("canvas", canvas_size,
                         ImGuiButtonFlags_MouseButtonLeft |
                             ImGuiButtonFlags_MouseButtonRight);
  bool is_hovered = ImGui::IsItemHovered();
  bool is_active = ImGui::IsItemActive();
  ImVec2 mouse_pos = ImGui::GetIO().MousePos;
  ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;

  if (is_hovered && ImGui::IsMouseClicked(0)) {
    is_drawing = true;
    if (lines.size() > 0)
      lines.pop_back();
    if (points.size() > 0)
      points.pop_back();
    last_pos.x = mouse_pos.x - canvas_pos.x;
    last_pos.y = mouse_pos.y - canvas_pos.y;
  }

  if (is_hovered && ImGui::IsMouseDown(1)) {
    points.pop_back();
  }

  if (is_hovered && ImGui::IsKeyPressed(ImGuiKey_Tab)) {
    lines.clear();
    points.clear();
  }

  if (is_drawing) {
    if (is_active && ImGui::IsMouseDown(0)) {
      ImVec2 current_pos;
      current_pos.x = mouse_pos.x - canvas_pos.x;
      current_pos.y = mouse_pos.y - canvas_pos.y;
      lines.push_back({last_pos, current_pos});
      points.push_back(current_pos);
      last_pos = current_pos;
    } else {
      is_drawing = false;
      lines.push_back({last_pos, lines[0].p1});
    }
  }

  for (const Line &line : lines) {
    draw_list->AddLine(
        ImVec2(canvas_pos.x + line.p1.x, canvas_pos.y + line.p1.y),
        ImVec2(canvas_pos.x + line.p2.x, canvas_pos.y + line.p2.y),
        IM_COL32(155, 155, 155, 255), 2.0f);
  }
}

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
#ifdef IMGUI_HAS_VIEWPORT
      ImGuiViewport *viewport = ImGui::GetMainViewport();
      ImGui::SetNextWindowPos(viewport->GetWorkPos());
      ImGui::SetNextWindowSize(viewport->GetWorkSize());
      ImGui::SetNextWindowViewport(viewport->ID);
#else
      ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
      ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
      ImGui::Begin("canvas", nullptr,
                   ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
                       ImGuiWindowFlags_NoBringToFrontOnFocus);
      RenderCanvas();
      ImGui::End();
      ImGui::PopStyleVar(1);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair
    // to create a named window.

    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                     // and append into it.

      ImGui::Text("This is some useful text."); // Display some text (you can
                                                // use a format strings too)
                                                //
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / io.Framerate, io.Framerate);
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
  std::cout << lines.size() << std::endl;
  std::cout << lines[0].p1 << " - " << lines[lines.size() - 1].p2 << std::endl;
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
