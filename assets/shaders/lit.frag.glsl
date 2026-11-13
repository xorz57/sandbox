#version 460 core

struct PointLight {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float kc;
  float kl;
  float kq;
};

uniform PointLight u_point_light;
uniform sampler2D u_texture;
uniform vec3 u_camera_position;

in vec3 v_world_position;
in vec3 v_world_normal;
in vec2 v_uv;

out vec4 f_color;

void main() {
  const vec3 world_normal = normalize(v_world_normal);
  const vec3 albedo = texture(u_texture, v_uv).rgb;

  const vec3 light_direction = normalize(u_point_light.position - v_world_position);
  const vec3 view_direction = normalize(u_camera_position - v_world_position);
  const vec3 halfway_direction = normalize(light_direction + view_direction);

  const float distance = length(u_point_light.position - v_world_position);
  const float attenuation = 1.0F / (u_point_light.kc + u_point_light.kl * distance + u_point_light.kq * (distance * distance));

  const float diffuse_factor = max(dot(world_normal, light_direction), 0.0F);
  const float specular_factor = pow(max(dot(world_normal, halfway_direction), 0.0F), 32.0F);

  const vec3 ambient = u_point_light.ambient * albedo;
  const vec3 diffuse = u_point_light.diffuse * diffuse_factor * albedo;
  const vec3 specular = u_point_light.specular * specular_factor;

  f_color = vec4(attenuation * (ambient + diffuse + specular), 1.0F);
}
