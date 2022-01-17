#include "application_solar.hpp"
#include "window_handler.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"
#include "texture_loader.hpp"

#include <glbinding/gl/gl.h>
// use gl definitions from glbinding 
using namespace gl;

//dont load gl bindings from glfw
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
//debug
#include <glm/gtx/string_cast.hpp>

// SceneGraph and nodes from assignment 1
#include <camera_node.hpp>
#include <pointlight_node.hpp>


#include <iostream>

//Contstants
const double Mouse_Multiplier = 0.005;
const unsigned int STAR_COUNT = 100000;
const unsigned int STAR_DENSITY = 1000;
const unsigned int STAR_BRIGHTNESS = 100;
const unsigned int ORBIT_POINTS = 100;
const glm::fvec4 ORIGIN = {0.0f, 0.0f, 0.0f, 1.0f};
const unsigned int COLOR_SEED = 3;
const float SKYBOX_SIZE = 1000.0f;
const float SUN_BRIGHTNESS = 4.5f;

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,star_object{}
 ,orbit_object{}
 ,skybox_object{}
 ,quad_object{}
 ,m_scene_graph {}
 ,m_screen_height{initial_resolution.x}
 ,m_screen_width{initial_resolution.y}
{
  initializeScreenGraph();

  //setting up the camera 
  auto camera_node = m_scene_graph.getCamera();
  assert(camera_node != nullptr);
  //view transform
  camera_node->setLocalTransform(glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f}));
  //view projection
  camera_node->setProjectionMatrix(utils::calculate_projection_matrix(initial_aspect_ratio));

  initializeGeometry(m_resource_path + "models/sphere.obj");
  initializeStars();
  initializeOrbits();
  initializeTextures();
  initializeSkybox();
  initializeShaderPrograms();
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteBuffers(1, &star_object.element_BO);
  glDeleteBuffers(1, &orbit_object.vertex_BO);
  glDeleteBuffers(1, &orbit_object.element_BO);
  glDeleteBuffers(1, &skybox_object.vertex_BO);
  glDeleteBuffers(1, &quad_object.vertex_AO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);
  glDeleteVertexArrays(1, &orbit_object.vertex_AO);
  glDeleteVertexArrays(1, &skybox_object.vertex_AO);
  glDeleteVertexArrays(1, &quad_object.vertex_AO);
  glDeleteFramebuffers(1, &m_fbo);
  glDeleteTextures(1, &m_tex);
}

void ApplicationSolar::render() { 
  //including the planets since they should also be displayed
  renderStars();
  renderOrbits();
  renderPlanets();
  renderSkybox();
  renderFramebuffer();
}


