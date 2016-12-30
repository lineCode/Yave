/*******************************
Copyright (C) 2013-2016 gregoire ANGERAND

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************/
#include "MaterialCompiler.h"

#include <yave/shaders/ShaderModule.h>
#include <yave/mesh/Vertex.h>
#include <yave/Device.h>

#include <yave/shaders/ShaderProgram.h>

#include <numeric>

namespace yave {

static vk::Format vec_format(ShaderModuleBase::Attribute attr) {
	switch(attr.vec_size) {
		case 1:
			return vk::Format::eR32Sfloat;
		case 2:
			return vk::Format::eR32G32Sfloat;
		case 3:
			return vk::Format::eR32G32B32Sfloat;
		case 4:
			return vk::Format::eR32G32B32A32Sfloat;

		default:
			break;
	}
	return fatal("Unsupported format");
}

static void create_attribs(u32 binding,
						   vk::VertexInputRate rate,
						   const core::Vector<ShaderModuleBase::Attribute>& attribs,
						   core::Vector<vk::VertexInputBindingDescription>& bindings,
						   core::Vector<vk::VertexInputAttributeDescription>& descriptions) {
	if(!attribs.is_empty()) {
		u32 offset = 0;
		for(const auto& attr : attribs) {
			auto format = vec_format(attr);
			for(u32 i = 0; i != attr.columns; i++) {
				descriptions << vk::VertexInputAttributeDescription()
						.setBinding(binding)
						.setLocation(attr.location + i)
						.setFormat(format)
						.setOffset(offset)
					;
				offset += attr.vec_size * attr.component_size;
			}
		}
		bindings << vk::VertexInputBindingDescription()
				.setBinding(binding)
				.setStride(offset)
				.setInputRate(rate)
			;
	}
}

static void create_attribs(const core::Vector<ShaderModuleBase::Attribute>& attribs,
						   core::Vector<vk::VertexInputBindingDescription>& bindings,
						   core::Vector<vk::VertexInputAttributeDescription>& descriptions) {

	core::Vector<ShaderModuleBase::Attribute> v_attribs;
	core::Vector<ShaderModuleBase::Attribute> i_attribs;

	for(const auto& attr : attribs) {
		(attr.location < ShaderProgram::PerInstanceLocation ? v_attribs : i_attribs) << attr;
	}
	create_attribs(0, vk::VertexInputRate::eVertex, v_attribs, bindings, descriptions);
	create_attribs(1, vk::VertexInputRate::eInstance, i_attribs, bindings, descriptions);
}

MaterialCompiler::MaterialCompiler(DevicePtr dptr) : DeviceLinked(dptr) {
}

GraphicPipeline MaterialCompiler::compile(const Material& material, const RenderPass& render_pass, Viewport view) const {
#warning move program creation
#warning attrib creation to program

	FragmentShader frag = FragmentShader(material.device(), material.data()._frag);
	VertexShader vert = VertexShader(material.device(), material.data()._vert);
	GeometryShader geom = material.data()._geom.is_empty() ? GeometryShader() : GeometryShader(material.device(), material.data()._geom);
	ShaderProgram program(frag, vert, geom);

	auto pipeline_shader_stage = program.vk_pipeline_stage_info();

	core::Vector<vk::VertexInputBindingDescription>	attribute_bindings;
	core::Vector<vk::VertexInputAttributeDescription> attribute_descriptions;

	create_attribs(program.vertex_attributes(), attribute_bindings, attribute_descriptions);

	auto vertex_input = vk::PipelineVertexInputStateCreateInfo()
			.setVertexAttributeDescriptionCount(attribute_descriptions.size())
			.setPVertexAttributeDescriptions(attribute_descriptions.begin())
			.setVertexBindingDescriptionCount(attribute_bindings.size())
			.setPVertexBindingDescriptions(attribute_bindings.begin())
		;

	auto input_assembly = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false)
		;

	auto viewport = vk::Viewport()
			.setWidth(view.extent.x())
			.setHeight(view.extent.y())
			.setX(view.offset.x())
			.setY(view.offset.y())
			.setMinDepth(view.depth.x())
			.setMaxDepth(view.depth.y())
		;

	auto scissor = vk::Rect2D()
			.setExtent(vk::Extent2D(u32(view.extent.x()), u32(view.extent.y())))
			.setOffset({0, 0})
		;

	auto viewport_state = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setPViewports(&viewport)
			.setScissorCount(1)
			.setPScissors(&scissor)
		;

	auto rasterizer = vk::PipelineRasterizationStateCreateInfo()
			.setCullMode(vk::CullModeFlagBits::eBack)
			//.setCullMode(vk::CullModeFlagBits::eNone)
			.setPolygonMode(vk::PolygonMode::eFill)
			.setLineWidth(1.0f)
			.setFrontFace(vk::FrontFace::eClockwise)
			.setDepthBiasEnable(false)
			.setDepthClampEnable(false)
		;

	auto multisampling = vk::PipelineMultisampleStateCreateInfo()
			.setSampleShadingEnable(false)
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		;

	auto color_blend_attachment = vk::PipelineColorBlendAttachmentState()
			.setBlendEnable(false)
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eA)
		;

	auto att_blends = core::range(usize(0), render_pass.attachment_count()).map([=](usize) { return color_blend_attachment; }).collect<core::Vector>();

	auto color_blending = vk::PipelineColorBlendStateCreateInfo()
			.setLogicOpEnable(false)
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachmentCount(u32(att_blends.size()))
			.setPAttachments(att_blends.begin())
			.setBlendConstants({{0.0f, 0.0f, 0.0f, 0.0f}})
		;

	auto depth_testing = vk::PipelineDepthStencilStateCreateInfo()
			.setDepthTestEnable(true)
			.setDepthWriteEnable(true)
			.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
		;

	auto pipeline_layout = device()->vk_device().createPipelineLayout(vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(u32(program.descriptor_layouts().size()))
			.setPSetLayouts(program.descriptor_layouts().begin())
		);

	auto pipeline = device()->vk_device().createGraphicsPipeline(VK_NULL_HANDLE, vk::GraphicsPipelineCreateInfo()
			.setStageCount(u32(pipeline_shader_stage.size()))
			.setPStages(pipeline_shader_stage.begin())
			.setPVertexInputState(&vertex_input)
			.setPInputAssemblyState(&input_assembly)
			.setPViewportState(&viewport_state)
			.setPRasterizationState(&rasterizer)
			.setPMultisampleState(&multisampling)
			.setPColorBlendState(&color_blending)
			.setPDepthStencilState(&depth_testing)
			.setLayout(pipeline_layout)
			.setRenderPass(render_pass.vk_render_pass())
			.setSubpass(0)
			.setBasePipelineIndex(-1)
		);

	return GraphicPipeline(material, pipeline, pipeline_layout);
}

}
