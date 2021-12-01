#include "application_solar.hpp"
#include "window_handler.hpp"

#include "utils.hpp"
#include "shader_loader.hpp"
#include "model_loader.hpp"

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
const unsigned int STAR_COUNT = 100000;
const unsigned int STAR_DENSITY = 150;
const unsigned int STAR_BRIGHTNESS = 100;
const unsigned int ORBIT_POINTS = 100;
const glm::fvec4 ORIGIN = {0.0f, 0.0f, 0.0f, 1.0f};


ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,star_object{}
 ,orbit_object{}
 ,m_scene_graph {}
{
  initializeScreenGraph();

  //setting up the camera 
  auto camera_node = m_scene_graph.getCamera();
  assert(camera_node != nullptr);
  //view transform
  camera_node->setLocalTransform(glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f}));
  //view projection
  camera_node->setProjectionMatrix(utils::calculate_projection_matrix(initial_aspect_ratio));

  initializeGeometry();
  initializeStars();
  initializeOrbits();
  initializeShaderPrograms();
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteBuffers(1, &star_object.element_BO);
  glDeleteBuffers(1, &orbit_object.vertex_BO);
  glDeleteBuffers(1, &orbit_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);
  glDeleteVertexArrays(1, &orbit_object.vertex_AO);
}

void ApplicationSolar::render() const { 
  //including the planets since they should also be displayed
  renderStars();
  renderOrbits();
  renderPlanet();
}


void ApplicationSolar::renderPlanet() const {
  //getting pointers to the light nodes
  auto light = std::dynamic_pointer_cast<PointLightNode>(m_scene_graph.getRoot()->getChildren("PointLight"));
  auto ambient = std::dynamic_pointer_cast<PointLightNode>(m_scene_graph.getRoot()->getChildren("AmbientLight"));
  assert(light && ambient);
  
  // using the vector to get references to the planets to render each one
  for (auto i : m_geo) {
    //rotating the placeholder in the center
    auto i_parent = i->getParent();
    i_parent->setLocalTransform(glm::rotate(i_parent->getLocalTransform(), 
                                            float(0.01f * i->getRot()), 
                                            glm::fvec3{0.0f, 1.0f, 0.0f}));
    glUseProgram(m_shaders.at("planet").handle);

    auto model_matrix = i->getWorldTransform();
    //rotating the planet around itself
    model_matrix = glm::rotate(model_matrix, float(i->getSpin()), glm::fvec3{0.0f, 1.0f, 0.0f});
    
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                      1, GL_FALSE, glm::value_ptr(model_matrix));

    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_scene_graph.getCamera()->getLocalTransform() * model_matrix));
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                      1, GL_FALSE, glm::value_ptr(normal_matrix));
    auto mat = i->getMaterial();
    glUniform3f(m_shaders.at("planet").u_locs.at("PlanetColor"), mat->diffuse->r, mat->diffuse->g, mat->diffuse->b);
    glUniform3f(m_shaders.at("planet").u_locs.at("AmbientColor"), ambient->getLightColor().r, ambient->getLightColor().g, ambient->getLightColor().b);
    glUniform1f(m_shaders.at("planet").u_locs.at("AmbientIntensity"), ambient->getIntensity());
    glUniform3f(m_shaders.at("planet").u_locs.at("LightColor"), light->getLightColor().r, light->getLightColor().g, light->getLightColor().b);
    glUniform1f(m_shaders.at("planet").u_locs.at("LightIntensity"), light->getIntensity());
    glm::vec3 l_pos(light->getWorldTransform() * ORIGIN);
    glUniform3fv(m_shaders.at("planet").u_locs.at("LightPosition"), 1, glm::value_ptr(l_pos));

    // bind the VAO to draw
    glBindVertexArray(planet_object.vertex_AO);

    // draw bound vertex array using bound shader
    glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
  }
}

void ApplicationSolar::renderStars() const {
  //doing essentially the same as with the planets but since they don't move we don't need special matrices
  //and due to the fact that they are stored in an Array we can just use glDrawArray to do that
  glUseProgram(m_shaders.at("star").handle);
  glBindVertexArray(star_object.vertex_AO);
  glDrawArrays(star_object.draw_mode, 0, star_object.num_elements);
}

