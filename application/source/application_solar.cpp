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

// SceneGraph and nodes from assignment 1
#include <camera_node.hpp>
#include <pointlight_node.hpp>


#include <iostream>

ApplicationSolar::ApplicationSolar(std::string const& resource_path)
 :Application{resource_path}
 ,planet_object{}
 ,star_object{}
 ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})}
 ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
 ,m_scene_graph {}
{
  initializeScreenGraph();
  initializeGeometry();
  initializeStars();
  initializeShaderPrograms();
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteBuffers(1, &star_object.vertex_BO);
  glDeleteBuffers(1, &star_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
  glDeleteVertexArrays(1, &star_object.vertex_AO);
}

void ApplicationSolar::render() const { 
  //including the planets since they should also be displayed
  renderStars();
  renderPlanet();

  // glUseProgram(m_shaders.at("planet").handle);

  //   glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()), glm::fvec3{-1.0f, 0.0f, 0.0f});
  //   model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f});
  //   glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
  //                     1, GL_FALSE, glm::value_ptr(model_matrix));

  //   // extra matrix for normal transformation to keep them orthogonal to surface
  //   glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
  //   glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
  //                     1, GL_FALSE, glm::value_ptr(normal_matrix));

  //   // bind the VAO to draw
  //   glBindVertexArray(planet_object.vertex_AO);

  //   // draw bound vertex array using bound shader
  //   glDrawElements(planet_object.draw_mode, planet_object.num_elements, model::INDEX.type, NULL);
}


void ApplicationSolar::renderPlanet() const {
  // using the vector to get references to the planets to render each one

  for (auto i : m_geo) {
    glUseProgram(m_shaders.at("planet").handle);

    auto i_parent = i->getParent();
    i_parent->setLocalTransform(glm::rotate(i_parent->getWorldTransform(), 
                                            float(0.04f * i->getRot()), 
                                            glm::fvec3{0.0f, 0.0f, -1.0f}));


    auto model_matrix = i->getWorldTransform();
    //model_matrix = glm::rotate(model_matrix, float(i->getSpin()), glm::fvec3{0.0f, 1.0f, 0.0f});
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ModelMatrix"),
                      1, GL_FALSE, glm::value_ptr(model_matrix));

    // extra matrix for normal transformation to keep them orthogonal to surface
    glm::fmat4 normal_matrix = glm::inverseTranspose(glm::inverse(m_view_transform) * model_matrix);
    glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("NormalMatrix"),
                      1, GL_FALSE, glm::value_ptr(normal_matrix));
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

void ApplicationSolar::uploadView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ModelViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::uploadProjection() {
  // upload matrix to gpu
  glUseProgram(m_shaders.at("planet").handle);
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
  glUseProgram(m_shaders.at("star").handle);
  glUniformMatrix4fv(m_shaders.at("star").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
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

    auto mdl_ptr = std::make_shared<model>();
    //creation of the root node and the SceneGraph
    auto root = std::make_shared<Node>(nullptr, "Root");
    auto camera = std::make_shared<CameraNode>(root, "Camera", true, true, glm::mat4());

    m_scene_graph = SceneGraph("Solar System Scene Graph", root);

    //adding all the planet nodes
    auto sun = std::make_shared<PointLightNode>(root, "PointLight", 100.0f);
    auto mercury = std::make_shared<Node>(root, "Mercury");
    auto venus = std::make_shared<Node>(root, "Venus");
    auto earth = std::make_shared<Node>(root, "Earth");
    auto moon = std::make_shared<Node>(nullptr, "Moon"); //earth geo, not instantiated
    auto mars = std::make_shared<Node>(root, "Mars");
    auto jupiter = std::make_shared<Node>(root, "Jupiter");
    auto saturn = std::make_shared<Node>(root, "Saturn");
    auto uranus = std::make_shared<Node>(root, "Uranus");
    auto neptune = std::make_shared<Node>(root, "Neptune");

    //storing pointers to the geo vector while initializing the geo nodes
    m_geo = { {std::make_shared<GeometryNode>(sun, "Sun Geometry", mdl_ptr, 0.0f, 1.0f)},
              {std::make_shared<GeometryNode>(mercury, "Mercury Geometry", mdl_ptr, 0.2f, 0.3f)},
              {std::make_shared<GeometryNode>(venus, "Venus Geometry", mdl_ptr, 0.3f, 0.1f)},
              {std::make_shared<GeometryNode>(earth, "Earth Geometry", mdl_ptr, 0.25f, 0.4f)},
              {std::make_shared<GeometryNode>(moon, "Moon Geometry", mdl_ptr, 0.4f, 0.23f)},
              {std::make_shared<GeometryNode>(mars, "Mars Geometry", mdl_ptr, 0.32f, 0.14f)},
              {std::make_shared<GeometryNode>(jupiter, "Jupiter Geometry", mdl_ptr, 0.13f, 0.26f)},
              {std::make_shared<GeometryNode>(saturn, "Saturn Geometry", mdl_ptr, 0.06f, 0.31f)},
              {std::make_shared<GeometryNode>(uranus, "Uranus Geometry", mdl_ptr, 0.15f, 0.27f)},
              {std::make_shared<GeometryNode>(neptune, "Neptune Geometry", mdl_ptr, 0.17f, 0.23f)} };

    //adding all the nodes that are children of root
    root->addChildren(camera);
    root->addChildren(sun);
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

    //setting distance and scale for the planets
    std::vector<float> dist {0.0f, 1.5f, 3.2f, 5.0f, 0.5f, 6.7f, 9.8f, 11.6f, 13.0f, 15.2f};
    std::vector<float> scale {1.0f, 0.7f, 0.4f, 0.5f, 0.2f, 0.35f, 0.7f, 0.6f, 0.4f, 0.3f};

    for (int i = 0; i < m_geo.size(); ++i) {
      m_geo[i]->setLocalTransform(glm::translate(m_geo[i]->getLocalTransform(), glm::fvec3{0.0f , dist[i], 0.0f}));
      m_geo[i]->setLocalTransform(glm::scale(m_geo[i]->getLocalTransform(), glm::fvec3{scale[i], scale[i], scale[i]}));
    }
    std::cout << m_scene_graph.printGraph();
  }

void ApplicationSolar::initializeStars() {

  //parameters for the creation of the stars
  unsigned int stars_size = 100000;
  unsigned int density = 150;
  unsigned int base_brightness = 150;

  std::vector<GLfloat> stars;

  //only to make the loop a bit faster, as these stay constant
  float densityh = float(density) / 2;
  unsigned int rand_brightness = 265 - base_brightness;
  
  for (int i = 0; i < stars_size; ++i) {
    //as we need three coordinate- and colour values we can add another for loop
    for (int j = 0; j < 3; ++j) {
      stars.push_back((std::rand() % density) - densityh);
    }
    for (int k = 0; k < 3; ++k) {
      //we add a randomly generated number to the base value and divide by 256 to get our value
      stars.push_back(float((std::rand() % rand_brightness) + base_brightness)/256);
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
  star_object.num_elements = stars_size;

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

  m_shaders.emplace("star", shader_program{{{GL_VERTEX_SHADER,m_resource_path + "shaders/vao.vert"},
                                           {GL_FRAGMENT_SHADER, m_resource_path + "shaders/vao.frag"}}});
  // request uniform locations for shader program
  m_shaders.at("star").u_locs["ModelViewMatrix"] = -1;
  m_shaders.at("star").u_locs["ProjectionMatrix"] = -1;
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
  if (key == GLFW_KEY_W  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, -0.1f});
  }
  else if (key == GLFW_KEY_S  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.0f, 0.1f});
  }
  else if (key == GLFW_KEY_A  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{-0.1f, 0.0f, 0.0f});
  }
  else if (key == GLFW_KEY_D  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.1f, 0.0f, 0.0f});
  }
  else if (key == GLFW_KEY_SPACE  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, 0.1f, 0.0f});
  }
  //interestingly you can't use L-shift like the other keys, likely having to to with the modifier-properties it has
  else if (key == GLFW_KEY_LEFT_SHIFT  && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
    m_view_transform = glm::translate(m_view_transform, glm::fvec3{0.0f, -0.1f, 0.0f});
  }
  uploadView();
}

//handle delta mouse movement input
void ApplicationSolar::mouseCallback(double pos_x, double pos_y) {
  double multiplier = 0.005; //stored as a double, so that we can easily multiply with pos_n and then cast to float
  
  // mouse handling in x and y directions
  m_view_transform = glm::rotate( m_view_transform, float(pos_x * multiplier),
                                    glm::fvec3{0.0f, -1.0f, 0.0f});
  m_view_transform = glm::rotate(m_view_transform, float(pos_y * multiplier),
                                    glm::fvec3{-1.0f, 0.0f, 0.0f});

  uploadView();
}

//handle resizing
void ApplicationSolar::resizeCallback(unsigned width, unsigned height) {
  // recalculate projection matrix for new aspect ration
  m_view_projection = utils::calculate_projection_matrix(float(width) / float(height));
  // upload new projection matrix
  uploadProjection();
}


// exe entry point
int main(int argc, char* argv[]) {

  Application::run<ApplicationSolar>(argc, argv, 3, 2);

}