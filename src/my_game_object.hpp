#pragma once

#include <my_model.hpp>

#include <glm/gtc/matrix_transform.hpp>

//std
#include <memory>
#include <unordered_map>

struct TransformComponent 
{
	glm::vec3 translation{ 0.f, 0.f, 0.f };
	glm::vec3 scale{ 1.f, 1.f, 1.f };
	glm::vec3 rotation;

	glm::mat4 mat4() 
	{
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		return glm::mat4
		{
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{translation.x * 2, translation.y * 2, translation.z * 2, 2}
		};
	}
};

class MyGameObject 
{
public:
	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, MyGameObject>;

	static MyGameObject createGameObject() 
	{
		static id_t currentId = 0;
		return MyGameObject(currentId++);
	}

	MyGameObject(const MyGameObject&) = delete;
	MyGameObject& operator=(const MyGameObject&) = delete;
	MyGameObject(MyGameObject&&) = default;
	MyGameObject& operator=(MyGameObject&&) = default;

	id_t getId() { return id; }

	std::shared_ptr<MyModel> model{};
	glm::vec3 color{};
	TransformComponent transform{};

private:
	MyGameObject(id_t objId) : id(objId) {}

	id_t id;
};