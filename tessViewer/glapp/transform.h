#pragma once

#include <glm\glm.hpp>

class transform {
public:
	transform(void);
	virtual ~transform(void);

protected:
	glm::vec3 _translate;
	glm::vec3 _rotate;
	glm::vec3 _scale;

public:
	virtual const float* data(void) const;
};
