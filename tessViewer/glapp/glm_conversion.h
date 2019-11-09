#pragma once

#include <limits>
#include <string>
#include <glm/glm.hpp>
#include <glm/detail/qualifier.hpp>
#include "nlohmann/adl_serializer.hpp"
#include "nlohmann/json.hpp"

namespace nlohmann {
template <size_t L, class T>
struct adl_serializer<glm::vec<L, T>> {
	using vec = glm::vec<L, T>;
	template <typename BasicJsonType>
	static void to_json(BasicJsonType& j, const vec& v) {
		for (auto i = 0; i < vec::length(); ++i)
			j.push_back(v[i]);
	}
	template <typename BasicJsonType>
	static void from_json(const BasicJsonType& j, vec& v) {
		vec value;
		for (auto i = 0; i < j.size() && i < vec::length(); ++i) {
			value[i] = j[i].template get<typename vec::value_type>();
		}
		v = std::move(value);
	}
};
template <>
struct adl_serializer<glm::vec<4, float>> {
	using vec = glm::vec<4, float>;
	template <typename BasicJsonType>
	static void from_json(const BasicJsonType& j, vec& v) {
		vec val(0, 0, 0, 1);

		if (j.is_array()) {
			auto size = j.size();
			for (int i = 0; i < size && i < 4; ++i) {
				val[i] = j.at(i).get<float>();
			}
		} else if (j.is_string()) {
			auto str = j.get<std::string>();
			if (str[0] == '#') { //example '#0099aaFF'
				if (str.size() % 2 == 0) {
					str.append("0");
				}
				for (int i = 0; i < 4 && i < (str.size() - 1) / 2; ++i) {
					val[i] = float(std::stoi("0x" + str.substr(i * 2 + 1, 2))) /
							 float(std::numeric_limits<unsigned char>().max());
				}
			}
		}
		v = val;
	}
};
} // namespace nlohmann
