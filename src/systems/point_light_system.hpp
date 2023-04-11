#pragma once

#include <my_camera.hpp>
#include <my_pipeline.hpp>
#include <my_device.hpp>
#include <my_game_object.hpp>
#include <my_frame_info.hpp>

//std
#include <memory>

class PointLightSystem
{
public:
	PointLightSystem(MyDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~PointLightSystem();

	PointLightSystem(const PointLightSystem&) = delete;
	PointLightSystem& operator=(const PointLightSystem&) = delete;

	void render(FrameInfo& frameInfo);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

	MyDevice& myDevice;

	std::unique_ptr<MyPipeline> myPipeline;
	VkPipelineLayout pipelineLayout;
};