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
#include "material.hpp"
#include "mesh_data.hpp"
#include "point_light.hpp"
#include "primitives.hpp"
#include "program.hpp"
#include "scope_exit.hpp"
#include "types.hpp"
#include "vertex.hpp"

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int main() {
#pragma region InitSDL
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    spdlog::error("Failed to initialize SDL: {}", SDL_GetError());
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
  const f32 main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
  const i32 w{static_cast<i32>(1'280 * main_scale)};
  const i32 h{static_cast<i32>(720 * main_scale)};
  const SDL_WindowFlags flags{SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN};
  SDL_Window *window{SDL_CreateWindow(title, w, h, flags)};
  if (window == nullptr) {
    spdlog::error("Failed to create SDL window: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto window_guard{ScopeExit{[window] { SDL_DestroyWindow(window); }}};
#pragma endregion CreateWindow

#pragma region CreateContext
  const SDL_GLContext context{SDL_GL_CreateContext(window)};
  if (context == nullptr) {
    spdlog::error("Failed to create OpenGL context: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto context_guard{ScopeExit{[context] { SDL_GL_DestroyContext(context); }}};
#pragma endregion CreateContext

  SDL_GL_SetSwapInterval(1);

  SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(window);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)) == 0) {
    spdlog::error("Failed to initialize GLAD.");
    return EXIT_FAILURE;
  }

  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  spdlog::info("OpenGL Version: {}", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
  spdlog::info("GLSL Version: {}", reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
  spdlog::info("Vendor: {}", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
  spdlog::info("Renderer: {}", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  const PointLight point_light{
      .position = {0.0F, 0.0F, 2.0F},
      .ambient = {0.2F, 0.2F, 0.2F},
      .diffuse = {1.0F, 1.0F, 1.0F},
      .specular = {1.0F, 1.0F, 1.0F},
      .kc = 1.0F,
      .kl = 0.09F,
      .kq = 0.032F,
  };

  const Material cube1_material{
      .ambient = {0.1F, 0.1F, 0.1F},
      .diffuse = {1.0F, 0.0F, 0.0F},
      .specular = {1.0F, 0.5F, 0.5F},
      .shininess = 32.0F,
  };
  const Material cube2_material{
      .ambient = {0.1F, 0.1F, 0.1F},
      .diffuse = {0.0F, 1.0F, 0.0F},
      .specular = {0.5F, 1.0F, 0.5F},
      .shininess = 32.0F,
  };
  const Material cube3_material{
      .ambient = {0.1F, 0.1F, 0.1F},
      .diffuse = {0.0F, 0.0F, 1.0F},
      .specular = {0.5F, 0.5F, 1.0F},
      .shininess = 32.0F,
  };
  const Material cube4_material{
      .ambient = {0.1F, 0.1F, 0.1F},
      .diffuse = {1.0F, 1.0F, 0.0F},
      .specular = {1.0F, 1.0F, 0.5F},
      .shininess = 32.0F,
  };

  glm::mat4 cube1_model{glm::mat4{1.0F}};
  glm::mat4 cube2_model{glm::mat4{1.0F}};
  glm::mat4 cube3_model{glm::mat4{1.0F}};
  glm::mat4 cube4_model{glm::mat4{1.0F}};

  cube1_model = glm::translate(cube1_model, {-1.0F, 1.0F, 0.0F});
  cube2_model = glm::translate(cube2_model, {1.0F, 1.0F, 0.0F});
  cube3_model = glm::translate(cube3_model, {-1.0F, -1.0F, 0.0F});
  cube4_model = glm::translate(cube4_model, {1.0F, -1.0F, 0.0F});

  const glm::mat3 cube1_normal_matrix{glm::transpose(glm::inverse(glm::mat3{cube1_model}))};
  const glm::mat3 cube2_normal_matrix{glm::transpose(glm::inverse(glm::mat3{cube2_model}))};
  const glm::mat3 cube3_normal_matrix{glm::transpose(glm::inverse(glm::mat3{cube3_model}))};
  const glm::mat3 cube4_normal_matrix{glm::transpose(glm::inverse(glm::mat3{cube4_model}))};

  const MeshData cube_mesh_data{primitives::MakeCube()};

  u32 cube_vbo{};
  glCreateBuffers(1, &cube_vbo);
  if (cube_vbo == 0) {
    spdlog::error("Failed to create OpenGL vertex buffer.");
    return EXIT_FAILURE;
  }
  const auto cube_vbo_guard{ScopeExit{[&] { glDeleteBuffers(1, &cube_vbo); }}};

  u32 cube_ebo{};
  glCreateBuffers(1, &cube_ebo);
  if (cube_ebo == 0) {
    spdlog::error("Failed to create OpenGL index buffer.");
    return EXIT_FAILURE;
  }
  const auto cube_ebo_guard{ScopeExit{[&] { glDeleteBuffers(1, &cube_ebo); }}};

  u32 cube_vao{};
  glCreateVertexArrays(1, &cube_vao);
  if (cube_vao == 0) {
    spdlog::error("Failed to create OpenGL vertex array.");
    return EXIT_FAILURE;
  }
  const auto cube_vao_guard{ScopeExit{[&] { glDeleteVertexArrays(1, &cube_vao); }}};

  const isize vertices_bytes{static_cast<isize>(cube_mesh_data.vertices.size() * sizeof(Vertex))};
  glNamedBufferStorage(cube_vbo, vertices_bytes, cube_mesh_data.vertices.data(), 0);

  const isize indices_bytes{static_cast<isize>(cube_mesh_data.indices.size() * sizeof(u32))};
  glNamedBufferStorage(cube_ebo, indices_bytes, cube_mesh_data.indices.data(), 0);

  glVertexArrayVertexBuffer(cube_vao, 0, cube_vbo, 0, static_cast<i32>(sizeof(Vertex)));
  glVertexArrayElementBuffer(cube_vao, cube_ebo);

  glEnableVertexArrayAttrib(cube_vao, 0);
  glEnableVertexArrayAttrib(cube_vao, 1);
  glEnableVertexArrayAttrib(cube_vao, 2);

  glVertexArrayAttribFormat(cube_vao, 0, 3, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, position)));
  glVertexArrayAttribFormat(cube_vao, 1, 3, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, normal)));
  glVertexArrayAttribFormat(cube_vao, 2, 2, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, uv)));

  glVertexArrayAttribBinding(cube_vao, 0, 0);
  glVertexArrayAttribBinding(cube_vao, 1, 0);
  glVertexArrayAttribBinding(cube_vao, 2, 0);

  const i32 cube_index_count{static_cast<i32>(cube_mesh_data.indices.size())};

  auto lit_program{Program::Create("assets/shaders/lit")};
  if (!lit_program.has_value()) {
    return EXIT_FAILURE;
  }

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

    glBindVertexArray(cube_vao);

    lit_program->ReloadIfChanged();

    lit_program->Use();
    lit_program->SetUniform("u_projection", projection);
    lit_program->SetUniform("u_view", view);
    lit_program->SetUniform("u_point_light.position", point_light.position);
    lit_program->SetUniform("u_point_light.ambient", point_light.ambient);
    lit_program->SetUniform("u_point_light.diffuse", point_light.diffuse);
    lit_program->SetUniform("u_point_light.specular", point_light.specular);
    lit_program->SetUniform("u_point_light.kc", point_light.kc);
    lit_program->SetUniform("u_point_light.kl", point_light.kl);
    lit_program->SetUniform("u_point_light.kq", point_light.kq);
    lit_program->SetUniform("u_camera_position", camera.position);

    lit_program->SetUniform("u_model", cube1_model);
    lit_program->SetUniform("u_normal_matrix", cube1_normal_matrix);
    lit_program->SetUniform("u_material.ambient", cube1_material.ambient);
    lit_program->SetUniform("u_material.diffuse", cube1_material.diffuse);
    lit_program->SetUniform("u_material.specular", cube1_material.specular);
    lit_program->SetUniform("u_material.shininess", cube1_material.shininess);
    glDrawElements(GL_TRIANGLES, cube_index_count, GL_UNSIGNED_INT, nullptr);

    lit_program->SetUniform("u_model", cube2_model);
    lit_program->SetUniform("u_normal_matrix", cube2_normal_matrix);
    lit_program->SetUniform("u_material.ambient", cube2_material.ambient);
    lit_program->SetUniform("u_material.diffuse", cube2_material.diffuse);
    lit_program->SetUniform("u_material.specular", cube2_material.specular);
    lit_program->SetUniform("u_material.shininess", cube2_material.shininess);
    glDrawElements(GL_TRIANGLES, cube_index_count, GL_UNSIGNED_INT, nullptr);

    lit_program->SetUniform("u_model", cube3_model);
    lit_program->SetUniform("u_normal_matrix", cube3_normal_matrix);
    lit_program->SetUniform("u_material.ambient", cube3_material.ambient);
    lit_program->SetUniform("u_material.diffuse", cube3_material.diffuse);
    lit_program->SetUniform("u_material.specular", cube3_material.specular);
    lit_program->SetUniform("u_material.shininess", cube3_material.shininess);
    glDrawElements(GL_TRIANGLES, cube_index_count, GL_UNSIGNED_INT, nullptr);

    lit_program->SetUniform("u_model", cube4_model);
    lit_program->SetUniform("u_normal_matrix", cube4_normal_matrix);
    lit_program->SetUniform("u_material.ambient", cube4_material.ambient);
    lit_program->SetUniform("u_material.diffuse", cube4_material.diffuse);
    lit_program->SetUniform("u_material.specular", cube4_material.specular);
    lit_program->SetUniform("u_material.shininess", cube4_material.shininess);
    glDrawElements(GL_TRIANGLES, cube_index_count, GL_UNSIGNED_INT, nullptr);

    SDL_GL_SwapWindow(window);
  }

  return EXIT_SUCCESS;
}
