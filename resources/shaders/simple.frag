#version 150

#define PI 3.14159265359
#define steps 4
#define normal_factor 1

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

vec3 calcNormal(vec3 Position, vec3 Normal, vec2 TexCoord) {
  //getting the derivative of the vertex and texture
  vec3 q0 = dFdx(Position.xyz);
  vec3 q1 = dFdy(Position.xyz);
  vec2 st0 = dFdx(TexCoord.st);
  vec2 st1 = dFdy(TexCoord.st);

  //setting the tangent space via S, T and N
  vec3 S = normalize(q0 * st1.t - q1 * st0.t);
  vec3 T = normalize(-q0 * st1.s + q1 * st0.s);
  vec3 N = normalize(Normal);
  //saving it as a matrix
  mat3 stn = mat3(S, T, N);

  //getting the point on the normal map, since the rgb values can only
  //be between 0 and 1 we have to subtract 1 for it to work
  vec3 mapN = (texture2D(normalTexture, TexCoord).xyz * 2.0 - 1.0) * normal_factor;
  //combining both into a modified normal that works in the world space
  return normalize(stn * mapN);
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
  vec3 newNormal = (HasNormal) ? calcNormal(Position, Normal, TexCoord) : Normal;
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

  //checking if the object is the sun
  //technically we could skip most of the steps above 
  //however this still enables celshading and is rather convenient
  vec3 phong = (AmbientIntensity == LightIntensity) ? 
                main_col : ambient + beta * (diffuse + specular);
  
  //celshading works on the finished phong
  //first we multiply phong by the step-count and cast the result to integers
  //then we divide again to have the desired effect of low fidelity / cartoonish apperance
  //outline that is a bit darker than the orbit color
  if (Cel) {
    phong = (dot(v, normalize(newNormal)) < 0.4) ? PlanetColor - 0.3f : ceil(phong * steps)/steps;
  }
  out_Color = vec4(phong, 1.0f);
}