void ApplicationSolar::renderPlanets() {
  //getting pointers to the light nodes
  auto light = std::dynamic_pointer_cast<PointLightNode>(m_scene_graph.getRoot()->getChildren("PointLight"));
  auto ambient = std::dynamic_pointer_cast<PointLightNode>(m_scene_graph.getRoot()->getChildren("AmbientLight"));
  assert(light && ambient);
  
  // using the vector to get references to the planets to render each one
  for (auto i : m_geo) {

    glUseProgram(m_shaders.at("planet").handle);

    //setting the ambient intensity as the sun needs a higher one to shine
    float speed = m_scene_graph.getSpeed();
    auto i_parent = i->getParent();
    auto mat = i->getMaterial();
    auto texture = i->getTexId(0);
    bool hasNormal = i->hasNormal();
    glm::vec3 l_pos(light->getWorldTransform() * ORIGIN);
    glm::vec3 c_pos(m_scene_graph.getCamera()->getWorldTransform() * ORIGIN);

    //rotating the placeholder in the center
    i_parent->setLocalTransform(glm::rotate(i_parent->getLocalTransform(), 
                                            float(0.01f * speed * i->getRot()), 
                                            glm::fvec3{0.0f, 1.0f, 0.0f}));

    //rotating the planet around itself
    i->setLocalTransform(glm::rotate(i->getLocalTransform(), float(0.01f * speed * i->getSpin()), glm::fvec3{0.0f, 1.0f, 0.0f}));
    
    //will probably build a GLSL struct to to this a bit nicer
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                      1, GL_FALSE, glm::value_ptr(i->getWorldTransform()));

    //all the colors
    glUniform3f(m_shaders.at("planet").u_locs.at("PlanetColor"), mat->diffuse->r, mat->diffuse->g, mat->diffuse->b);
    glUniform3f(m_shaders.at("planet").u_locs.at("AmbientColor"), ambient->getLightColor().r, ambient->getLightColor().g, ambient->getLightColor().b);
    glUniform3f(m_shaders.at("planet").u_locs.at("LightColor"), light->getLightColor().r, light->getLightColor().g, light->getLightColor().b);
    glUniform3f(m_shaders.at("planet").u_locs.at("PlanetSpecular"), mat->specular->r, mat->specular->g, mat->specular->b);
    //positions
    glUniform3fv(m_shaders.at("planet").u_locs.at("CameraPosition"), 1, glm::value_ptr(c_pos));
    glUniform3fv(m_shaders.at("planet").u_locs.at("LightPosition"), 1, glm::value_ptr(l_pos));
    //floats
    glUniform1f(m_shaders.at("planet").u_locs.at("AmbientIntensity"), (i->getName() == "Sun Geometry") ? light->getIntensity() : ambient->getIntensity());
    glUniform1f(m_shaders.at("planet").u_locs.at("LightIntensity"), light->getIntensity());
    glUniform1f(m_shaders.at("planet").u_locs.at("PlanetAlpha"), mat->alpha);
    glUniform1f(m_shaders.at("planet").u_locs.at("PlanetRoughness"), mat->roughness);

    glUniform1b(m_shaders.at("planet").u_locs.at("HasNormal"), hasNormal);

    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);
    //diffuse texture
    glActiveTexture(GL_TEXTURE0);
    //uploading the texture
    glBindTexture(GL_TEXTURE_2D, i->getTexId(0));
    //telling the shader where to look
    glUniform1i(m_shaders.at("planet").u_locs.at("planetTexture"), 0);
    // bind the specific texture, in this case the normal map
    glBindTexture(GL_TEXTURE_2D, texture);
    if (hasNormal) {
      //same thing as b4
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, i->getTexId(1));
      glUniform1i(m_shaders.at("planet").u_locs.at("normalTexture"), 1);
    }
    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }
}

void ApplicationSolar::renderStars() {
  //doing essentially the same as with the planets but since they don't move we don't need special matrices
  //and due to the fact that they are stored in an Array we can just use glDrawArray to do that
  glUseProgram(m_shaders.at("star").handle);
  glBindVertexArray(star_object.vertex_AO);
  glDrawArrays(star_object.draw_mode, 0, star_object.num_elements);
}

void ApplicationSolar::renderOrbits() {
  for (auto i : m_orbit) {
    glUseProgram(m_shaders.at("orbit").handle);
    auto model_matrix = i->getLocalTransform();
    if (i->getName() == "Moon Orbit") {
      model_matrix = i->getWorldTransform();
      //cancelling out the rotation of the orbit
      i->setLocalTransform(glm::rotate(i->getLocalTransform(), float(0.01f * i->getRot() + i->getSpin()) * m_scene_graph.getSpeed(), glm::fvec3{0.0f, 1.0f, 0.0f}));
    }
    glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
                      1, GL_FALSE, glm::value_ptr(model_matrix));
    auto col = i->getMaterial()->diffuse;
    glUniform3f(m_shaders.at("orbit").u_locs.at("in_Color"), col->r, col->g, col->b);
    glBindVertexArray(orbit_object.vertex_AO);
    glDrawArrays(orbit_object.draw_mode, 0, orbit_object.num_elements);
  }
}
void ApplicationSolar::renderSkybox() {
  glUseProgram(m_shaders.at("skybox").handle);
  glDepthMask(GL_FALSE);
  glBindVertexArray(skybox_object.vertex_AO);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);
  //the cube is rendered in Triangles and consists out of 36 points
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glDepthMask(GL_TRUE);
}

