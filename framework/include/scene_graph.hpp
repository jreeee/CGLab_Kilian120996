#ifndef SCENE_GRAPH_HPP
#define SCENE_GRAPH_HPP

#include <string>
#include <sstream>

#include "node.hpp"

class SceneGraph {

    private:

        // member variables
        std::string name_;
        std::shared_ptr<Node> root_;

    public:

        // con- & destructor
        SceneGraph();
        SceneGraph(std::string const& name, std::shared_ptr<Node> root);
        ~SceneGraph();
        
        // methods
        std::string getName() const;
        void setName(std::string const& name);
        std::shared_ptr<Node> getRoot() const;
        void setRoot(std::shared_ptr<Node> root);
        std::string printGraph();

};

#endif