void ApplicationSolar::renderOrbits() const {
  for (auto i : m_orbit) {
    glUseProgram(m_shaders.at("orbit").handle);
    auto model_matrix = i->getLocalTransform();
    if (i->getName() == "Moon Orbit") {
      model_matrix = i->getWorldTransform();
      //cancelling out the rotation of the orbit
      i->setLocalTransform(glm::rotate(i->getLocalTransform(), float(0.01f * i->getRot() + i->getSpin()), glm::fvec3{0.0f, 1.0f, 0.0f}));
    }
    glUniformMatrix4fv(m_shaders.at("orbit").u_locs.at("ModelMatrix"),
                      1, GL_FALSE, glm::value_ptr(model_matrix));
    auto mat = i->getMaterial();
    glUniform3f(m_shaders.at("orbit").u_locs.at("in_Color"), mat->diffuse->r, mat->diffuse->g, mat->diffuse->b);
    glBindVertexArray(orbit_object.vertex_AO);
    glDrawArrays(orbit_object.draw_mode, 0, orbit_object.num_elements);
  }
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
    auto spec = std::make_shared<Color>(1.0f, 1.0f, 1.0f);
    std::vector<std::shared_ptr<Color>> cols = {{std::make_shared<Color>(1.0f, 1.0f, 1.0f)},
                                                  {std::make_shared<Color>(0.5f, 0.5f, 0.5f)},
                                                  {std::make_shared<Color>(0.1f, 0.1f, 0.1f)},
                                                  {std::make_shared<Color>(0.3f, 0.8f, 0.2f)},
                                                  {std::make_shared<Color>(0.2f, 0.4f, 0.6f)},
                                                  {std::make_shared<Color>(0.9f, 0.3f, 0.4f)},
                                                  {std::make_shared<Color>(0.5f, 0.8f, 0.8f)},
                                                  {std::make_shared<Color>(0.9f, 0.5f, 0.1f)},
                                                  {std::make_shared<Color>(0.8f, 0.2f, 0.7f)},
                                                  {std::make_shared<Color>(0.2f, 0.5f, 0.9f)} };
    std::vector<std::shared_ptr<Material>> mats  = {{std::make_shared<Material>(cols[0], spec, 25.0f, 10.0f)},
                                                    {std::make_shared<Material>(cols[1], spec, 25.0f, 10.0f)},
                                                    {std::make_shared<Material>(cols[2], spec, 25.0f, 10.0f)},
                                                    {std::make_shared<Material>(cols[3], spec, 25.0f, 10.0f)},
                                                    {std::make_shared<Material>(cols[4], spec, 25.0f, 10.0f)},
                                                    {std::make_shared<Material>(cols[5], spec, 25.0f, 10.0f)},
                                                    {std::make_shared<Material>(cols[6], spec, 25.0f, 10.0f)},
                                                    {std::make_shared<Material>(cols[7], spec, 25.0f, 10.0f)},
                                                    {std::make_shared<Material>(cols[8], spec, 25.0f, 10.0f)},
                                                    {std::make_shared<Material>(cols[9], spec, 25.0f, 10.0f)} };

    auto mdl_ptr = std::make_shared<model>();
    //creation of the root node and the SceneGraph
    auto root = std::make_shared<Node>(nullptr, "Root");
    auto camera = std::make_shared<CameraNode>(root, "Camera", true, true, glm::mat4());

    m_scene_graph = SceneGraph("Solar System Scene Graph", root);

    //adding all the planet nodes
    Color sun_color = {0.9f, 1.0f, 0.3f};
    Color ambient_color = {1.0f, 1.0f, 1.0f};
    auto sun = std::make_shared<PointLightNode>(root, "PointLight", sun_color, 100.0f);
    auto ambient =std::make_shared<PointLightNode>(root, "AmbientLight", ambient_color, 0.05f);
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
              {std::make_shared<GeometryNode>(mercury, "Mercury Geometry", mdl_ptr, 0.2f, 0.3f, 1.5f, 0.35f, mats[4])},
              {std::make_shared<GeometryNode>(venus, "Venus Geometry", mdl_ptr, 0.3f, 0.1f, 3.2f, 0.4f, mats[5])},
              {std::make_shared<GeometryNode>(earth, "Earth Geometry", mdl_ptr, 0.25f, 0.4f, 5.0f, 0.5f, mats[9])},
              {std::make_shared<GeometryNode>(moon, "Moon Geometry", mdl_ptr, 0.4f, 0.23f, 2.0f, 0.5f, mats[1])},
              {std::make_shared<GeometryNode>(mars, "Mars Geometry", mdl_ptr, 0.32f, 0.14f, 6.7f, 0.35f, mats[6])},
              {std::make_shared<GeometryNode>(jupiter, "Jupiter Geometry", mdl_ptr, 0.13f, 0.26f, 9.8f, 0.7f, mats[7])},
              {std::make_shared<GeometryNode>(saturn, "Saturn Geometry", mdl_ptr, 0.06f, 0.31f, 11.6f, 0.6f, mats[8])},
              {std::make_shared<GeometryNode>(uranus, "Uranus Geometry", mdl_ptr, 0.15f, 0.27f, 13.0f, 0.4f, mats[5])},
              {std::make_shared<GeometryNode>(neptune, "Neptune Geometry", mdl_ptr, 0.17f, 0.23f, 15.3f, 0.3f, mats[7])} };

    m_orbit = { {std::make_shared<GeometryNode>(mercury, "Mercury Orbit", 1.5f, mats[4])},
                {std::make_shared<GeometryNode>(venus, "Venus Orbit", 3.2f, mats[5])},
                {std::make_shared<GeometryNode>(earth, "Earth Orbit", 5.0f, mats[9])},
                //initializing with negative spin and rotation to cancel out the planet holder
                {std::make_shared<GeometryNode>(moon, "Moon Orbit", mdl_ptr, -0.4f, -0.23f, 2.0f, 1.0f, mats[1])},
                {std::make_shared<GeometryNode>(mars, "Mars Orbit", 6.7f, mats[6])},
                {std::make_shared<GeometryNode>(jupiter, "Jupiter Orbit", 9.8f, mats[7])},
                {std::make_shared<GeometryNode>(saturn, "Saturn Orbit", 11.6f, mats[8])},
                {std::make_shared<GeometryNode>(uranus, "Uranus Orbit", 13.0f, mats[5])},
                {std::make_shared<GeometryNode>(neptune, "Neptune Orbit", 15.3f, mats[7])} };

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

