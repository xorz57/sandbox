#include "primitives.hpp"

#include "mesh_data.hpp"

MeshData primitives::MakeCube() {
  // clang-format off
  return MeshData{
    .vertices = {
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
    },
    .indices = {
     0,  1,  2,  2,  3,  0,  // +z Front
     4,  5,  6,  6,  7,  4,  // -z Back
     8,  9, 10, 10, 11,  8,  // -x Left
    12, 13, 14, 14, 15, 12,  // +x Right
    16, 17, 18, 18, 19, 16,  // +y Top
    20, 21, 22, 22, 23, 20,  // -y Bottom
    },
  };
  // clang-format on
}
