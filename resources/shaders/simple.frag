#version 150
#define PI 3.14159265359

in  vec3 pass_Normal;
out vec4 out_Color;

uniform vec3 l_color;
uniform float l_intensity;
uniform vec3 a_color;
uniform float a_intensity;
uniform vec3 p_color;

void main() {
  vec3 ambient = a_intensity * a_color;
  vec3 light = l_color * l_intensity;
  out_Color = vec4(ambient + light * p_color, 1.0);
}
