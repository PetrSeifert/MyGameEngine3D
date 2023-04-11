#include <systems/simple_renderer_system.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

//std
#include <stdexcept>
#include <array>

struct SimplePushConstantData
{
	glm::mat4 modelMatrix{ 1.f };
};

SimpleRendererSystem::SimpleRendererSystem(MyDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : myDevice{ device }
{
	createPipelineLayout(globalSetLayout);
	createPipeline(renderPass);
}

SimpleRendererSystem::~SimpleRendererSystem()
{
	vkDestroyPipelineLayout(myDevice.device(), pipelineLayout, nullptr);
}

void SimpleRendererSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(myDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout");
	}
}

void SimpleRendererSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "Cannor create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	MyPipeline::defaultPipelineConfigInfo(pipelineConfig);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	myPipeline = std::make_unique<MyPipeline>(
		myDevice,
		"shaders/simple.vert.spv",
		"shaders/simple.frag.spv",
		pipelineConfig
	);
}

void SimpleRendererSystem::renderGameObjects(FrameInfo& frameInfo)
{
	myPipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(
		frameInfo.commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		0,
		1,
		&frameInfo.globalDescriptorSet,
		0,
		nullptr
	);

	for (auto& kv : frameInfo.gameObjects)
	{
		auto& obj = kv.second;
		if (obj.model == nullptr) continue;
		SimplePushConstantData push{};
		push.modelMatrix = obj.transform.mat4();

		vkCmdPushConstants(
			frameInfo.commandBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(SimplePushConstantData),
			&push
		);
		obj.model->bind(frameInfo.commandBuffer);
		obj.model->draw(frameInfo.commandBuffer);
	}
}