void ApplicationSolar::renderFramebuffer() {
  //generating the framebuffer
  unsigned int fbo;
  glGenFramebuffers(1, &fbo);
  //binding the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  //creating a texture for the framebuffer
  glGenTextures(1, &m_tex);
  glBindTexture(GL_TEXTURE_2D, m_tex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //attaching the texture to the framebuffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, 0);
  //if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
  //setting it as the one that's being displayed
  //glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glUseProgram(m_shaders.at("fbo").handle);
  // first pass
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
  glEnable(GL_DEPTH_TEST);
  // DrawScene();	
    
  // second pass
  glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
  glClear(GL_COLOR_BUFFER_BIT);
  
  glBindVertexArray(quad_object.vertex_AO);
  glDisable(GL_DEPTH_TEST);
  glBindTexture(GL_TEXTURE_2D, m_tex);
  glDrawArrays(GL_TRIANGLES, 0, 6);

  glDeleteFramebuffers(1, &fbo);
}

void ApplicationSolar::uploadView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_scene_graph.getCamera()->getLocalTransform());
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ModelViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
  glUseProgram(m_shaders.at("skybox").handle);
  glUniformMatrix4fv(m_shaders.at("skybox").u_locs.at("ViewMatrix"),
                      1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::uploadProjection() {
  // upload matrix to gpu
  auto camera_proj = m_scene_graph.getCamera()->getProjectionMatrix();

  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(camera_proj));
  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(camera_proj));
  glUseProgram(m_shaders.at("orbit").handle);
  glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(camera_proj));
  glUseProgram(m_shaders.at("skybox").handle);
  glUniformMatrix4fv(m_shaders.at("skybox").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(camera_proj));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() { 
  // upload uniform values to new locations
  uploadView();
  uploadProjection();
}

