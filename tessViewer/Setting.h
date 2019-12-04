#pragma once

#include <map>
#include <memory>
#include <functional>
#include <string>

//#include "picojson.h"

class Setting {
public:
	Setting();
	~Setting();

	template <typename T>
	const T& Get(const std::string& name) {
		return *static_cast<T*>(list[name].val);
	}
	template <typename T>
	void Set(const std::string& name, const T& val) {
		list[name] = std::move(std::unique_ptr<void, Generic_unique_deleter>(
			new T(val), Generic_unique_deleter<T>()));
	}

private:
	class Generic_unique_deleter {
		std::function<void(void*)> del;

	public:
		template <class T>
		Generic_unique_deleter() {
			del = [](void* p) { delete static_cast<T*>(p); }
		}
		template <class T>
		Generic_unique_deleter(T func) {
			del = func;
		}
		void operator()(void* p) const noexcept { del(p); }
	};
	class Generic_unique_ptr {
		int                                           size;
		std::unique_ptr<void, Generic_unique_deleter> val;
	};
	std::map<std::string, Generic_unique_ptr> list;
};
