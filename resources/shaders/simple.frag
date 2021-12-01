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
uniform float PlanetRoughness;
uniform vec3  PlanetSpecular;
uniform float PlanetAlpha;
uniform vec3  CameraPosition;

void main() {
  //ambient
  //beta(Y,X) = (lcol *lint)/(4PI (Y-X)^2)
  vec3 beta = (LightColor * LightIntensity) / 
              (4 * PI * pow(length(LightPosition - Position), 2.0f));
  vec3 ambient = AmbientIntensity * AmbientColor;

  //diffuse
  vec3 diffuse =  PlanetColor * dot((LightPosition - Position) , pass_Normal) * 
                  (PlanetRoughness / PI);
  //specular
  vec3 l = normalize(LightPosition - Position);
  vec3 v = normalize(CameraPosition - Position);
  vec3 h = (l + v) / (length(l+v));
  vec3 specular = PlanetSpecular * pow(dot(h, v), 4 * PlanetAlpha);
  //vec3 light = LightColor * LightIntensity;
  out_Color = vec4(ambient + beta * (diffuse + specular) , 1.0);
}