///////////////////////////// intialisation functions /////////////////////////
  //build ScreenGraph
  void ApplicationSolar::initializeScreenGraph() {
    //creating the Materials
    //specular, roughness and alpha are the same for every planet atm, that can be easily changed tho
    auto spec = std::make_shared<Color>(1.0f, 1.0f, 1.0f);
    std::vector<std::shared_ptr<Material>> mats;
    for (int i = 0; i < 10; ++i) {
      //creating the planet and orbit color (diffuse) TODO
      std::vector<float> col;
      for (int j = 1; j < 4; ++j) {
        srand(3 * i + j + COLOR_SEED);
        col.push_back(float(std::rand()%256)/255);
      }
      //creating the material
      mats.push_back(std::make_shared<Material>(std::make_shared<Color>(col[0], col[1], col[2]), spec, 5.0f, 30.0f));
    }
    
    auto mdl_ptr = std::make_shared<model>();
    //creation of the root node and the SceneGraph
    auto root = std::make_shared<Node>(nullptr, "Root");
    auto camera = std::make_shared<CameraNode>(root, "Camera", true, true, glm::mat4());

    m_scene_graph = SceneGraph("Solar System Scene Graph", root);

    //adding all the planet nodes
    Color sun_color = {1.0f, 1.0f, 1.0f};
    Color ambient_color = {1.0f, 1.0f, 1.0f};
    auto sun = std::make_shared<PointLightNode>(root, "PointLight", sun_color, SUN_BRIGHTNESS);
    auto ambient =std::make_shared<PointLightNode>(root, "AmbientLight", ambient_color, 0.2f);
    auto mercury = std::make_shared<Node>(root, "Mercury");
    auto venus = std::make_shared<Node>(root, "Venus");
    auto earth = std::make_shared<Node>(root, "Earth");
    auto moon = std::make_shared<Node>(nullptr, "Moon"); //earth geo, not instantiated yet
    auto mars = std::make_shared<Node>(root, "Mars");
    auto jupiter = std::make_shared<Node>(root, "Jupiter");
    auto saturn = std::make_shared<Node>(root, "Saturn");
    auto uranus = std::make_shared<Node>(root, "Uranus");
    auto neptune = std::make_shared<Node>(root, "Neptune");

    //storing pointers to the geo vector while initializing the geo nodes
    m_geo = { {std::make_shared<GeometryNode>(sun, "Sun Geometry", mdl_ptr, 0.0f, 1.0f, 0.0f, 1.0f, mats[0])},
              {std::make_shared<GeometryNode>(mercury, "Mercury Geometry", mdl_ptr, 1.5f, 0.3f, 1.5f, 0.35f, mats[1])},
              {std::make_shared<GeometryNode>(venus, "Venus Geometry", mdl_ptr, 0.5f, 0.1f, 3.2f, 0.4f, mats[2])},
              {std::make_shared<GeometryNode>(earth, "Earth Geometry", mdl_ptr, 2.0f, 0.4f, 5.0f, 0.5f, mats[3])},
              {std::make_shared<GeometryNode>(moon, "Moon Geometry", mdl_ptr, 1.7f, 0.23f, 2.0f, 0.5f, mats[4])},
              {std::make_shared<GeometryNode>(mars, "Mars Geometry", mdl_ptr, 2.3f, 0.14f, 6.7f, 0.35f, mats[5])},
              {std::make_shared<GeometryNode>(jupiter, "Jupiter Geometry", mdl_ptr, 1.1f, 0.26f, 9.8f, 0.7f, mats[6])},
              {std::make_shared<GeometryNode>(saturn, "Saturn Geometry", mdl_ptr, 2.5f, 0.31f, 11.6f, 0.6f, mats[7])},
              {std::make_shared<GeometryNode>(uranus, "Uranus Geometry", mdl_ptr, 1.2f, 0.27f, 13.0f, 0.4f, mats[8])},
              {std::make_shared<GeometryNode>(neptune, "Neptune Geometry", mdl_ptr, 3.1f, 0.23f, 15.3f, 0.3f, mats[9])} };

    m_orbit = { {std::make_shared<GeometryNode>(mercury, "Mercury Orbit", 1.5f, mats[1])},
                {std::make_shared<GeometryNode>(venus, "Venus Orbit", 3.2f, mats[2])},
                {std::make_shared<GeometryNode>(earth, "Earth Orbit", 5.0f, mats[3])},
                //initializing with negative spin and rotation to cancel out the planet holder
                {std::make_shared<GeometryNode>(moon, "Moon Orbit", mdl_ptr, -0.4f, -0.23f, 2.0f, 1.0f, mats[4])},
                {std::make_shared<GeometryNode>(mars, "Mars Orbit", 6.7f, mats[5])},
                {std::make_shared<GeometryNode>(jupiter, "Jupiter Orbit", 9.8f, mats[6])},
                {std::make_shared<GeometryNode>(saturn, "Saturn Orbit", 11.6f, mats[7])},
                {std::make_shared<GeometryNode>(uranus, "Uranus Orbit", 13.0f, mats[8])},
                {std::make_shared<GeometryNode>(neptune, "Neptune Orbit", 15.3f, mats[9])} };

    //adding all the nodes that are children of root
    root->addChildren(camera);
    root->addChildren(sun);
    root->addChildren(ambient);
    root->addChildren(mercury);
    root->addChildren(venus);
    root->addChildren(mars);
    root->addChildren(earth);
    root->addChildren(jupiter);
    root->addChildren(saturn);
    root->addChildren(uranus);
    root->addChildren(neptune);
    //doing the same for the planets (and the moon)
    sun->addChildren(m_geo[0]);
    mercury->addChildren(m_geo[1]);
    venus->addChildren(m_geo[2]);
    earth->addChildren(m_geo[3]);
    m_geo[3]->addChildren(moon);
    moon->setParent(m_geo[3]);
    moon->addChildren(m_geo[4]);
    mars->addChildren(m_geo[5]);
    jupiter->addChildren(m_geo[6]);
    saturn->addChildren(m_geo[7]);
    uranus->addChildren(m_geo[8]);
    neptune->addChildren(m_geo[9]);
    //orbits
    mercury->addChildren(m_orbit[0]);
    venus->addChildren(m_orbit[1]);
    earth->addChildren(m_orbit[2]);
    m_orbit[2]->addChildren(moon);
    moon->addChildren(m_orbit[3]);
    mars->addChildren(m_orbit[4]);
    jupiter->addChildren(m_orbit[5]);
    saturn->addChildren(m_orbit[6]);
    uranus->addChildren(m_orbit[7]);
    neptune->addChildren(m_orbit[8]);

    for (int i = 0; i < m_geo.size(); ++i) {
      m_geo[i]->setLocalTransform(glm::translate(m_geo[i]->getLocalTransform(), glm::fvec3{m_geo[i]->getDist(), 0.0f, 0.0f}));
      m_geo[i]->setLocalTransform(glm::scale(m_geo[i]->getLocalTransform(), glm::fvec3{m_geo[i]->getSize(), m_geo[i]->getSize(), m_geo[i]->getSize()}));
    }
    for (int i = 0; i < m_orbit.size(); ++i) {
      auto dist = m_orbit[i]->getDist();
      m_orbit[i]->setLocalTransform(glm::scale(m_orbit[i]->getLocalTransform(), glm::fvec3{dist, dist, dist}));
    }

    //setting the normal(s), have to be set explicitly and require a "Planet"Normal.png to exist
    m_geo[4]->setNormal(true);

    std::cout << m_scene_graph.printGraph();
  }

