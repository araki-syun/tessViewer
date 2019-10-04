#pragma once

#include <boost\property_tree\ptree.hpp>
#include <boost\property_tree\xml_parser.hpp>

namespace glapp{
	class Config
	{
	public:
		//static Config config;

		Config(void);
		~Config(void);
		Config(const std::string& filename);

		void Reset(const std::string& filename);

		template<typename T>
		const T& Get(const std::string& key) const{
			return _tree.get<T>(key);
		}
		template<typename T>
		void Set(const std::string& key, const T& value){
			_tree.put(key, value);
		}

	private:
		boost::property_tree::ptree _tree;

	};
}

