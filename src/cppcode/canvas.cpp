#include "canvas.h"
#include "glad/gl.h"
#include "shader.h"
#include <GL/gl.h>
#include <algorithm>
#include <cmath>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/trigonometric.hpp>
#include <imgui.h>
#include <iostream>
#include <random>
// #include <sched.h>
#include <vector>

std::vector<keyframe> keyframes;

GLuint framebuffer, tex, rbo;
GLuint meshVAO, meshVBO;

float canvasheight, canvaswidth;

Shader *shdr = nullptr;

void RenderCanvasLerp(float t_raw) {
  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
  ImVec2 canvas_size = ImGui::GetContentRegionAvail();
  canvasheight = canvas_size.y;
  canvaswidth = canvas_size.x;

  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  int i = 0, j = 0;
  float t = t_raw;

  if (keyframes.size() == 2) {
    j = 1;
  }
  if (keyframes.size() > 2) {
    t_raw *= (keyframes.size() - 1);
    i = floor(t_raw);
    j = i + 1;
    t = t_raw - i;
  }

  std::vector<ImVec2> lerpresult;

  {
    // Lerp
    for (int index = 0; index < keyframes[i].points.size(); index++) {
      lerpresult.push_back(ImVec2((1 - t) * keyframes[i].points[index].x +
                                      t * keyframes[j].points[index].x,
                                  (1 - t) * keyframes[i].points[index].y +
                                      t * keyframes[j].points[index].y));
    }
    lerpresult.push_back(lerpresult[0]);
  }

  {
    // render
    for (int index = 1; index < lerpresult.size(); index++) {
      draw_list->AddLine(ImVec2(canvas_pos.x + lerpresult[index - 1].x,
                                canvas_pos.y + lerpresult[index - 1].y),
                         ImVec2(canvas_pos.x + lerpresult[index].x,
                                canvas_pos.y + lerpresult[index].y),
                         IM_COL32(255, 255, 255, 255), 2.0f);
    }
  }
}

void RenderCanvas() {
  static bool is_drawing = false;

  ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
  ImVec2 canvas_size = ImGui::GetContentRegionAvail();
  if (canvas_size.x < 50.0f)
    canvas_size.x = 50.0f;
  if (canvas_size.y < 50.0f)
    canvas_size.y = 50.0f;

  ImDrawList *draw_list = ImGui::GetWindowDrawList();

  ImGui::InvisibleButton("canvas", canvas_size,
                         ImGuiButtonFlags_MouseButtonLeft |
                             ImGuiButtonFlags_MouseButtonRight);
  bool is_hovered = ImGui::IsItemHovered();
  bool is_active = ImGui::IsItemActive();
  ImVec2 mouse_pos = ImGui::GetIO().MousePos;
  ImVec2 mouse_delta = ImGui::GetIO().MouseDelta;

  if (is_hovered && ImGui::IsMouseClicked(0)) {
    is_drawing = true;
    if (keyframes[0].points.size() > 0)
      keyframes[0].points.pop_back();
  }

  if (is_hovered && ImGui::IsMouseDown(1)) {
    keyframes[0].points.pop_back();
  }

  if (is_hovered && ImGui::IsKeyPressed(ImGuiKey_Tab)) {
    keyframes[0].points.clear();
  }

  if (is_drawing) {
    if (is_active && ImGui::IsMouseDown(0)) {
      point current_pos;
      current_pos.x = mouse_pos.x - canvas_pos.x;
      current_pos.y = mouse_pos.y - canvas_pos.y;
      if (current_pos.y > keyframes[0].max_y) {
        keyframes[0].max_y = current_pos.y;
        keyframes[0].index_of_max_y = keyframes[0].points.size();
      }
      if (keyframes[0].points.size() > 0) {
        // keyframes[0].perimeter +=
      }
      keyframes[0].points.push_back(current_pos);
    } else {
      is_drawing = false;
    }
  }

  {
    // render
    for (int i = 1; i < keyframes[0].points.size(); i++) {
      draw_list->AddLine(ImVec2(canvas_pos.x + keyframes[0].points[i - 1].x,
                                canvas_pos.y + keyframes[0].points[i - 1].y),
                         ImVec2(canvas_pos.x + keyframes[0].points[i].x,
                                canvas_pos.y + keyframes[0].points[i].y),
                         IM_COL32(255, 255, 255, 255), 2.0f);
    }
    if (keyframes[0].points.size() > 0) {
      size_t i = keyframes[0].points.size() - 1;
      draw_list->AddLine(ImVec2(canvas_pos.x + keyframes[0].points[0].x,
                                canvas_pos.y + keyframes[0].points[0].y),
                         ImVec2(canvas_pos.x + keyframes[0].points[i].x,
                                canvas_pos.y + keyframes[0].points[i].y),
                         IM_COL32(155, 0, 0, 255), 1.0f);
    }
  }
}

float distance(const point &first, const point &other) {
  return std::sqrt((first.x - other.x) * (first.x - other.x) +
                   (first.y - other.y) * (first.y - other.y));
}