void ApplicationSolar::initializeStars() {

  std::vector<GLfloat> stars;

  //only to make the loop a bit faster, as these stay constant
  float densityh = float(STAR_DENSITY) / 2.0f;
  unsigned int brightness = 265 - STAR_BRIGHTNESS;
  
  for (int i = 0; i < STAR_COUNT; ++i) {
    //as we need three coordinate- and colour values we can add another for loop
    for (int j = 0; j < 3; ++j) {
      //the * 1000 / 1000 is to get floats that have 4 decimal places and thus are more evenly spaced out
      stars.push_back(float(std::rand() % (STAR_DENSITY * 1000)) / 1000.0f - densityh);
    }
    for (int k = 0; k < 3; ++k) {
      //we add a randomly generated number to the base value and divide by 256 to get our value
      stars.push_back(float((std::rand() % brightness) + STAR_BRIGHTNESS)/256);
    }
  }

  //initialize Vertex Array
  glGenVertexArrays(GLint(1), &star_object.vertex_AO);
  glBindVertexArray(star_object.vertex_AO);
  //Buffers + Data
  glGenBuffers(GLuint(1), &star_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, star_object.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*stars.size(), stars.data(), GL_STATIC_DRAW);
  //position information via attributes
  glEnableVertexArrayAttrib(star_object.vertex_AO, 0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, 0);
  //same for color
  glEnableVertexArrayAttrib(star_object.vertex_AO, 1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float)*6, (void*)(sizeof(float)*3));
  //setting the draw mode
  star_object.draw_mode = GL_POINTS;
  star_object.num_elements = STAR_COUNT;

}

void ApplicationSolar::initializeOrbits() {
  std::vector<GLfloat> orbit_pts;

  for (int i = 0; i < ORBIT_POINTS; ++i) {
    float angle = 2.0f * M_PI * (float)i / ORBIT_POINTS;
    orbit_pts.push_back(sin(angle));
    orbit_pts.push_back(0);
    orbit_pts.push_back(cos(angle));
  }

  //initialize Vertex Array
  glGenVertexArrays(GLint(1), &orbit_object.vertex_AO);
  glBindVertexArray(orbit_object.vertex_AO);
  //Buffers + Data
  glGenBuffers(GLuint(1), &orbit_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, orbit_object.vertex_BO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*orbit_pts.size(), orbit_pts.data(), GL_STATIC_DRAW);
  //position information via attributes
  glEnableVertexArrayAttrib(orbit_object.vertex_AO, 0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);
  //generic buffer
  glGenBuffers(1, &orbit_object.element_BO);
  //binding
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, orbit_object.element_BO);
  
  //setting the draw mode
  orbit_object.draw_mode = GL_LINE_LOOP; //LINE_LOOP
  orbit_object.num_elements = ORBIT_POINTS;
}

