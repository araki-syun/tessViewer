#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

//#include <boost\flyweight.hpp>
//#include <boost\flyweight\key_value.hpp>

#include "mesh.h"
#include "material.h"
#include "node.h"

namespace glapp{
	class model
	{
		struct model_node_value{
			std::shared_ptr<mesh> pMesh;
			std::shared_ptr<material> pMaterial;
		};
		typedef node<model_node_value> model_node;

	public:
		model(void);
		virtual ~model(void);

		void Load(const std::string& filename);
		void Draw() const;

	protected:
		void node_Draw(const model_node* node) const;

		model_node root_mesh;

		//typedef boost::flyweight< boost::flyweights::key_value< std::string, mesh> > fw_mesh;
		//typedef boost::flyweight< boost::flyweights::key_value< std::string, material> > fw_material;

		static std::unordered_map<std::string, std::shared_ptr<mesh> >		mesh_list;
		static std::unordered_map<std::string, std::shared_ptr<material> >	material_list;

		typedef typename std::unordered_map<std::string, std::shared_ptr<mesh> >::iterator				mesh_list_iterator;
		typedef typename std::unordered_map<std::string, std::shared_ptr<mesh> >::const_iterator		const_mesh_list_iterator;
		typedef typename std::unordered_map<std::string, std::shared_ptr<material> >::iterator			material_list_iterator;
		typedef typename std::unordered_map<std::string, std::shared_ptr<material> >::const_iterator	const_material_list_iterator;

		

	};
}

