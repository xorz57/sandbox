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
#include <cstdlib>

#include "camera.hpp"
#include "mesh_data.hpp"
#include "point_light.hpp"
#include "primitives.hpp"
#include "program.hpp"
#include "scope_exit.hpp"
#include "texture.hpp"
#include "types.hpp"
#include "vertex.hpp"

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int main() {
#pragma region InitSDL
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    spdlog::error("SDL: failed to initialize: {}.", SDL_GetError());
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
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#pragma endregion SetAttributes

#pragma region CreateWindow
  const char *title{"sandbox"};
  const f32 main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
  const i32 w{static_cast<i32>(1'280 * main_scale)};
  const i32 h{static_cast<i32>(720 * main_scale)};
  const SDL_WindowFlags flags{SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN};
  SDL_Window *window{SDL_CreateWindow(title, w, h, flags)};
  if (window == nullptr) {
    spdlog::error("SDL: failed to create window: {}.", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto window_guard{ScopeExit{[window] { SDL_DestroyWindow(window); }}};
#pragma endregion CreateWindow

#pragma region CreateContext
  const SDL_GLContext context{SDL_GL_CreateContext(window)};
  if (context == nullptr) {
    spdlog::error("OpenGL: failed to create context: {}.", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto context_guard{ScopeExit{[context] { SDL_GL_DestroyContext(context); }}};
#pragma endregion CreateContext

  SDL_GL_SetSwapInterval(1);

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)) == 0) {
    spdlog::error("OpenGL: failed to initialize GLAD.");
    return EXIT_FAILURE;
  }

  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  spdlog::info("OpenGL: version {}.", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
  spdlog::info("OpenGL: GLSL version {}.", reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
  spdlog::info("OpenGL: vendor {}.", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
  spdlog::info("OpenGL: renderer {}.", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  glm::mat4 model1{glm::mat4{1.0F}};
  glm::mat4 model2{glm::mat4{1.0F}};
  glm::mat4 model3{glm::mat4{1.0F}};
  glm::mat4 model4{glm::mat4{1.0F}};

  model1 = glm::translate(model1, {-1.0F, 1.0F, 0.0F});
  model2 = glm::translate(model2, {1.0F, 1.0F, 0.0F});
  model3 = glm::translate(model3, {-1.0F, -1.0F, 0.0F});
  model4 = glm::translate(model4, {1.0F, -1.0F, 0.0F});

  const glm::mat3 normal_matrix1{glm::transpose(glm::inverse(glm::mat3{model1}))};
  const glm::mat3 normal_matrix2{glm::transpose(glm::inverse(glm::mat3{model2}))};
  const glm::mat3 normal_matrix3{glm::transpose(glm::inverse(glm::mat3{model3}))};
  const glm::mat3 normal_matrix4{glm::transpose(glm::inverse(glm::mat3{model4}))};

  const MeshData mesh_data{primitives::MakeCube()};

  u32 vbo{};
  glCreateBuffers(1, &vbo);
  if (vbo == 0) {
    spdlog::error("OpenGL: failed to create vertex buffer.");
    return EXIT_FAILURE;
  }
  const auto vbo_guard{ScopeExit{[&] { glDeleteBuffers(1, &vbo); }}};

  u32 ebo{};
  glCreateBuffers(1, &ebo);
  if (ebo == 0) {
    spdlog::error("OpenGL: failed to create index buffer.");
    return EXIT_FAILURE;
  }
  const auto ebo_guard{ScopeExit{[&] { glDeleteBuffers(1, &ebo); }}};

  u32 vao{};
  glCreateVertexArrays(1, &vao);
  if (vao == 0) {
    spdlog::error("OpenGL: failed to create vertex array.");
    return EXIT_FAILURE;
  }
  const auto vao_guard{ScopeExit{[&] { glDeleteVertexArrays(1, &vao); }}};

  const isize vertices_bytes{static_cast<isize>(mesh_data.vertices.size() * sizeof(Vertex))};
  glNamedBufferStorage(vbo, vertices_bytes, mesh_data.vertices.data(), 0);

  const isize indices_bytes{static_cast<isize>(mesh_data.indices.size() * sizeof(u32))};
  glNamedBufferStorage(ebo, indices_bytes, mesh_data.indices.data(), 0);

  auto texture{Texture::Create("assets/textures/checkerboard.png")};
  if (!texture.has_value()) {
    return EXIT_FAILURE;
  }

  glVertexArrayVertexBuffer(vao, 0, vbo, 0, static_cast<i32>(sizeof(Vertex)));
  glVertexArrayElementBuffer(vao, ebo);

  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);

  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, position)));
  glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, normal)));
  glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, uv)));

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);

  const i32 index_count{static_cast<i32>(mesh_data.indices.size())};

  auto lit_program{Program::Create("assets/shaders/lit")};
  if (!lit_program.has_value()) {
    return EXIT_FAILURE;
  }

  const PointLight point_light{
      .position = {0.0F, 0.0F, 2.0F},
      .color = {1.0F, 1.0F, 1.0F},
      .intensity = 1.0F,
      .kc = 1.0F,
      .kl = 0.09F,
      .kq = 0.032F,
  };

  const glm::vec3 ambient_light{0.1F, 0.1F, 0.1F};

  const Camera camera{.position = {0.0F, 0.0F, 4.0F}};

  bool running{true};

  while (running) {
    SDL_Event event{};
#pragma region ProcessEvents
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        running = false;
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
    i32 framebuffer_w{};
    i32 framebuffer_h{};
    SDL_GetWindowSizeInPixels(window, &framebuffer_w, &framebuffer_h);
    glViewport(0, 0, framebuffer_w, framebuffer_h);
#pragma endregion UpdateViewport

#pragma region ClearFramebuffer
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#pragma endregion ClearFramebuffer

    const f32 framebuffer_a{static_cast<f32>(framebuffer_w) / static_cast<f32>(framebuffer_h)};
    const glm::mat4 view{camera.ComputeViewMatrix()};
    const glm::mat4 projection{camera.ComputeProjectionMatrix(framebuffer_a)};

    glBindVertexArray(vao);

    texture->Bind(0);

    lit_program->ReloadIfChanged();

    lit_program->Use();

    lit_program->SetUniform("u_projection", projection);
    lit_program->SetUniform("u_view", view);

    lit_program->SetUniform("u_texture", 0);

    lit_program->SetUniform("u_point_light.position", point_light.position);
    lit_program->SetUniform("u_point_light.color", point_light.color);
    lit_program->SetUniform("u_point_light.intensity", point_light.intensity);
    lit_program->SetUniform("u_point_light.kc", point_light.kc);
    lit_program->SetUniform("u_point_light.kl", point_light.kl);
    lit_program->SetUniform("u_point_light.kq", point_light.kq);

    lit_program->SetUniform("u_ambient_light", ambient_light);

    lit_program->SetUniform("u_camera_position", camera.position);

    lit_program->SetUniform("u_model", model1);
    lit_program->SetUniform("u_normal_matrix", normal_matrix1);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

    lit_program->SetUniform("u_model", model2);
    lit_program->SetUniform("u_normal_matrix", normal_matrix2);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

    lit_program->SetUniform("u_model", model3);
    lit_program->SetUniform("u_normal_matrix", normal_matrix3);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

    lit_program->SetUniform("u_model", model4);
    lit_program->SetUniform("u_normal_matrix", normal_matrix4);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

    SDL_GL_SwapWindow(window);
  }

  return EXIT_SUCCESS;
}
