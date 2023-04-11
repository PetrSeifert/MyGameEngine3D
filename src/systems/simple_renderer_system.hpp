#pragma once

#include <my_camera.hpp>
#include <my_pipeline.hpp>
#include <my_device.hpp>
#include <my_game_object.hpp>
#include <my_frame_info.hpp>

//std
#include <memory>

class SimpleRendererSystem
{
public:
	SimpleRendererSystem(MyDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~SimpleRendererSystem();

	SimpleRendererSystem(const SimpleRendererSystem&) = delete;
	SimpleRendererSystem& operator=(const SimpleRendererSystem&) = delete;

	void renderGameObjects(FrameInfo& frameInfo);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

	MyDevice& myDevice;

	std::unique_ptr<MyPipeline> myPipeline;
	VkPipelineLayout pipelineLayout;
};