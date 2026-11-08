#version 460 core

struct PointLight {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

uniform PointLight u_point_light;
uniform Material u_material;
uniform vec3 u_camera_position;

in vec3 v_world_position;
in vec3 v_world_normal;

out vec4 f_color;

void main() {
  const vec3 world_normal = normalize(v_world_normal);

  const vec3 light_direction = normalize(u_point_light.position - v_world_position);
  const vec3 view_direction = normalize(u_camera_position - v_world_position);
  const vec3 halfway_direction = normalize(light_direction + view_direction);

  const float diffuse_factor = max(dot(world_normal, light_direction), 0.0F);
  const float specular_factor = pow(max(dot(world_normal, halfway_direction), 0.0F), u_material.shininess);

  const vec3 ambient = u_point_light.ambient * u_material.ambient;
  const vec3 diffuse = u_point_light.diffuse * diffuse_factor * u_material.diffuse;
  const vec3 specular = u_point_light.specular * specular_factor * u_material.specular;

  f_color = vec4(ambient + diffuse + specular, 1.0F);
}
