#version 150
#define PI 3.14159265359
#define steps 4


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
uniform sampler2D ourTexture;

//used in the keyCallback
uniform bool  Cel;

void main() {
  //ambient
  //beta(Y,X) = (lcol *lint)/(4PI (Y-X)^2)
  vec3 light_dir = LightPosition - Position;
  vec3 beta = (LightColor * LightIntensity) / 
              (4.0f * PI * pow(length(light_dir), 2.0f));
  vec3 ambient = AmbientIntensity * AmbientColor;

  //diffuse as described in the slides
  //Cd (l - n) rho/PI
  vec3 diffuse =  PlanetColor * dot((light_dir) , Normal) * 
                  (PlanetRoughness / PI);

  //specular
  //Cs (h * v)^(4alpha) <- the v should be n
  vec3 l = normalize(light_dir);
  vec3 v = normalize(CameraPosition - Position);
  vec3 h = (l + v) / (length(l+v));
  vec3 specular = PlanetSpecular * pow(max(dot(h, normalize(Normal)),
                                               0.0f), 4 * PlanetAlpha);

  vec3 phong = ambient + beta * (diffuse + specular);

  //celshading works on the finished phong
  if (Cel) {
    //first we multiply phong by the step-count and cast the result to integers
    //then we divide again to have the desired effect of low fidelity / cartoonish apperance
    phong = ceil(phong * steps)/steps;
    if (dot(v, normalize(Normal)) < 0.4) {
      //outline that is a bit darker than the planet color
      phong = PlanetColor - 0.3f;
    }
  }
  //out_Color = vec4(phong, 1.0f);
  out_Color = texture(ourTexture, TexCoord);
}
