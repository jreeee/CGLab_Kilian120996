#version 150
#define PI 3.14159265359

in  vec3 Position;
in  vec3 pass_Normal;
out vec4 out_Color;

uniform vec3  LightPosition;
uniform vec3  LightColor;
uniform float LightIntensity;
uniform vec3  AmbientColor;
uniform float AmbientIntensity;
uniform vec3  PlanetColor;

void main() {
  //beta(Y,X) = (lcol *lint)/(4PI (Y-X)^2)
  vec3 beta = (LightColor * LightIntensity) / (4 * PI * pow(length(LightPosition - Position), 2));
  vec3 ambient = AmbientIntensity * AmbientColor;
  vec3 light = LightColor * LightIntensity;
  out_Color = vec4(ambient + beta * PlanetColor, 1.0);
}
