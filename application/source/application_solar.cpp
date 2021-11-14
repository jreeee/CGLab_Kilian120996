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
 ,m_view_transform{glm::translate(glm::fmat4{}, glm::fvec3{0.0f, 0.0f, 4.0f})}
 ,m_view_projection{utils::calculate_projection_matrix(initial_aspect_ratio)}
 ,geonodes {std::vector<std::shared_ptr<GeometryNode>>{}}
{
  initializeScreenGraph();
  initializeGeometry();
  initializeShaderPrograms();
}

ApplicationSolar::~ApplicationSolar() {
  glDeleteBuffers(1, &planet_object.vertex_BO);
  glDeleteBuffers(1, &planet_object.element_BO);
  glDeleteVertexArrays(1, &planet_object.vertex_AO);
}

void ApplicationSolar::render() const { 
  //including the planets since they should also be displayed
  renderPlanet();
  glUseProgram(m_shaders.at("planet").handle);

    glm::fmat4 model_matrix = glm::rotate(glm::fmat4{}, float(glfwGetTime()), glm::fvec3{-1.0f, 0.0f, 0.0f});
    model_matrix = glm::translate(model_matrix, glm::fvec3{0.0f, 0.0f, -1.0f});
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


void ApplicationSolar::renderPlanet() const {
  // using the vector to get references to the planets to render each one
  for (auto i : geonodes) {
    glUseProgram(m_shaders.at("planet").handle);

    auto model_matrix = i->getWorldTransform();
  
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

void ApplicationSolar::uploadView() {
  // vertices are transformed in camera space, so camera transform must be inverted
  glm::fmat4 view_matrix = glm::inverse(m_view_transform);
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ViewMatrix"),
                     1, GL_FALSE, glm::value_ptr(view_matrix));
}

void ApplicationSolar::uploadProjection() {
  // upload matrix to gpu
  glUniformMatrix4fv(m_shaders.at("planet").u_locs.at("ProjectionMatrix"),
                     1, GL_FALSE, glm::value_ptr(m_view_projection));
}

// update uniform locations
void ApplicationSolar::uploadUniforms() { 
  // bind shader to which to upload unforms
  glUseProgram(m_shaders.at("planet").handle);
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
    root->addChildren(camera);

    auto solarsystem = SceneGraph("Solar System Scene Graph", root);

    //adding all the other nodes and their children
    auto sun = std::make_shared<PointLightNode>(root, "PointLight", 100.0f);
    root->addChildren(sun);

    auto sun_geo = std::make_shared<GeometryNode>(sun, "Sun Geometry", mdl_ptr);
    sun->addChildren(sun_geo);

    auto mercury = std::make_shared<Node>(root, "Mercury");
    root->addChildren(mercury);

    auto mercury_geo = std::make_shared<GeometryNode>(mercury, "Mercury Geometry", mdl_ptr);
    mercury->addChildren(mercury_geo);

    auto venus = std::make_shared<Node>(root, "Venus");
    root->addChildren(venus);

    auto venus_geo = std::make_shared<GeometryNode>(venus, "Venus Geometry", mdl_ptr);
    venus->addChildren(venus_geo);

    auto earth = std::make_shared<Node>(root, "Earth");
    root->addChildren(earth);

    auto earth_geo = std::make_shared<GeometryNode>(earth, "Earth Geometry", mdl_ptr);
    earth->addChildren(earth_geo);

    auto moon = std::make_shared<Node>(earth, "Moon");
    earth->addChildren(moon);

    auto moon_geo = std::make_shared<GeometryNode>(moon, "Moon Geometry", mdl_ptr);
    moon->addChildren(moon_geo);

    auto mars = std::make_shared<Node>(root, "Mars");
    root->addChildren(mars);

    auto mars_geo = std::make_shared<GeometryNode>(mars, "Mars Geometry", mdl_ptr);
    mars->addChildren(mars_geo);

    auto jupiter = std::make_shared<Node>(root, "Jupiter");
    root->addChildren(jupiter);

    auto jupiter_geo = std::make_shared<GeometryNode>(jupiter, "Jupiter Geometry", mdl_ptr);
    jupiter->addChildren(jupiter_geo);  

    auto saturn = std::make_shared<Node>(root, "Saturn");
    root->addChildren(saturn);

    auto saturn_geo = std::make_shared<GeometryNode>(saturn, "Saturn Geometry", mdl_ptr);
    saturn->addChildren(saturn_geo);

    auto uranus = std::make_shared<Node>(root, "Uranus");
    root->addChildren(uranus);

    auto uranus_geo = std::make_shared<GeometryNode>(uranus, "Uranus Geometry", mdl_ptr);
    uranus->addChildren(uranus_geo);

    auto neptune = std::make_shared<Node>(root, "Neptune");
    root->addChildren(neptune);

    auto neptune_geo = std::make_shared<GeometryNode>(neptune, "Neptune Geometry", mdl_ptr);
    neptune->addChildren(neptune_geo);

    //could be done with dynamic pointers or the above things could be initialized in the array
    //moving pointers to all the geometric nodes into the test vector to easily be able to iterate over them later on
    geonodes.push_back(sun_geo);
    geonodes.push_back(mercury_geo);
    geonodes.push_back(venus_geo);
    geonodes.push_back(earth_geo);
    geonodes.push_back(moon_geo);
    geonodes.push_back(mars_geo);
    geonodes.push_back(jupiter_geo);
    geonodes.push_back(saturn_geo);
    geonodes.push_back(uranus_geo);
    geonodes.push_back(neptune_geo);

    float distance = 3.0f;
    for (auto i : geonodes) {
      if (i->getName() == "Moon Geometry") {
        i->setLocalTransform(glm::translate(i->getLocalTransform(), glm::fvec3{0.0f , 2.0f, 0.0f}));
      }
      else {
        distance += 2;
        i->setLocalTransform(glm::translate(i->getLocalTransform(), glm::fvec3{distance , 0.0f, 0.0f}));
      }
    }
    std::cout << solarsystem.printGraph();
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