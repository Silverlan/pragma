#include "pragma/math/c_util_math.hpp"

const std::array<Mat4,6> &pragma::math::get_cubemap_view_matrices()
{
	static const std::array<Mat4,6> viewMatrices = {
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(-1.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,1.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,-1.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,1.0f),glm::vec3(0.0f,-1.0f,0.0f)),
		glm::lookAtRH(glm::vec3(0.0f,0.0f,0.0f),glm::vec3(0.0f,0.0f,-1.0f),glm::vec3(0.0f,-1.0f,0.0f))
	};
	return viewMatrices;
}

const Mat4 &pragma::math::get_cubemap_projection_matrix(float aspectRatio,float nearZ,float farZ)
{
	static const auto projMatrix = glm::perspectiveRH<float>(glm::radians(90.0f),aspectRatio,nearZ,farZ);
	return projMatrix;
}
