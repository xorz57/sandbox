#include "camera.hpp"
#include "scope_exit.hpp"
#include "light.hpp"
#include "material.hpp"
#include "shader.hpp"
#include "vertex.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_video.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <vector>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int main() {
#pragma region InitSDL
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    spdlog::error("SDL_Init failed: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto sdl_guard{ScopeExit{[] { SDL_Quit(); }}};
#pragma endregion InitSDL

#pragma region SetAttributes
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#pragma endregion SetAttributes

#pragma region CreateWindow
  const char *title{"sandbox"};
  const float main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
  const std::int32_t w{static_cast<std::int32_t>(1'280 * main_scale)};
  const std::int32_t h{static_cast<std::int32_t>(720 * main_scale)};
  // clang-format off
  const SDL_WindowFlags flags{
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY
  };
  // clang-format on
  SDL_Window *window{SDL_CreateWindow(title, w, h, flags)};
  if (window == nullptr) {
    spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto window_guard{ScopeExit{[window] { SDL_DestroyWindow(window); }}};
#pragma endregion CreateWindow

#pragma region CreateContext
  const SDL_GLContext context{SDL_GL_CreateContext(window)};
  if (context == nullptr) {
    spdlog::error("SDL_GL_CreateContext failed: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto context_guard{ScopeExit{[context] { SDL_GL_DestroyContext(context); }}};
#pragma endregion CreateContext

  SDL_GL_SetSwapInterval(1);

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)) == 0) {
    spdlog::error("gladLoadGL failed");
    return EXIT_FAILURE;
  }

#pragma region LogContextInfo
  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  spdlog::info("OpenGL Version: {}", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
  spdlog::info("GLSL Version: {}", reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
  spdlog::info("Vendor: {}", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
  spdlog::info("Renderer: {}", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
#pragma endregion LogContextInfo

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  const glm::vec3 ambient_light{0.1F, 0.1F, 0.1F};

  const Light light{.position = {2.0F, 4.0F, 3.0F}, .color = {1.0F, 1.0F, 1.0F}, .intensity = 1.0F};

  const Material material1{.diffuse = {1.0F, 0.0F, 0.0F}, .specular = {1.0F, 0.5F, 0.5F}, .shininess = 64.0F};
  const Material material2{.diffuse = {0.0F, 1.0F, 0.0F}, .specular = {0.5F, 1.0F, 0.5F}, .shininess = 32.0F};
  const Material material3{.diffuse = {0.0F, 0.0F, 1.0F}, .specular = {0.5F, 0.5F, 1.0F}, .shininess = 16.0F};

  #pragma region Vertices
  // clang-format off
  const std::vector<Vertex> vertices{
    // +z Front
    {.position = {-0.5F, -0.5F,  0.5F}, .normal = { 0.0F,  0.0F,  1.0F}, .uv = { 0.0F,  0.0F}},
    {.position = { 0.5F, -0.5F,  0.5F}, .normal = { 0.0F,  0.0F,  1.0F}, .uv = { 1.0F,  0.0F}},
    {.position = { 0.5F,  0.5F,  0.5F}, .normal = { 0.0F,  0.0F,  1.0F}, .uv = { 1.0F,  1.0F}},
    {.position = {-0.5F,  0.5F,  0.5F}, .normal = { 0.0F,  0.0F,  1.0F}, .uv = { 0.0F,  1.0F}},
    // -z Back
    {.position = { 0.5F, -0.5F, -0.5F}, .normal = { 0.0F,  0.0F, -1.0F}, .uv = { 0.0F,  0.0F}},
    {.position = {-0.5F, -0.5F, -0.5F}, .normal = { 0.0F,  0.0F, -1.0F}, .uv = { 1.0F,  0.0F}},
    {.position = {-0.5F,  0.5F, -0.5F}, .normal = { 0.0F,  0.0F, -1.0F}, .uv = { 1.0F,  1.0F}},
    {.position = { 0.5F,  0.5F, -0.5F}, .normal = { 0.0F,  0.0F, -1.0F}, .uv = { 0.0F,  1.0F}},
    // -x Left
    {.position = {-0.5F, -0.5F, -0.5F}, .normal = {-1.0F,  0.0F,  0.0F}, .uv = { 0.0F,  0.0F}},
    {.position = {-0.5F, -0.5F,  0.5F}, .normal = {-1.0F,  0.0F,  0.0F}, .uv = { 1.0F,  0.0F}},
    {.position = {-0.5F,  0.5F,  0.5F}, .normal = {-1.0F,  0.0F,  0.0F}, .uv = { 1.0F,  1.0F}},
    {.position = {-0.5F,  0.5F, -0.5F}, .normal = {-1.0F,  0.0F,  0.0F}, .uv = { 0.0F,  1.0F}},
    // +x Right
    {.position = { 0.5F, -0.5F,  0.5F}, .normal = { 1.0F,  0.0F,  0.0F}, .uv = { 0.0F,  0.0F}},
    {.position = { 0.5F, -0.5F, -0.5F}, .normal = { 1.0F,  0.0F,  0.0F}, .uv = { 1.0F,  0.0F}},
    {.position = { 0.5F,  0.5F, -0.5F}, .normal = { 1.0F,  0.0F,  0.0F}, .uv = { 1.0F,  1.0F}},
    {.position = { 0.5F,  0.5F,  0.5F}, .normal = { 1.0F,  0.0F,  0.0F}, .uv = { 0.0F,  1.0F}},
    // +y Top
    {.position = {-0.5F,  0.5F,  0.5F}, .normal = { 0.0F,  1.0F,  0.0F}, .uv = { 0.0F,  0.0F}},
    {.position = { 0.5F,  0.5F,  0.5F}, .normal = { 0.0F,  1.0F,  0.0F}, .uv = { 1.0F,  0.0F}},
    {.position = { 0.5F,  0.5F, -0.5F}, .normal = { 0.0F,  1.0F,  0.0F}, .uv = { 1.0F,  1.0F}},
    {.position = {-0.5F,  0.5F, -0.5F}, .normal = { 0.0F,  1.0F,  0.0F}, .uv = { 0.0F,  1.0F}},
    // -y Bottom
    {.position = {-0.5F, -0.5F, -0.5F}, .normal = { 0.0F, -1.0F,  0.0F}, .uv = { 0.0F,  0.0F}},
    {.position = { 0.5F, -0.5F, -0.5F}, .normal = { 0.0F, -1.0F,  0.0F}, .uv = { 1.0F,  0.0F}},
    {.position = { 0.5F, -0.5F,  0.5F}, .normal = { 0.0F, -1.0F,  0.0F}, .uv = { 1.0F,  1.0F}},
    {.position = {-0.5F, -0.5F,  0.5F}, .normal = { 0.0F, -1.0F,  0.0F}, .uv = { 0.0F,  1.0F}},
  };
  // clang-format on
  #pragma endregion Vertices

  #pragma region Indices
  // clang-format off
  const std::vector<std::uint32_t> indices {
     0,  1,  2,  2,  3,  0,  // +z Front
     4,  5,  6,  6,  7,  4,  // -z Back
     8,  9, 10, 10, 11,  8,  // -x Left
    12, 13, 14, 14, 15, 12,  // +x Right
    16, 17, 18, 18, 19, 16,  // +y Top
    20, 21, 22, 22, 23, 20,  // -y Bottom
  };
  // clang-format on
  #pragma endregion Indices

  const auto index_count{static_cast<std::int32_t>(indices.size())};

#pragma region CreateBuffers
  std::uint32_t vbo{};
  glCreateBuffers(1, &vbo);
  const auto vbo_guard{ScopeExit{[vbo] { glDeleteBuffers(1, &vbo); }}};

  std::uint32_t ebo{};
  glCreateBuffers(1, &ebo);
  const auto ebo_guard{ScopeExit{[ebo] { glDeleteBuffers(1, &ebo); }}};

  std::uint32_t vao{};
  glCreateVertexArrays(1, &vao);
  const auto vao_guard{ScopeExit{[vao] { glDeleteVertexArrays(1, &vao); }}};

  glNamedBufferStorage(vbo, static_cast<std::ptrdiff_t>(vertices.size() * sizeof(Vertex)), vertices.data(), 0);
  glNamedBufferStorage(ebo, static_cast<std::ptrdiff_t>(indices.size() * sizeof(std::uint32_t)), indices.data(), 0);

  glVertexArrayVertexBuffer(vao, 0, vbo, 0, static_cast<std::int32_t>(sizeof(Vertex)));
  glVertexArrayElementBuffer(vao, ebo);

  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);

  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
  glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
  glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, uv));

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);
#pragma endregion CreateBuffers

  auto lit_shader{Shader::CreateFromFiles("assets/shaders/lit.vert.glsl", "assets/shaders/lit.frag.glsl")};
  if (!lit_shader.has_value()) {
    spdlog::error("{}", lit_shader.error());
    return EXIT_FAILURE;
  }

  auto unlit_shader{Shader::CreateFromFiles("assets/shaders/unlit.vert.glsl", "assets/shaders/unlit.frag.glsl")};
  if (!unlit_shader.has_value()) {
    spdlog::error("{}", unlit_shader.error());
    return EXIT_FAILURE;
  }

  const Camera camera{{0.0F, 2.0F, 10.0F}, {}};

  bool running{true};

  while (running) {
    SDL_Event event{};
#pragma region ProcessEvents
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        running = false;
        break;
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        if (event.window.windowID == SDL_GetWindowID(window)) {
          running = false;
        }
        break;
      case SDL_EVENT_KEY_DOWN:
        if (!event.key.repeat && (event.key.key == SDLK_RETURN) && static_cast<bool>(event.key.mod & SDL_KMOD_ALT)) {
          const bool is_fullscreen{static_cast<bool>(SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN)};
          SDL_SetWindowFullscreen(window, !is_fullscreen);
        }
        break;
      default:
        break;
      }
    }
#pragma endregion ProcessEvents

    #pragma region UpdateViewport
    std::int32_t framebuffer_w{};
    std::int32_t framebuffer_h{};
    SDL_GetWindowSizeInPixels(window, &framebuffer_w, &framebuffer_h);
    glViewport(0, 0, framebuffer_w, framebuffer_h);
    #pragma endregion UpdateViewport

    #pragma region ClearFramebuffer
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    #pragma endregion ClearFramebuffer

    const float framebuffer_a{static_cast<float>(framebuffer_w) / static_cast<float>(framebuffer_h)};
    const glm::mat4 projection{camera.ComputeProjectionMatrix(framebuffer_a)};
    const glm::mat4 view{camera.ComputeViewMatrix()};

    glBindVertexArray(vao);

    unlit_shader->Use();
    unlit_shader->SetUniform("u_projection", projection);
    unlit_shader->SetUniform("u_view", view);

#pragma region DrawLight
    glm::mat4 light_model{glm::mat4{1.0F}};
    light_model = glm::translate(light_model, light.position);
    unlit_shader->SetUniform("u_model", light_model);
    unlit_shader->SetUniform("u_color", light.color);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, static_cast<void *>(nullptr));
#pragma endregion DrawLight

    lit_shader->Use();
    lit_shader->SetUniform("u_projection", projection);
    lit_shader->SetUniform("u_view", view);
    lit_shader->SetUniform("u_ambient_light", ambient_light);
    lit_shader->SetUniform("u_light.position", light.position);
    lit_shader->SetUniform("u_light.color", light.color);
    lit_shader->SetUniform("u_light.intensity", light.intensity);
    lit_shader->SetUniform("u_view_position", camera.GetPosition());

#pragma region DrawBox1
    glm::mat4 model1{glm::mat4{1.0F}};
    model1 = glm::translate(model1, {-1.0F, 2.0F, -1.0F});
    lit_shader->SetUniform("u_model", model1);
    lit_shader->SetUniform("u_material.diffuse", material1.diffuse);
    lit_shader->SetUniform("u_material.specular", material1.specular);
    lit_shader->SetUniform("u_material.shininess", material1.shininess);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, static_cast<void *>(nullptr));
#pragma endregion DrawBox1

#pragma region DrawBox2
    glm::mat4 model2{glm::mat4{1.0F}};
    model2 = glm::translate(model2, {1.0F, 2.0F, 1.0F});
    lit_shader->SetUniform("u_model", model2);
    lit_shader->SetUniform("u_material.diffuse", material2.diffuse);
    lit_shader->SetUniform("u_material.specular", material2.specular);
    lit_shader->SetUniform("u_material.shininess", material2.shininess);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, static_cast<void *>(nullptr));
#pragma endregion DrawBox2

#pragma region DrawBox3
    glm::mat4 model3{glm::mat4{1.0F}};
    model3 = glm::scale(model3, {4.0F, 1.0F, 4.0F});
    lit_shader->SetUniform("u_model", model3);
    lit_shader->SetUniform("u_material.diffuse", material3.diffuse);
    lit_shader->SetUniform("u_material.specular", material3.specular);
    lit_shader->SetUniform("u_material.shininess", material3.shininess);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, static_cast<void *>(nullptr));
#pragma endregion DrawBox3

    SDL_GL_SwapWindow(window);
  }

  return EXIT_SUCCESS;
}