// Function to generate a random float between min and max
float randomFloat(float min, float max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(min, max);
  return dis(gen);
}

void resample(size_t kf_index, size_t n) {
  std::vector<point> &pt = keyframes[kf_index].points;
  if (pt.size() < 2 || n < 2)
    return;

  size_t pointCount = pt.size();
  std::vector<float> segmentLengths(pointCount);
  float totalLength = 0.0f;

  // Step 1: Compute segment lengths including loop back to start
  for (size_t i = 0; i < pointCount; ++i) {
    const point &a = pt[i];
    const point &b = pt[(i + 1) % pointCount]; // wrap around
    float len = distance(a, b);
    segmentLengths[i] = len;
    totalLength += len;
  }

  // Handle degenerate case
  if (totalLength < 1e-5f) {
    pt.resize(n, pt[0]);
    return;
  }

  // Step 2: Resample with equal spacing along the loop
  std::vector<point> nwPolyline;
  float targetSpacing = totalLength / n;
  float accumulated = 0.0f;
  size_t currSeg = 0;

  for (size_t i = 0; i < n; ++i) {
    float distAlong = i * targetSpacing;

    // Advance to the correct segment
    while (currSeg < segmentLengths.size() &&
           accumulated + segmentLengths[currSeg] < distAlong) {
      accumulated += segmentLengths[currSeg];
      currSeg++;
    }

    currSeg %= pointCount; // wrap segment index in loop
    float segStart = accumulated;
    float segLen = segmentLengths[currSeg];
    float t = segLen > 0 ? (distAlong - segStart) / segLen : 0.0f;

    const point &p0 = pt[currSeg];
    const point &p1 = pt[(currSeg + 1) % pointCount];

    point interp(p0.x + t * (p1.x - p0.x), p0.y + t * (p1.y - p0.y));

    nwPolyline.push_back(interp);
  }

  size_t index_of_max_y = 0;
  float max_y = 0;

  for (int i = 0; i < nwPolyline.size(); i++) {
    if (nwPolyline[i].y > max_y) {
      max_y = nwPolyline[i].y;
      index_of_max_y = i;
    }
  }

  std::rotate(nwPolyline.begin(), nwPolyline.begin() + index_of_max_y,
              nwPolyline.end());

  pt.swap(nwPolyline);
}

void initOpengl() {
  auto wsize = ImGui::GetWindowSize();
  if (!gladLoaderLoadGL()) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return;
  }

  delete shdr;
  shdr = new Shader("../src/shaders/mesh.vert", "../src/shaders/mesh.frag");

  // TODO generalize for n ribs.
  float vertices[3 * 2 * keyframes[0].points.size()];
  for (int i = 0; i < keyframes[0].points.size(); i++) {
    vertices[6 * i] = keyframes[0].points[i].x / canvaswidth;
    vertices[6 * i + 1] = keyframes[0].points[i].y / canvasheight;
    vertices[6 * i + 2] = 0.5;
    vertices[6 * i + 3] = keyframes[1].points[i].x / canvaswidth;
    vertices[6 * i + 4] = keyframes[1].points[i].y / canvasheight;
    vertices[6 * i + 5] = -0.5;
  }

  glGenVertexArrays(1, &meshVAO);
  glGenBuffers(1, &meshVBO);
  glBindVertexArray(meshVAO);
  glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
  glBufferData(GL_ARRAY_BUFFER, 3 * 2 * keyframes[0].points.size(), &vertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  glDeleteRenderbuffers(1, &rbo);
  glDeleteFramebuffers(1, &framebuffer);

  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, wsize.x, wsize.y, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         tex, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, wsize.x, wsize.y);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);
  if ((glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)) {
    std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
              << std::endl;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render3D(double yaw, double pitch, double roll) {
  // Get the current cursor position (where your window is)
  ImVec2 wsize = ImGui::GetWindowSize();
  int texwidth, texheight;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texwidth);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texheight);

  if ((glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) ||
      (texwidth != wsize.x) || (texheight != wsize.y)) {
    initOpengl();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
  glViewport(0, 0, wsize.x, wsize.y);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  shdr->use();
  glm::mat4 model = glm::mat4(1.0);
  model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 view = glm::mat4(1.0);
  view = glm::translate(view, glm::vec3(0.0f, 0.0f, -1.5f));
  glm::mat4 proj = glm::perspective(
      glm::radians(45.0f), (float)wsize.x / (float)wsize.y, 0.1f, 100.0f);
  shdr->setMat4("model", model);
  shdr->setMat4("view", view);
  shdr->setMat4("proj", proj);
  glBindVertexArray(meshVAO);
  glDisable(GL_CULL_FACE);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * keyframes[0].points.size());

  glDisable(GL_DEPTH_TEST);
  // Because I use the texture from OpenGL, I need to invert the V from the
  // UV.
  ImGui::Image((ImTextureID)tex, wsize, ImVec2(0, 1), ImVec2(1, 0));
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
