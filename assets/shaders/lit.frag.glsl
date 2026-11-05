#version 460 core

struct Light {
  vec3 position;
  vec3 color;
  float intensity;
};

struct Material {
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

uniform vec3 u_ambient_light;
uniform Light u_light;
uniform Material u_material;
uniform vec3 u_camera_position;

in vec3 v_world_position;
in vec3 v_world_normal;

out vec4 f_color;

void main() {
  const vec3 world_normal = normalize(v_world_normal);
  const vec3 light_direction = normalize(u_light.position - v_world_position);
  const vec3 view_direction = normalize(u_camera_position - v_world_position);
  const vec3 halfway_direction = normalize(light_direction + view_direction);

  const float diffuse_factor = max(dot(world_normal, light_direction), 0.0F);
  const float specular_factor = pow(max(dot(world_normal, halfway_direction), 0.0F), u_material.shininess);
  const vec3 radiance = u_light.color * u_light.intensity;

  const vec3 ambient = u_ambient_light * u_material.diffuse;
  const vec3 diffuse = diffuse_factor * radiance * u_material.diffuse;
  const vec3 specular = specular_factor * radiance * u_material.specular;

  f_color = vec4(ambient + diffuse + specular, 1.0F);
}