void ApplicationSolar::initializeTextures() {
  //iterating over all planets
  bool extra = false;
  for (auto i : m_geo) {
    NORMAL:
    auto path = m_resource_path + "textures/" + i->getParent()->getName() + ".png";
    if (extra) {
      path = m_resource_path + "textures/" + i->getParent()->getName() + "Normal.png";
    }
    //storing the image in texture
    pixel_data texture = texture_loader::file(path);
    //initialiasing
    glGenTextures(1, &m_tex);
    (!extra) ? i->setTexId(0, m_tex) : i->setTexId(1, m_tex);
    glBindTexture(GL_TEXTURE_2D, m_tex);
    
    //parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, texture.channels, texture.width, texture.height, 0, texture.channels, texture.channel_type, texture.ptr());
    glGenerateMipmap(GL_TEXTURE_2D);
    extra = (i->hasNormal() && !extra) ? true : false;
    
    if (extra) goto NORMAL;
  }
}

void ApplicationSolar::initializeSkybox() {
  //saving all the points into the pos-vector
  std::vector<GLfloat> pos = {-SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
                              SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE,
                              SKYBOX_SIZE,  SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE,
                              -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE,
                              -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE,
                              -SKYBOX_SIZE, SKYBOX_SIZE,  SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
                              SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, SKYBOX_SIZE,
                              SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,
                              SKYBOX_SIZE,  SKYBOX_SIZE,  -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE,
                              -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, SKYBOX_SIZE,  SKYBOX_SIZE,
                              SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,
                              SKYBOX_SIZE,  -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
                              -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, SKYBOX_SIZE,  SKYBOX_SIZE,  -SKYBOX_SIZE,
                              SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,  SKYBOX_SIZE,
                              -SKYBOX_SIZE, SKYBOX_SIZE,  SKYBOX_SIZE,  -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE,
                              -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,
                              SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,  -SKYBOX_SIZE, -SKYBOX_SIZE,
                              -SKYBOX_SIZE, -SKYBOX_SIZE, SKYBOX_SIZE,  SKYBOX_SIZE,  -SKYBOX_SIZE, SKYBOX_SIZE };
  //generating & binding the array
  glGenVertexArrays(1, &skybox_object.vertex_AO);
  glBindVertexArray(skybox_object.vertex_AO);
  //same with buffers
  glGenBuffers(1, &skybox_object.vertex_BO);
  glBindBuffer(GL_ARRAY_BUFFER, skybox_object.vertex_BO);
  //specifying how the data should be read
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pos.size(), pos.data(), GL_STATIC_DRAW);
  glEnableVertexArrayAttrib(skybox_object.vertex_AO, 0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glGenTextures(1, &m_tex);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_tex);

  //saving the textures in the skybox-vector
  std::vector<pixel_data> skybox;
  skybox.push_back(texture_loader::file(m_resource_path + "textures/sb_r.png"));
  skybox.push_back(texture_loader::file(m_resource_path + "textures/sb_l.png"));
  skybox.push_back(texture_loader::file(m_resource_path + "textures/sb_b.png"));
  skybox.push_back(texture_loader::file(m_resource_path + "textures/sb_t.png"));
  skybox.push_back(texture_loader::file(m_resource_path + "textures/sb_f.png"));
  skybox.push_back(texture_loader::file(m_resource_path + "textures/sb_d.png"));

  //writing them into a cube map
  for (int i = 0; i < skybox.size(); ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, skybox[i].channels, skybox[i].width, skybox[i].height, 0, skybox[i].channels, skybox[i].channel_type, skybox[i].ptr());
  }

  //parameters for the cube map
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}

// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/simple.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/simple.frag"}}});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["LightColor"] = -1;
  m_shaders.at("planet").u_locs["LightIntensity"] = -1;
  m_shaders.at("planet").u_locs["AmbientColor"] = -1;
  m_shaders.at("planet").u_locs["AmbientIntensity"] = -1;
  m_shaders.at("planet").u_locs["PlanetColor"] = -1;
  m_shaders.at("planet").u_locs["PlanetSpecular"] = -1;
  m_shaders.at("planet").u_locs["PlanetAlpha"] = -1;
  m_shaders.at("planet").u_locs["PlanetRoughness"] = -1;
  m_shaders.at("planet").u_locs["LightPosition"] = -1;
  m_shaders.at("planet").u_locs["CameraPosition"] = -1;
  m_shaders.at("planet").u_locs["Cel"] = -1;
  m_shaders.at("planet").u_locs["Solid"] = -1;
  m_shaders.at("planet").u_locs["HasNormal"] = -1;
  m_shaders.at("planet").u_locs["normalTexture"] = -1;
  m_shaders.at("planet").u_locs["planetTexture"] = -1;

  m_shaders.emplace("star", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/vao.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});
  m_shaders.at("star").u_locs["ModelViewMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;

  m_shaders.emplace("orbit", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/orbit.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});
  m_shaders.at("orbit").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("orbit").u_locs["in_Color"] = -1;

  m_shaders.emplace("skybox", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/skybox.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/skybox.frag"}}});
  m_shaders.at("skybox").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("skybox").u_locs["ViewMatrix"] = -1;
}

// load models
void ApplicationSolar::initializeGeometry(std::string const& path) {
  model planet_model = model_loader::obj(path, model::NORMAL | model::TEXCOORD);

  // generate vertex array object
  glGenVertexArrays(1, &planet_object.vertex_AO);
  // bind the array for attaching buffers
  glBindVertexArray(planet_object.vertex_AO);

  // generate generic buffer
  glGenBuffers(1, &planet_object.vertex_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ARRAY_BUFFER, planet_object.vertex_BO);
  // configure currently bound array buffer
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * planet_model.data.size(), planet_model.data.data(), GL_STATIC_DRAW);

  // activate first attribute for the gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute for the gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);
  // third attribute for the gpu
  glEnableVertexAttribArray(2);
  //texture
  glVertexAttribPointer(2, model::TEXCOORD.components, model::TEXCOORD.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::TEXCOORD]);
   // generate generic buffer
  glGenBuffers(1, &planet_object.element_BO);
  // bind this as an vertex array buffer containing all attributes
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planet_object.element_BO);
  // configure currently bound array buffer
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, model::INDEX.size * planet_model.indices.size(), planet_model.indices.data(), GL_STATIC_DRAW);

  // store type of primitive to draw
  planet_object.draw_mode = GL_TRIANGLES;
  // transfer number of indices to model object 
  planet_object.num_elements = GLsizei(planet_model.indices.size());
}

void ApplicationSolar::initializeFramebuffer() {
  if ( ! glIsFramebuffer(m_fbo)) {
    glGenFramebuffers(1, &m_fbo);
    glGenTextures(1, &m_tex);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glBindTexture(GL_TEXTURE_2D, m_tex);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_screen_width, m_screen_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, 0);
  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);   
}

