#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include <array>
#include <cstddef>
#include <cstdlib>
#include <span>

#include "camera.hpp"
#include "directional_light.hpp"
#include "mesh_data.hpp"
#include "platform/buffer.hpp"
#include "platform/context.hpp"
#include "platform/program.hpp"
#include "platform/renderer.hpp"
#include "platform/texture.hpp"
#include "platform/window.hpp"
#include "point_light.hpp"
#include "primitives.hpp"
#include "scope_exit.hpp"
#include "types.hpp"
#include "vertex.hpp"

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int main() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    spdlog::error("SDL: failed to initialize: {}.", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto sdl_guard{ScopeExit{[] { SDL_Quit(); }}};

  auto window{platform::Window::Create("sandbox", 1'280, 720)};
  if (!window) {
    return EXIT_FAILURE;
  }

  auto context{platform::Context::Create(window)};
  if (!context) {
    return EXIT_FAILURE;
  }

  platform::Context::SetVSync(platform::Context::VSyncMode::Enabled);

  window->Show();

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  const MeshData mesh_data{primitives::MakeCube()};

  auto vbo{platform::Buffer::Create("vbo", std::as_bytes(std::span{mesh_data.vertices}))};
  if (!vbo) {
    return EXIT_FAILURE;
  }

  auto ebo{platform::Buffer::Create("ebo", std::as_bytes(std::span{mesh_data.elements}))};
  if (!ebo) {
    return EXIT_FAILURE;
  }

  u32 vao{};
  glCreateVertexArrays(1, &vao);
  if (vao == 0) {
    spdlog::error("OpenGL: failed to create vertex array.");
    return EXIT_FAILURE;
  }
  const auto vao_guard{ScopeExit{[&] { glDeleteVertexArrays(1, &vao); }}};

  glVertexArrayVertexBuffer(vao, 0, vbo->GetHandle(), 0, static_cast<i32>(sizeof(Vertex)));

  glVertexArrayElementBuffer(vao, ebo->GetHandle());

  glEnableVertexArrayAttrib(vao, 0);
  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, position)));
  glVertexArrayAttribBinding(vao, 0, 0);

  glEnableVertexArrayAttrib(vao, 1);
  glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, normal)));
  glVertexArrayAttribBinding(vao, 1, 0);

  glEnableVertexArrayAttrib(vao, 2);
  glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, uv)));
  glVertexArrayAttribBinding(vao, 2, 0);

  auto texture{platform::Texture::Create("assets/textures/checkerboard.png")};
  if (!texture.has_value()) {
    return EXIT_FAILURE;
  }

  auto blinn_phong_program{platform::Program::Create("assets/shaders/blinn_phong")};
  if (!blinn_phong_program.has_value()) {
    return EXIT_FAILURE;
  }

  auto unlit_program{platform::Program::Create("assets/shaders/unlit")};
  if (!unlit_program.has_value()) {
    return EXIT_FAILURE;
  }

  std::array<PointLight, 2> point_lights{PointLight{
                                             .position = {2.0F, 1.0F, 2.0F},
                                             .color = {0.0F, 1.0F, 0.0F},
                                             .intensity = 1.0F,
                                             .kc = 1.0F,
                                             .kl = 0.24F,
                                             .kq = 0.07F,
                                         },
                                         PointLight{
                                             .position = {-2.0F, 1.0F, -2.0F},
                                             .color = {0.0F, 0.0F, 1.0F},
                                             .intensity = 1.0F,
                                             .kc = 1.0F,
                                             .kl = 0.24F,
                                             .kq = 0.07F,
                                         }};

  const DirectionalLight directional_light{
      .direction = {0.0F, 1.0F, 0.0F},
      .color = {1.0F, 0.0F, 0.0F},
      .intensity = 1.0F,
  };

  const glm::vec3 ambient_light{0.1F, 0.1F, 0.1F};

  Camera camera{.position = {0.0F, 2.0F, 8.0F}};

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
          window->ToggleFullscreen();
        }
        break;
      default:
        break;
      }
    }
#pragma endregion ProcessEvents

    const auto [w, h]{window->GetSize()};

    platform::Renderer::SetViewport(0, 0, w, h);
    platform::Renderer::SetClearColor({0.0F, 0.0F, 0.0F, 1.0F});
    platform::Renderer::Clear();

    camera.aspect_ratio = static_cast<f32>(w) / static_cast<f32>(h);

    const glm::mat4 view{camera.ComputeViewMatrix()};
    const glm::mat4 projection{camera.ComputeProjectionMatrix()};

    glBindVertexArray(vao);

    texture->Bind(0);

    blinn_phong_program->Use();
    blinn_phong_program->SetUniform("u_projection", projection);
    blinn_phong_program->SetUniform("u_view", view);
    blinn_phong_program->SetUniform("u_texture", 0);
    blinn_phong_program->SetUniform("u_point_lights[0].position", point_lights.at(0).position);
    blinn_phong_program->SetUniform("u_point_lights[0].color", point_lights.at(0).color);
    blinn_phong_program->SetUniform("u_point_lights[0].intensity", point_lights.at(0).intensity);
    blinn_phong_program->SetUniform("u_point_lights[0].kc", point_lights.at(0).kc);
    blinn_phong_program->SetUniform("u_point_lights[0].kl", point_lights.at(0).kl);
    blinn_phong_program->SetUniform("u_point_lights[0].kq", point_lights.at(0).kq);
    blinn_phong_program->SetUniform("u_point_lights[1].position", point_lights.at(1).position);
    blinn_phong_program->SetUniform("u_point_lights[1].color", point_lights.at(1).color);
    blinn_phong_program->SetUniform("u_point_lights[1].intensity", point_lights.at(1).intensity);
    blinn_phong_program->SetUniform("u_point_lights[1].kc", point_lights.at(1).kc);
    blinn_phong_program->SetUniform("u_point_lights[1].kl", point_lights.at(1).kl);
    blinn_phong_program->SetUniform("u_point_lights[1].kq", point_lights.at(1).kq);
    blinn_phong_program->SetUniform("u_directional_light.direction", directional_light.direction);
    blinn_phong_program->SetUniform("u_directional_light.color", directional_light.color);
    blinn_phong_program->SetUniform("u_directional_light.intensity", directional_light.intensity);
    blinn_phong_program->SetUniform("u_ambient_light", ambient_light);
    blinn_phong_program->SetUniform("u_camera_position", camera.position);

    {
      glm::mat4 model{glm::mat4{1.0F}};
      model = glm::translate(model, {0.0F, -0.5F, 0.0F});
      model = glm::scale(model, {8.0F, 1.0F, 8.0F});
      blinn_phong_program->SetUniform("u_model", model);
      blinn_phong_program->SetUniform("u_normal_matrix", glm::transpose(glm::inverse(glm::mat3{model})));
      glDrawElements(GL_TRIANGLES, static_cast<i32>(mesh_data.elements.size()), GL_UNSIGNED_INT, nullptr);
    }

    unlit_program->Use();
    unlit_program->SetUniform("u_projection", projection);
    unlit_program->SetUniform("u_view", view);

    for (usize i{}; i < point_lights.size(); ++i) {
      glm::mat4 model{glm::mat4{1.0F}};
      model = glm::translate(model, point_lights.at(i).position);
      unlit_program->SetUniform("u_model", model);
      unlit_program->SetUniform("u_color", point_lights.at(i).color);
      glDrawElements(GL_TRIANGLES, static_cast<i32>(mesh_data.elements.size()), GL_UNSIGNED_INT, nullptr);
    }

    platform::Context::SwapBuffers(window);
  }

  return EXIT_SUCCESS;
}
