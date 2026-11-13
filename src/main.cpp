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
#include <vector>

#include "camera.hpp"
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

  u32 vbo{};
  glCreateBuffers(1, &vbo);
  if (vbo == 0) {
    spdlog::error("Failed to create OpenGL vertex buffer.");
    return EXIT_FAILURE;
  }
  const auto vbo_guard{ScopeExit{[&] { glDeleteBuffers(1, &vbo); }}};

  u32 ebo{};
  glCreateBuffers(1, &ebo);
  if (ebo == 0) {
    spdlog::error("Failed to create OpenGL index buffer.");
    return EXIT_FAILURE;
  }
  const auto ebo_guard{ScopeExit{[&] { glDeleteBuffers(1, &ebo); }}};

  u32 vao{};
  glCreateVertexArrays(1, &vao);
  if (vao == 0) {
    spdlog::error("Failed to create OpenGL vertex array.");
    return EXIT_FAILURE;
  }
  const auto vao_guard{ScopeExit{[&] { glDeleteVertexArrays(1, &vao); }}};

  const isize vertices_bytes{static_cast<isize>(cube_mesh_data.vertices.size() * sizeof(Vertex))};
  glNamedBufferStorage(vbo, vertices_bytes, cube_mesh_data.vertices.data(), 0);

  const isize indices_bytes{static_cast<isize>(cube_mesh_data.indices.size() * sizeof(u32))};
  glNamedBufferStorage(ebo, indices_bytes, cube_mesh_data.indices.data(), 0);

  constexpr i32 texture_size{8};
  constexpr usize texture_pixel_count{static_cast<usize>(texture_size) * static_cast<usize>(texture_size) * 4U};
  std::vector<u8> texture_pixels;
  texture_pixels.reserve(texture_pixel_count);
  for (i32 y{}; y < texture_size; ++y) {
    for (i32 x{}; x < texture_size; ++x) {
      const bool is_light_tile{((x / 2) + (y / 2)) % 2 == 0};
      texture_pixels.push_back(static_cast<u8>(is_light_tile ? 242 : 193));
      texture_pixels.push_back(static_cast<u8>(is_light_tile ? 204 : 102));
      texture_pixels.push_back(static_cast<u8>(is_light_tile ? 143 : 74));
      texture_pixels.push_back(255);
    }
  }

  u32 texture{};
  glCreateTextures(GL_TEXTURE_2D, 1, &texture);
  if (texture == 0) {
    spdlog::error("Failed to create OpenGL texture.");
    return EXIT_FAILURE;
  }
  const auto texture_guard{ScopeExit{[&] { glDeleteTextures(1, &texture); }}};

  glTextureStorage2D(texture, 1, GL_RGBA8, texture_size, texture_size);
  glTextureSubImage2D(texture, 0, 0, 0, texture_size, texture_size, GL_RGBA, GL_UNSIGNED_BYTE, texture_pixels.data());
  glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

  const i32 index_count{static_cast<i32>(cube_mesh_data.indices.size())};

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
    glBindTextureUnit(0, texture);

    lit_program->ReloadIfChanged();

    lit_program->Use();

    lit_program->SetUniform("u_texture", 0);
    lit_program->SetUniform("u_projection", projection);
    lit_program->SetUniform("u_view", view);
    lit_program->SetUniform("u_point_light.position", point_light.position);
    lit_program->SetUniform("u_point_light.color", point_light.color);
    lit_program->SetUniform("u_point_light.intensity", point_light.intensity);
    lit_program->SetUniform("u_point_light.kc", point_light.kc);
    lit_program->SetUniform("u_point_light.kl", point_light.kl);
    lit_program->SetUniform("u_point_light.kq", point_light.kq);
    lit_program->SetUniform("u_ambient_light", ambient_light);
    lit_program->SetUniform("u_camera_position", camera.position);

    lit_program->SetUniform("u_model", cube1_model);
    lit_program->SetUniform("u_normal_matrix", cube1_normal_matrix);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

    lit_program->SetUniform("u_model", cube2_model);
    lit_program->SetUniform("u_normal_matrix", cube2_normal_matrix);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

    lit_program->SetUniform("u_model", cube3_model);
    lit_program->SetUniform("u_normal_matrix", cube3_normal_matrix);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

    lit_program->SetUniform("u_model", cube4_model);
    lit_program->SetUniform("u_normal_matrix", cube4_normal_matrix);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, nullptr);

    SDL_GL_SwapWindow(window);
  }

  return EXIT_SUCCESS;
}