///////////////////////////// callback functions for window events ////////////
// handle key input for w a s d to move around
void ApplicationSolar::keyCallback(int key, int action, int mods) {
  auto camera_node = m_scene_graph.getCamera();
  if (key == GLFW_KEY_W  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    camera_node->setLocalTransform(glm::translate(camera_node->getLocalTransform(), glm::fvec3{0.0f, 0.0f, -0.1f}));
  }
  else if (key == GLFW_KEY_S  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    camera_node->setLocalTransform(glm::translate(camera_node->getLocalTransform(), glm::fvec3{0.0f, 0.0f, 0.1f}));
  }
  else if (key == GLFW_KEY_A  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    camera_node->setLocalTransform(glm::translate(camera_node->getLocalTransform(), glm::fvec3{-0.1f, 0.0f, 0.0f}));
  }
  else if (key == GLFW_KEY_D  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    camera_node->setLocalTransform(glm::translate(camera_node->getLocalTransform(), glm::fvec3{0.1f, 0.0f, 0.0f}));
  }
  else if (key == GLFW_KEY_SPACE  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    camera_node->setLocalTransform(glm::translate(camera_node->getLocalTransform(), glm::fvec3{0.0f, 0.1f, 0.0f}));
  }
  else if (key == GLFW_KEY_2) {
    glUseProgram(m_shaders.at("planet").handle);
    glUniform1b(m_shaders.at("planet").u_locs.at("Cel"), true);
  }
  else if (key == GLFW_KEY_1) {
    glUseProgram(m_shaders.at("planet").handle);
    glUniform1b(m_shaders.at("planet").u_locs.at("Cel"), false);
  }
  else if (key == GLFW_KEY_3) {
    glUseProgram(m_shaders.at("planet").handle);
    glUniform1b(m_shaders.at("planet").u_locs.at("Solid"), true);
  }
  else if (key == GLFW_KEY_4) {
    glUseProgram(m_shaders.at("planet").handle);
    glUniform1b(m_shaders.at("planet").u_locs.at("Solid"), false);
  }
  else if (key == GLFW_KEY_5 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_scene_graph.setSpeed(m_scene_graph.getSpeed() + 0.1f);
  }
  else if (key == GLFW_KEY_6 && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_scene_graph.setSpeed(m_scene_graph.getSpeed() - 0.1f);
  }
  else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
    initializeGeometry(m_resource_path + "models/sphere.obj");
  }
  else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
    initializeGeometry(m_resource_path + "models/low-poly-benchy.obj");
  }
  else if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
    auto light = std::dynamic_pointer_cast<PointLightNode>(m_scene_graph.getRoot()->getChildren("PointLight"));
    light->setIntensity(light->getIntensity() * 2.0f);
  }
  else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
  auto light = std::dynamic_pointer_cast<PointLightNode>(m_scene_graph.getRoot()->getChildren("PointLight"));
  light->setIntensity(light->getIntensity() / 2.0f);
  }
  //interestingly you can't use L-shift like the other keys, likely having to to with the modifier-properties it has
  else if (key == GLFW_KEY_LEFT_SHIFT  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    camera_node->setLocalTransform(glm::translate(camera_node->getLocalTransform(), glm::fvec3{0.0f, -0.1f, 0.0f}));
  }
  uploadView();
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  auto camera_node = m_scene_graph.getCamera();

  // mouse handling in x and y directions
  camera_node->setLocalTransform(glm::rotate(camera_node->getLocalTransform(), float(pos_x * Mouse_Multiplier),
                                    glm::fvec3{0.0f, -1.0f, 0.0f}));
  camera_node->setLocalTransform(glm::rotate(camera_node->getLocalTransform(), float(pos_y * Mouse_Multiplier),
                                    glm::fvec3{-1.0f, 0.0f, 0.0f}));

  uploadView();
}

//handle resizing
void ApplicationSolar::resizeCallback(unsigned width, unsigned height) {
  // recalculate projection matrix for new aspect ration
  m_scene_graph.getCamera()->setProjectionMatrix(utils::calculate_projection_matrix(float(width) / float(height)));
  // upload new projection matrix
  uploadProjection();
  m_screen_height = height;
  m_screen_width = width;
  initializeFramebuffer();
}


// exe entry point
int main(int argc, char* argv[]) {

  Application::run<ApplicationSolar>(argc, argv, 3, 2);

}