// load shader sources
void ApplicationSolar::initializeShaderPrograms() {
  // store shader program objects in container
  m_shaders.emplace("planet", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/simple.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/simple.frag"}}});
  // request uniform locations for shader program
  m_shaders.at("planet").u_locs["NormalMatrix"] = -1;
  m_shaders.at("planet").u_locs["ModelMatrix"] = -1;
  m_shaders.at("planet").u_locs["ViewMatrix"] = -1;
  m_shaders.at("planet").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("planet").u_locs["LightColor"] = -1;
  m_shaders.at("planet").u_locs["LightIntensity"] = -1;
  m_shaders.at("planet").u_locs["AmbientColor"] = -1;
  m_shaders.at("planet").u_locs["AmbientIntensity"] = -1;
  m_shaders.at("planet").u_locs["PlanetColor"] = -1;
  m_shaders.at("planet").u_locs["LightPosition"] = -1;

  m_shaders.emplace("star", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/vao.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});
  // request uniform locations for shader program
  m_shaders.at("star").u_locs["ModelViewMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;

  m_shaders.emplace("orbit", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/orbit.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});
                                           // request uniform locations for shader program
  m_shaders.at("orbit").u_locs["ModelMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ViewMatrix"] = -1;
  m_shaders.at("orbit").u_locs["ProjectionMatrix"] = -1;
  m_shaders.at("orbit").u_locs["in_Color"] = -1;
 
}

// load models
void ApplicationSolar::initializeGeometry() {
  model planet_model = model_loader::obj(m_resource_path + "models/sphere.obj", model::NORMAL);

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

  // activate first attribute on gpu
  glEnableVertexAttribArray(0);
  // first attribute is 3 floats with no offset & stride
  glVertexAttribPointer(0, model::POSITION.components, model::POSITION.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::POSITION]);
  // activate second attribute on gpu
  glEnableVertexAttribArray(1);
  // second attribute is 3 floats with no offset & stride
  glVertexAttribPointer(1, model::NORMAL.components, model::NORMAL.type, GL_FALSE, planet_model.vertex_bytes, planet_model.offsets[model::NORMAL]);

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
  //interestingly you can't use L-shift like the other keys, likely having to to with the modifier-properties it has
  else if (key == GLFW_KEY_LEFT_SHIFT  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    camera_node->setLocalTransform(glm::translate(camera_node->getLocalTransform(), glm::fvec3{0.0f, -0.1f, 0.0f}));
  }
  uploadView();
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  double multiplier = 0.005; //stored as a double, so that we can easily multiply with pos_n and then cast to float
  auto camera_node = m_scene_graph.getCamera();

  // mouse handling in x and y directions
  camera_node->setLocalTransform(glm::rotate( camera_node->getLocalTransform(), float(pos_x * multiplier),
                                    glm::fvec3{0.0f, -1.0f, 0.0f}));
  camera_node->setLocalTransform(glm::rotate(camera_node->getLocalTransform(), float(pos_y * multiplier),
                                    glm::fvec3{-1.0f, 0.0f, 0.0f}));

  uploadView();
}

//handle resizing
void ApplicationSolar::resizeCallback(unsigned width, unsigned height) {
  // recalculate projection matrix for new aspect ration
  m_scene_graph.getCamera()->setProjectionMatrix(utils::calculate_projection_matrix(float(width) / float(height)));
  // upload new projection matrix
  uploadProjection();
}


// exe entry point
int main(int argc, char* argv[]) {

  Application::run<ApplicationSolar>(argc, argv, 3, 2);

}