#version 460 core

struct PointLight {
  vec3 position;
  vec3 color;
  float intensity;
  float kc;
  float kl;
  float kq;
};

struct DirectionalLight {
  vec3 direction;
  vec3 color;
  float intensity;
};

uniform sampler2D u_texture;
uniform PointLight u_point_light;
uniform DirectionalLight u_directional_light;
uniform vec3 u_ambient_light;
uniform vec3 u_camera_position;

in vec3 v_world_position;
in vec3 v_world_normal;
in vec2 v_uv;

out vec4 f_color;

vec3 calculate_directional_light(DirectionalLight light, vec3 normal, vec3 view_direction, vec3 albedo) {
  const vec3 light_direction = normalize(-light.direction);
  const vec3 halfway_direction = normalize(light_direction + view_direction);

  const float diffuse_factor = max(dot(normal, light_direction), 0.0F);
  const float specular_factor = (diffuse_factor > 0.0F) ? pow(max(dot(normal, halfway_direction), 0.0F), 32.0F) : 0.0F;

  const vec3 radiance = light.color * light.intensity;

  const vec3 diffuse = radiance * diffuse_factor * albedo;
  const vec3 specular = radiance * specular_factor;

  return diffuse + specular;
}

vec3 calculate_point_light(PointLight light, vec3 normal, vec3 world_position, vec3 view_direction, vec3 albedo) {
  const vec3 light_direction = normalize(light.position - world_position);
  const vec3 halfway_direction = normalize(light_direction + view_direction);

  const float distance = length(light.position - world_position);

  const float attenuation = 1.0F / (light.kc + light.kl * distance + light.kq * distance * distance);

  const float diffuse_factor = max(dot(normal, light_direction), 0.0F);
  const float specular_factor = (diffuse_factor > 0.0F) ? pow(max(dot(normal, halfway_direction), 0.0F), 32.0F) : 0.0F;

  const vec3 radiance = light.color * light.intensity * attenuation;

  const vec3 diffuse = radiance * diffuse_factor * albedo;
  const vec3 specular = radiance * specular_factor;

  return diffuse + specular;
}

void main() {
  const vec3 world_normal = normalize(v_world_normal);

  const vec3 view_direction = normalize(u_camera_position - v_world_position);

  const vec3 albedo = texture(u_texture, v_uv).rgb;

  vec3 color = u_ambient_light * albedo;

  color += calculate_directional_light(u_directional_light, world_normal, view_direction, albedo);
  color += calculate_point_light(u_point_light, world_normal, v_world_position, view_direction, albedo);

  f_color = vec4(color, 1.0F);
}
