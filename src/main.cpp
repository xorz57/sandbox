#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_timer.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdlib>
#include <span>

#include "camera.hpp"
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

  auto context{platform::Context::Create(window->GetWindow())};
  if (!context) {
    return EXIT_FAILURE;
  }

  platform::Context::SetVSync(platform::Context::VSyncMode::Enabled);

  window->Show();

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  const MeshData mesh_data{primitives::MakeCube()};

  const auto vertices{std::as_bytes(std::span{mesh_data.vertices})};
  platform::Buffer vbo{"Vertex Buffer", vertices.size()};
  vbo.Update(vertices, 0);

  const auto indices{std::as_bytes(std::span{mesh_data.indices})};
  platform::Buffer ebo{"Element Buffer", indices.size()};
  ebo.Update(indices, 0);

  u32 vao{};
  glCreateVertexArrays(1, &vao);
  if (vao == 0) {
    spdlog::error("OpenGL: failed to create vertex array.");
    return EXIT_FAILURE;
  }
  const auto vao_guard{ScopeExit{[&] { glDeleteVertexArrays(1, &vao); }}};

  glVertexArrayVertexBuffer(vao, 0, vbo.GetHandle(), 0, static_cast<i32>(sizeof(Vertex)));
  glVertexArrayElementBuffer(vao, ebo.GetHandle());

  glEnableVertexArrayAttrib(vao, 0);
  glEnableVertexArrayAttrib(vao, 1);
  glEnableVertexArrayAttrib(vao, 2);

  glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, position)));
  glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, normal)));
  glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, static_cast<u32>(offsetof(Vertex, uv)));

  glVertexArrayAttribBinding(vao, 0, 0);
  glVertexArrayAttribBinding(vao, 1, 0);
  glVertexArrayAttribBinding(vao, 2, 0);

  auto texture{platform::Texture::Create("assets/textures/checkerboard.png")};
  if (!texture.has_value()) {
    return EXIT_FAILURE;
  }

  auto program{platform::Program::Create("assets/shaders/lit")};
  if (!program.has_value()) {
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

    const f32 aspect_ratio{static_cast<f32>(w) / static_cast<f32>(h)};

    const Camera camera{.position = {0.0F, 0.0F, 4.0F}, .aspect_ratio = aspect_ratio};

    const glm::mat4 view{camera.ComputeViewMatrix()};
    const glm::mat4 projection{camera.ComputeProjectionMatrix()};

    glBindVertexArray(vao);

    texture->Bind(0);

    program->ReloadIfChanged();

    program->Use();

    program->SetUniform("u_projection", projection);
    program->SetUniform("u_view", view);

    program->SetUniform("u_texture", 0);

    program->SetUniform("u_point_light.position", point_light.position);
    program->SetUniform("u_point_light.color", point_light.color);
    program->SetUniform("u_point_light.intensity", point_light.intensity);
    program->SetUniform("u_point_light.kc", point_light.kc);
    program->SetUniform("u_point_light.kl", point_light.kl);
    program->SetUniform("u_point_light.kq", point_light.kq);

    program->SetUniform("u_ambient_light", ambient_light);

    program->SetUniform("u_camera_position", camera.position);

    const f32 time{static_cast<f32>(SDL_GetTicks()) / SDL_MS_PER_SECOND};

    {
      glm::mat4 model{glm::mat4{1.0F}};
      model = glm::translate(model, {-1.0F, 1.0F, 0.0F});
      model = glm::rotate(model, time * 1.0F, {1.0F, 1.0F, 1.0F});
      program->SetUniform("u_model", model);
      program->SetUniform("u_normal_matrix", glm::transpose(glm::inverse(glm::mat3{model})));
      glDrawElements(GL_TRIANGLES, static_cast<i32>(mesh_data.indices.size()), GL_UNSIGNED_INT, nullptr);
    }

    {
      glm::mat4 model{glm::mat4{1.0F}};
      model = glm::translate(model, {1.0F, 1.0F, 0.0F});
      model = glm::rotate(model, time * 1.0F, {1.0F, 1.0F, 1.0F});
      program->SetUniform("u_model", model);
      program->SetUniform("u_normal_matrix", glm::transpose(glm::inverse(glm::mat3{model})));
      glDrawElements(GL_TRIANGLES, static_cast<i32>(mesh_data.indices.size()), GL_UNSIGNED_INT, nullptr);
    }

    {
      glm::mat4 model{glm::mat4{1.0F}};
      model = glm::translate(model, {-1.0F, -1.0F, 0.0F});
      model = glm::rotate(model, time * 1.0F, {1.0F, 1.0F, 1.0F});
      program->SetUniform("u_model", model);
      program->SetUniform("u_normal_matrix", glm::transpose(glm::inverse(glm::mat3{model})));
      glDrawElements(GL_TRIANGLES, static_cast<i32>(mesh_data.indices.size()), GL_UNSIGNED_INT, nullptr);
    }

    {
      glm::mat4 model{glm::mat4{1.0F}};
      model = glm::translate(model, {1.0F, -1.0F, 0.0F});
      model = glm::rotate(model, time * 1.0F, {1.0F, 1.0F, 1.0F});
      program->SetUniform("u_model", model);
      program->SetUniform("u_normal_matrix", glm::transpose(glm::inverse(glm::mat3{model})));
      glDrawElements(GL_TRIANGLES, static_cast<i32>(mesh_data.indices.size()), GL_UNSIGNED_INT, nullptr);
    }

    window->SwapBuffers();
  }

  return EXIT_SUCCESS;
}
