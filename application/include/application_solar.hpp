#ifndef APPLICATION_SOLAR_HPP
#define APPLICATION_SOLAR_HPP

#include "application.hpp"
#include "model.hpp"
#include "structs.hpp"

#include <scene_graph.hpp>
#include <geometry_node.hpp>

// gpu representation of model
class ApplicationSolar : public Application {
 public:
  // allocate and initialize objects
  ApplicationSolar(std::string const& resource_path);
  // free allocated objects
  ~ApplicationSolar();

  // react to key input
  void keyCallback(int key, int action, int mods);
  //handle delta mouse movement input
  void mouseCallback(double pos_x, double pos_y);
  //handle resizing
  void resizeCallback(unsigned width, unsigned height);

  // draw all objects
  void render() const;

  //render a planet
  void renderPlanet() const;
  void renderStars() const;
  void renderOrbits() const;

 protected:
  void initializeScreenGraph();
  void initializeStars();
  void initializeOrbits();
  void initializeTextures();
  void initializeShaderPrograms();
  void initializeGeometry(std::string const& path);
  // update uniform values
  void uploadUniforms();
  // upload projection matrix
  void uploadProjection();
  // upload view matrix
  void uploadView();

  // cpu representation of model
  model_object planet_object;
  model_object star_object;
  model_object orbit_object;
  
  // scene graph
  SceneGraph m_scene_graph;
  std::vector<std::shared_ptr<GeometryNode>> m_geo;
  std::vector<std::shared_ptr<GeometryNode>> m_orbit;
};

#endif