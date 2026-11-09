#include "camera.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "point_light.hpp"
#include "primitives.hpp"
#include "scope_exit.hpp"
#include "shader.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_video.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include <cstdint>
#include <cstdlib>

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
  const float main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
  const std::int32_t w{static_cast<std::int32_t>(1'280 * main_scale)};
  const std::int32_t h{static_cast<std::int32_t>(720 * main_scale)};
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

  const PointLight point_light{
      .position = {0.0F, 0.0F, 2.0F},
      .ambient = {0.2F, 0.2F, 0.2F},
      .diffuse = {1.0F, 1.0F, 1.0F},
      .specular = {1.0F, 1.0F, 1.0F},
      .kc = 1.0F,
      .kl = 0.09F,
      .kq = 0.032F,
  };

#pragma region Materials
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
#pragma endregion Materials

#pragma region Models
  glm::mat4 cube1_model{glm::mat4{1.0F}};
  glm::mat4 cube2_model{glm::mat4{1.0F}};
  glm::mat4 cube3_model{glm::mat4{1.0F}};
  glm::mat4 cube4_model{glm::mat4{1.0F}};

  cube1_model = glm::translate(cube1_model, {-1.0F, 1.0F, 0.0F});
  cube2_model = glm::translate(cube2_model, {1.0F, 1.0F, 0.0F});
  cube3_model = glm::translate(cube3_model, {-1.0F, -1.0F, 0.0F});
  cube4_model = glm::translate(cube4_model, {1.0F, -1.0F, 0.0F});
#pragma endregion Models

  const glm::mat3 cube1_normal_matrix{glm::transpose(glm::inverse(glm::mat3{cube1_model}))};
  const glm::mat3 cube2_normal_matrix{glm::transpose(glm::inverse(glm::mat3{cube2_model}))};
  const glm::mat3 cube3_normal_matrix{glm::transpose(glm::inverse(glm::mat3{cube3_model}))};
  const glm::mat3 cube4_normal_matrix{glm::transpose(glm::inverse(glm::mat3{cube4_model}))};

  const auto cube_mesh{Mesh::Create(primitives::MakeCube())};
  if (!cube_mesh.has_value()) {
    return EXIT_FAILURE;
  }

  auto lit_shader{Shader::Create("assets/shaders/lit")};
  if (!lit_shader.has_value()) {
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
    const glm::mat4 view{camera.ComputeViewMatrix()};
    const glm::mat4 projection{camera.ComputeProjectionMatrix(framebuffer_a)};

    lit_shader->Use();
    lit_shader->SetUniform("u_projection", projection);
    lit_shader->SetUniform("u_view", view);
    lit_shader->SetUniform("u_point_light.position", point_light.position);
    lit_shader->SetUniform("u_point_light.ambient", point_light.ambient);
    lit_shader->SetUniform("u_point_light.diffuse", point_light.diffuse);
    lit_shader->SetUniform("u_point_light.specular", point_light.specular);
    lit_shader->SetUniform("u_point_light.kc", point_light.kc);
    lit_shader->SetUniform("u_point_light.kl", point_light.kl);
    lit_shader->SetUniform("u_point_light.kq", point_light.kq);
    lit_shader->SetUniform("u_camera_position", camera.position);

#pragma region DrawCube1
    lit_shader->SetUniform("u_model", cube1_model);
    lit_shader->SetUniform("u_normal_matrix", cube1_normal_matrix);
    lit_shader->SetUniform("u_material.ambient", cube1_material.ambient);
    lit_shader->SetUniform("u_material.diffuse", cube1_material.diffuse);
    lit_shader->SetUniform("u_material.specular", cube1_material.specular);
    lit_shader->SetUniform("u_material.shininess", cube1_material.shininess);
    cube_mesh->Draw();
#pragma endregion DrawCube1

#pragma region DrawCube2
    lit_shader->SetUniform("u_model", cube2_model);
    lit_shader->SetUniform("u_normal_matrix", cube2_normal_matrix);
    lit_shader->SetUniform("u_material.ambient", cube2_material.ambient);
    lit_shader->SetUniform("u_material.diffuse", cube2_material.diffuse);
    lit_shader->SetUniform("u_material.specular", cube2_material.specular);
    lit_shader->SetUniform("u_material.shininess", cube2_material.shininess);
    cube_mesh->Draw();
#pragma endregion DrawCube2

#pragma region DrawCube3
    lit_shader->SetUniform("u_model", cube3_model);
    lit_shader->SetUniform("u_normal_matrix", cube3_normal_matrix);
    lit_shader->SetUniform("u_material.ambient", cube3_material.ambient);
    lit_shader->SetUniform("u_material.diffuse", cube3_material.diffuse);
    lit_shader->SetUniform("u_material.specular", cube3_material.specular);
    lit_shader->SetUniform("u_material.shininess", cube3_material.shininess);
    cube_mesh->Draw();
#pragma endregion DrawCube3

#pragma region DrawCube4
    lit_shader->SetUniform("u_model", cube4_model);
    lit_shader->SetUniform("u_normal_matrix", cube4_normal_matrix);
    lit_shader->SetUniform("u_material.ambient", cube4_material.ambient);
    lit_shader->SetUniform("u_material.diffuse", cube4_material.diffuse);
    lit_shader->SetUniform("u_material.specular", cube4_material.specular);
    lit_shader->SetUniform("u_material.shininess", cube4_material.shininess);
    cube_mesh->Draw();
#pragma endregion DrawCube4

    SDL_GL_SwapWindow(window);
  }

  return EXIT_SUCCESS;
}
