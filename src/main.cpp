#include "camera.hpp"
#include "camera_controller.hpp"
#include "core/scope_exit.hpp"
#include "light.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "primitives.hpp"
#include "program.hpp"
#include "vertex.hpp"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl3.h>
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

namespace {

[[nodiscard]] std::expected<Program, std::string> LoadProgram(std::string_view name) {
  const std::filesystem::path directory{"assets/shaders"};
  const std::string base{name};

  return Program::CreateFromFiles(directory / (base + ".vert.glsl"), directory / (base + ".frag.glsl"));
}

[[nodiscard]] bool IsEventWantedByImGui(const SDL_Event &event, const ImGuiIO &io) {
  switch (event.type) {
  case SDL_EVENT_MOUSE_MOTION:
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP:
  case SDL_EVENT_MOUSE_WHEEL:
    return io.WantCaptureMouse;
  case SDL_EVENT_KEY_DOWN:
  case SDL_EVENT_KEY_UP:
  case SDL_EVENT_TEXT_EDITING:
  case SDL_EVENT_TEXT_INPUT:
  case SDL_EVENT_TEXT_EDITING_CANDIDATES:
    return io.WantCaptureKeyboard;
  default:
    return false;
  }
}

} // namespace

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
int main() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
    spdlog::error("SDL_Init failed: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto sdl_guard{ScopeExit{[] { SDL_Quit(); }}};

#pragma region SetOpenGLWindowAttributes
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#pragma endregion SetOpenglWindowAttributes

  const char *title{"sandbox"};
  const float main_scale{SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay())};
  const std::int32_t w{static_cast<std::int32_t>(1'280 * main_scale)};
  const std::int32_t h{static_cast<std::int32_t>(720 * main_scale)};
  // clang-format off
  const SDL_WindowFlags flags{
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY
  };
  // clang-format on
  SDL_Window *sdl_window{SDL_CreateWindow(title, w, h, flags)};
  if (sdl_window == nullptr) {
    spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto sdl_window_guard{ScopeExit{[sdl_window] { SDL_DestroyWindow(sdl_window); }}};

  const SDL_GLContext sdl_gl_context{SDL_GL_CreateContext(sdl_window)};
  if (sdl_gl_context == nullptr) {
    spdlog::error("SDL_GL_CreateContext failed: {}", SDL_GetError());
    return EXIT_FAILURE;
  }
  const auto sdl_gl_context_guard{ScopeExit{[sdl_gl_context] { SDL_GL_DestroyContext(sdl_gl_context); }}};

  SDL_GL_SetSwapInterval(1);

  SDL_SetWindowPosition(sdl_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
  SDL_ShowWindow(sdl_window);

  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (gladLoadGL(reinterpret_cast<GLADloadfunc>(SDL_GL_GetProcAddress)) == 0) {
    spdlog::error("gladLoadGL failed");
    return EXIT_FAILURE;
  }

#pragma region LogOpenGLContextInfo
  // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
  spdlog::info("OpenGL Version: {}", reinterpret_cast<const char *>(glGetString(GL_VERSION)));
  spdlog::info("GLSL Version: {}", reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
  spdlog::info("Vendor: {}", reinterpret_cast<const char *>(glGetString(GL_VENDOR)));
  spdlog::info("Renderer: {}", reinterpret_cast<const char *>(glGetString(GL_RENDERER)));
  // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
#pragma endregion LogOpenGLContextInfo

  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

#pragma region InitImGui
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  const auto imgui_context_guard{ScopeExit{[] { ImGui::DestroyContext(); }}};

  ImGuiIO &io{ImGui::GetIO()};
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();

  ImGuiStyle &style{ImGui::GetStyle()};
  style.ScaleAllSizes(main_scale);
  style.FontScaleDpi = main_scale;
  io.ConfigDpiScaleFonts = true;
  io.ConfigDpiScaleViewports = true;

  if (static_cast<bool>(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) {
    style.WindowRounding = 0.0F;
    style.Colors[ImGuiCol_WindowBg].w = 1.0F;
  }

  ImGui_ImplSDL3_InitForOpenGL(sdl_window, sdl_gl_context);
  const auto imgui_sdl3_guard{ScopeExit{[] { ImGui_ImplSDL3_Shutdown(); }}};

  ImGui_ImplOpenGL3_Init();
  const auto imgui_opengl3_guard{ScopeExit{[] { ImGui_ImplOpenGL3_Shutdown(); }}};
#pragma endregion InitImGui

  const glm::vec3 ambient_light{0.1F, 0.1F, 0.1F};

  const Light light{.position = {2.0F, 4.0F, 3.0F}, .color = {1.0F, 1.0F, 1.0F}, .intensity = 1.0F};

  const Material material1{.diffuse = {1.0F, 0.0F, 0.0F}, .specular = {1.0F, 0.5F, 0.5F}, .shininess = 64.0F};
  const Material material2{.diffuse = {0.0F, 1.0F, 0.0F}, .specular = {0.5F, 1.0F, 0.5F}, .shininess = 32.0F};
  const Material material3{.diffuse = {0.0F, 0.0F, 1.0F}, .specular = {0.5F, 0.5F, 1.0F}, .shininess = 16.0F};

  const Mesh cube{MakeCube()};

#pragma region CreateCubeBuffers
  std::uint32_t vbo{};
  glCreateBuffers(1, &vbo);
  const auto vbo_guard{ScopeExit{[vbo] { glDeleteBuffers(1, &vbo); }}};

  std::uint32_t ebo{};
  glCreateBuffers(1, &ebo);
  const auto ebo_guard{ScopeExit{[ebo] { glDeleteBuffers(1, &ebo); }}};

  std::uint32_t vao{};
  glCreateVertexArrays(1, &vao);
  const auto vao_guard{ScopeExit{[vao] { glDeleteVertexArrays(1, &vao); }}};

  glNamedBufferStorage(
      vbo, static_cast<std::ptrdiff_t>(cube.vertices.size() * sizeof(Vertex)), cube.vertices.data(), 0);
  glNamedBufferStorage(
      ebo, static_cast<std::ptrdiff_t>(cube.indices.size() * sizeof(std::uint32_t)), cube.indices.data(), 0);

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
#pragma endregion CreateCubeBuffers

  const auto index_count{static_cast<std::int32_t>(cube.indices.size())};

  auto lit_program{LoadProgram("lit")};
  if (!lit_program.has_value()) {
    spdlog::error("{}", lit_program.error());
    return EXIT_FAILURE;
  }

  auto unlit_program{LoadProgram("unlit")};
  if (!unlit_program.has_value()) {
    spdlog::error("{}", unlit_program.error());
    return EXIT_FAILURE;
  }

  Camera camera{{0.0F, 2.0F, 10.0F}, {}};
  CameraController camera_controller{camera};

  std::uint64_t last_frame_ticks{SDL_GetTicks()};

  bool done{false};

  while (!done) {
    const std::uint64_t current_frame_ticks{SDL_GetTicks()};
    const float delta_time{static_cast<float>(current_frame_ticks - last_frame_ticks) / 1'000.0F};
    last_frame_ticks = current_frame_ticks;

    SDL_Event event{};
#pragma region ProcessEvents
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (IsEventWantedByImGui(event, io)) {
        continue;
      }

      switch (event.type) {
      case SDL_EVENT_QUIT:
        done = true;
        break;
      case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        if (event.window.windowID == SDL_GetWindowID(sdl_window)) {
          done = true;
        }
        break;
      case SDL_EVENT_WINDOW_FOCUS_LOST:
        SDL_SetWindowRelativeMouseMode(sdl_window, false);
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (event.button.button == SDL_BUTTON_RIGHT) {
          SDL_SetWindowRelativeMouseMode(sdl_window, true);
        }
        break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
        if (event.button.button == SDL_BUTTON_RIGHT) {
          SDL_SetWindowRelativeMouseMode(sdl_window, false);
        }
        break;
      case SDL_EVENT_MOUSE_MOTION:
        if (SDL_GetWindowRelativeMouseMode(sdl_window)) {
          camera_controller.Look(event.motion.xrel, event.motion.yrel);
        }
        break;
      case SDL_EVENT_KEY_DOWN:
        if (!event.key.repeat && (event.key.key == SDLK_RETURN) && static_cast<bool>(event.key.mod & SDL_KMOD_ALT)) {
          const bool is_fullscreen{static_cast<bool>(SDL_GetWindowFlags(sdl_window) & SDL_WINDOW_FULLSCREEN)};
          SDL_SetWindowFullscreen(sdl_window, !is_fullscreen);
        }
        break;
      default:
        break;
      }
    }
#pragma endregion ProcessEvents

    if (static_cast<bool>(SDL_GetWindowFlags(sdl_window) & SDL_WINDOW_MINIMIZED)) {
      SDL_Delay(10);
      continue;
    }

    if (!io.WantCaptureKeyboard && SDL_GetWindowRelativeMouseMode(sdl_window)) {
      const bool *keyboard_state{SDL_GetKeyboardState(nullptr)};
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      const float speed_multiplier{keyboard_state[SDL_SCANCODE_LSHIFT] ? 2.5F : 1.0F};
      CameraController::Direction direction{};
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      if (keyboard_state[SDL_SCANCODE_W]) {
        direction.forward += 1.0F;
      }
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      if (keyboard_state[SDL_SCANCODE_S]) {
        direction.forward -= 1.0F;
      }
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      if (keyboard_state[SDL_SCANCODE_A]) {
        direction.right -= 1.0F;
      }
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      if (keyboard_state[SDL_SCANCODE_D]) {
        direction.right += 1.0F;
      }
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      if (keyboard_state[SDL_SCANCODE_SPACE]) {
        direction.up += 1.0F;
      }
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      if (keyboard_state[SDL_SCANCODE_LCTRL]) {
        direction.up -= 1.0F;
      }
      camera_controller.Move(direction, delta_time, speed_multiplier);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::Render();

    std::int32_t framebuffer_w{};
    std::int32_t framebuffer_h{};
    SDL_GetWindowSizeInPixels(sdl_window, &framebuffer_w, &framebuffer_h);
    glViewport(0, 0, framebuffer_w, framebuffer_h);
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float framebuffer_a{static_cast<float>(framebuffer_w) / static_cast<float>(framebuffer_h)};
    const glm::mat4 projection{camera.ComputeProjectionMatrix(framebuffer_a)};
    const glm::mat4 view{camera.ComputeViewMatrix()};

    glBindVertexArray(vao);

    unlit_program->Use();
    unlit_program->SetUniform("u_projection", projection);
    unlit_program->SetUniform("u_view", view);

#pragma region DrawLight
    glm::mat4 light_model{glm::mat4{1.0F}};
    light_model = glm::translate(light_model, light.position);
    unlit_program->SetUniform("u_model", light_model);
    unlit_program->SetUniform("u_color", light.color);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, static_cast<void *>(nullptr));
#pragma endregion DrawLight

    lit_program->Use();
    lit_program->SetUniform("u_projection", projection);
    lit_program->SetUniform("u_view", view);
    lit_program->SetUniform("u_ambient_light", ambient_light);
    lit_program->SetUniform("u_light.position", light.position);
    lit_program->SetUniform("u_light.color", light.color);
    lit_program->SetUniform("u_light.intensity", light.intensity);
    lit_program->SetUniform("u_view_position", camera.GetPosition());

#pragma region DrawBox1
    glm::mat4 model1{glm::mat4{1.0F}};
    model1 = glm::translate(model1, {-1.0F, 2.0F, -1.0F});
    lit_program->SetUniform("u_model", model1);
    lit_program->SetUniform("u_material.diffuse", material1.diffuse);
    lit_program->SetUniform("u_material.specular", material1.specular);
    lit_program->SetUniform("u_material.shininess", material1.shininess);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, static_cast<void *>(nullptr));
#pragma endregion DrawBox1

#pragma region DrawBox2
    glm::mat4 model2{glm::mat4{1.0F}};
    model2 = glm::translate(model2, {1.0F, 2.0F, 1.0F});
    lit_program->SetUniform("u_model", model2);
    lit_program->SetUniform("u_material.diffuse", material2.diffuse);
    lit_program->SetUniform("u_material.specular", material2.specular);
    lit_program->SetUniform("u_material.shininess", material2.shininess);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, static_cast<void *>(nullptr));
#pragma endregion DrawBox2

#pragma region DrawBox3
    glm::mat4 model3{glm::mat4{1.0F}};
    model3 = glm::scale(model3, {4.0F, 1.0F, 4.0F});
    lit_program->SetUniform("u_model", model3);
    lit_program->SetUniform("u_material.diffuse", material3.diffuse);
    lit_program->SetUniform("u_material.specular", material3.specular);
    lit_program->SetUniform("u_material.shininess", material3.shininess);
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, static_cast<void *>(nullptr));
#pragma endregion DrawBox3

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (static_cast<bool>(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) {
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(sdl_window, sdl_gl_context);
    }

    SDL_GL_SwapWindow(sdl_window);
  }

  return EXIT_SUCCESS;
}
