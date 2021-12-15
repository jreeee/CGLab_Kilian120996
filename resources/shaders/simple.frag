#version 150

#define PI 3.14159265359
#define steps 4

#extension GL_OES_standard_derivatives : enable

in  vec3 Position;
in  vec3 Normal;
in  vec2 TexCoord;
out vec4 out_Color;

//in the renderPlanet
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

uniform sampler2D planetTexture;

//Normals
uniform bool HasNormal;
uniform sampler2D normalTexture;

//used in the keyCallback
uniform bool  Cel;
uniform bool  Solid;

vec3 calculateNormal(vec3 CameraPosition, vec3 Normal, vec2 TexCoord) {
  vec3 q0 = dFdx(CameraPosition.xyz);
  vec3 q1 = dFdy(CameraPosition.xyz);
  vec2 st0 = dFdx(TexCoord.st);
  vec2 st1 = dFdy(TexCoord.st);

  vec3 S = normalize(q0 * st1.t - q1 * st0.t);
  vec3 T = normalize(-q0 * st1.s + q1 * st0.s);
  vec3 N = normalize(Normal);

  vec3 mapN = texture2D(normalTexture, TexCoord).xyz * 2.0 - 1.0;
  //mapN.xy = normalScale * mapN.xy
  mat3 tsn = mat3(S, T, N); //?
  return normalize(tsn * mapN);
}

void main() {
  //ambient
  //beta(Y,X) = (lcol *lint)/(4PI (Y-X)^2)
  vec3 light_dir = LightPosition - Position;
  vec3 beta = (LightColor * LightIntensity) / 
              (4.0f * PI * pow(length(light_dir), 2.0f));
  vec3 ambient = AmbientIntensity * AmbientColor;

  //diffuse as described in the slides
  //Cd (l - n) rho/PI
  vec3 newNormal = (HasNormal) ? calculateNormal(CameraPosition, Normal, TexCoord) : Normal;
  vec3 main_col = (Solid) ? PlanetColor : vec3(texture(planetTexture, TexCoord));
  vec3 diffuse = main_col * dot((light_dir) , newNormal) * 
                  (PlanetRoughness / PI);

  //specular
  //Cs (h * v)^(4alpha) <- the v should be n
  vec3 l = normalize(light_dir);
  vec3 v = normalize(CameraPosition - Position);
  vec3 h = (l + v) / (length(l+v));
  vec3 specular = PlanetSpecular * pow(max(dot(h, normalize(newNormal)),
                                               0.0f), 4 * PlanetAlpha);

  vec3 phong = ambient + beta * (diffuse + specular);

  //celshading works on the finished phong
  if (Cel) {
    //first we multiply phong by the step-count and cast the result to integers
    //then we divide again to have the desired effect of low fidelity / cartoonish apperance
    //outline that is a bit darker than the planet color
    phong = (dot(v, normalize(newNormal)) < 0.4) ? PlanetColor - 0.3f : ceil(phong * steps)/steps;
  }
  out_Color = vec4(phong, 1.0f);

}
