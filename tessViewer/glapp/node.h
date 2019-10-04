#pragma once

#include <vector>
#include <memory>



template<class T>
class node{
public:
	node() : parent(nullptr){}
	node(T& t) : value(t){}
	virtual ~node(){}

	virtual const T* operator->() const{
		return &value;
	}

	const std::shared_ptr<node<T> > operator[](unsigned int i){
		return children[i];
	}
	size_t size() const{
		return children.size();
	}

	node<T>* parent;
	std::vector<std::shared_ptr<node<T> > > children;

private:
	T value;

};

