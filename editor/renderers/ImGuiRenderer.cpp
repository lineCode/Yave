/*******************************
Copyright (c) 2016-2019 Gr�goire Angerand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
**********************************/

#include "ImGuiRenderer.h"

#include <imgui/imgui.h>

#include <yave/graphics/buffers/TypedWrapper.h>
#include <yave/framegraph/FrameGraph.h>

#include <y/core/Chrono.h>
#include <y/io/File.h>

namespace editor {

static constexpr usize imgui_index_buffer_size = 64 * 1024;
static constexpr usize imgui_vertex_buffer_size = 64 * 1024;

static ImageData load_font() {
	y_profile();

	// https://skia.googlesource.com/external/github.com/ocornut/imgui/+/v1.50/extra_fonts/README.txt
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->AddFontDefault();
	if(io::File::exists("fonts/fa-solid-900.ttf")) {
		ImFontConfig config;
		config.MergeMode = true;
		const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
		io.Fonts->AddFontFromFileTTF("fonts/fa-solid-900.ttf", 13.0f, &config, icon_ranges);
	} else {
		log_msg("fonts/fa-solid-900.ttf not found.", Log::Error);
	}

	u8* font_data = nullptr;
	int width = 0;
	int height = 0;
	io.Fonts->GetTexDataAsRGBA32(&font_data, &width, &height);
	return ImageData(math::Vec2ui(width, height), font_data, ImageFormat(vk::Format::eR8G8B8A8Unorm));
}

template<typename = void>
static void setup_style() {

	ImGuiStyle& style = ImGui::GetStyle();
	style.ChildBorderSize = 0;
	style.PopupBorderSize = 0;
	style.FrameBorderSize = 0;
	style.TabBorderSize	= 0;

	style.PopupRounding = 0;
	style.WindowRounding = 0;
	style.ChildRounding = 0;
	style.FrameRounding = 3;
	style.TabRounding = 0;

	style.WindowMinSize = ImVec2(100, 100);

	// style not complete yet

#include "style.h"
}

ImGuiRenderer::ImGuiRenderer(DevicePtr dptr) :
		DeviceLinked(dptr),
		_index_buffer(imgui_index_buffer_size),
		_vertex_buffer(imgui_vertex_buffer_size),
		_uniform_buffer(device(), 1),
		_font(device(), load_font()),
		_font_view(_font) {

	ImGui::GetIO().Fonts->TexID = &_font_view;
	setup_style();
}

const DescriptorSet& ImGuiRenderer::create_descriptor_set(const void* data) {
	auto tex = data ? reinterpret_cast<const TextureView*>(data) : &_font_view;
	auto& ds = _descriptor_sets[tex->vk_view()];
	if(!ds.device()) {
		ds = DescriptorSet(device(), {Binding(*tex), Binding(_uniform_buffer)});
	}
	return ds;
}

void ImGuiRenderer::setup_state(RenderPassRecorder& recorder, const FrameToken& token, const void* tex) {
	recorder.bind_buffers(_index_buffer[token], {_vertex_buffer[token]});
	const auto* material = device()->device_resources()[DeviceResources::ImguiMaterialTemplate];
	recorder.bind_material(material, {create_descriptor_set(tex)});
}

void ImGuiRenderer::render(RenderPassRecorder& recorder, const FrameToken& token) {
	static_assert(sizeof(ImDrawVert) == sizeof(Vertex), "ImDrawVert is not of expected size");
	static_assert(sizeof(ImDrawIdx) == sizeof(u32), "16 bit indexes not supported");
	y_profile();

	auto region = recorder.region("ImGuiRenderer::render");

	ImDrawData* draw_data = ImGui::GetDrawData();

	if(!draw_data) {
		return;
	}

	auto index_subbuffer = _index_buffer[token];
	auto vertex_subbuffer = _vertex_buffer[token];
	auto indexes = TypedMapping(index_subbuffer);
	auto vertices = TypedMapping(vertex_subbuffer);
	auto uniform = TypedMapping(_uniform_buffer);

	uniform[0] = math::Vec2(ImGui::GetIO().DisplaySize);

	usize index_offset = 0;
	usize vertex_offset = 0;
	const void* current_tex = nullptr;
	for(auto i = 0; i != draw_data->CmdListsCount; ++i) {
		const ImDrawList* cmd_list = draw_data->CmdLists[i];

		if(cmd_list->IdxBuffer.Size + index_offset >= index_subbuffer.size()) {
			y_fatal("Index buffer overflow.");
		}

		if(cmd_list->VtxBuffer.Size + vertex_offset >= vertex_subbuffer.size()) {
			y_fatal("Vertex buffer overflow.");
		}

		std::copy(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Data + cmd_list->IdxBuffer.Size, &indexes[index_offset]);
		std::copy(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Data + cmd_list->VtxBuffer.Size, reinterpret_cast<ImDrawVert*>(&vertices[vertex_offset]));

		u32 drawn_index_offset = index_offset;
		for(auto i = 0; i != cmd_list->CmdBuffer.Size; ++i) {
			const ImDrawCmd& cmd = cmd_list->CmdBuffer[i];

			vk::Offset2D offset(cmd.ClipRect.x, cmd.ClipRect.y);
			vk::Extent2D extent(cmd.ClipRect.z - cmd.ClipRect.x, cmd.ClipRect.w - cmd.ClipRect.y);
			recorder.vk_cmd_buffer().setScissor(0, vk::Rect2D(offset, extent));

			if(cmd.UserCallback) {
				void* ptr = reinterpret_cast<void*>(cmd.UserCallback);
				reinterpret_cast<UIDrawCallback>(ptr)(recorder, cmd.UserCallbackData);
				current_tex = nullptr;
			}

			if(cmd.ElemCount) {
				if(current_tex != cmd.TextureId) {
					setup_state(recorder, token, current_tex = cmd.TextureId);
				}
				recorder.draw(vk::DrawIndexedIndirectCommand()
						.setFirstIndex(drawn_index_offset)
						.setVertexOffset(vertex_offset)
						.setIndexCount(cmd.ElemCount)
						.setInstanceCount(1)
					);

				drawn_index_offset += cmd.ElemCount;
			}
		}

		vertex_offset += cmd_list->VtxBuffer.Size;
		index_offset += cmd_list->IdxBuffer.Size;
	}
}



/*static const Material& create_imgui_material(DevicePtr dptr) {
	static std::unique_ptr<Material> mat;
	if(!mat) {
		mat = std::make_unique<Material>(dptr, MaterialData()
				.set_frag_data(device()->default_resources()[DefaultResources::ImguiFrag])
				.set_vert_data(device()->default_resources()[DefaultResources::ImguiVert])
				.set_depth_tested(false)
				.set_culled(false)
				.set_blended(true)
			);
	}
	return *mat;
}

static TextureView create_imgui_font(DevicePtr dptr) {
	static std::unique_ptr<Texture> img;
	if(!img) {
		img = std::make_unique<Texture>(dptr, load_font());
	}
	return *img;
}


FrameGraphImageId render_imgui(FrameGraph& framegraph, const math::Vec2ui& size) {
	struct Vertex {
		math::Vec2 pos;
		math::Vec2 uv;
		u32 col;
	};

	static_assert(sizeof(ImDrawVert) == sizeof(Vertex), "ImDrawVert is not of expected size");
	static_assert(sizeof(ImDrawIdx) == sizeof(u32), "16 bit indexes not supported");

	ImDrawData* draw_data = ImGui::GetDrawData();

	if(!draw_data) {
		y_fatal("ImDrawData is null.");
	}


	auto output = framegraph.declare_image(vk::Format::eR8G8B8A8Unorm, size);
	auto index_buffer = framegraph.declare_typed_buffer<ImDrawIdx>(imgui_index_buffer_size);
	auto vertex_buffer = framegraph.declare_typed_buffer<ImDrawVert>(imgui_vertex_buffer_size);
	auto uniform_buffer = framegraph.declare_typed_buffer<math::Vec2ui>();

	FrameGraphPassBuilder builder = framegraph.add_pass("ImGui pass");
	builder.add_color_output(output);
	builder.add_index_input(index_buffer);
	builder.add_attrib_input(vertex_buffer);
	builder.add_uniform_input(uniform_buffer);
	builder.map_update(index_buffer);
	builder.map_update(vertex_buffer);
	builder.map_update(uniform_buffer);
	builder.set_render_func([=](CmdBufferRecorder& recorder, const FrameGraphPass* self) {

			auto indexes = self->resources()->get_mapped_buffer(index_buffer);
			auto vertices = self->resources()->get_mapped_buffer(vertex_buffer);

			self->resources()->get_mapped_buffer(uniform_buffer)[0] = math::Vec2(ImGui::GetIO().DisplaySize);

			auto render_pass = recorder.bind_framebuffer(self->framebuffer());
			render_pass.bind_material(create_imgui_material(recorder.device()), {self->descriptor_sets()[0]});

			usize index_offset = 0;
			usize vertex_offset = 0;
			const void* current_tex = nullptr;
			for(auto i = 0; i != draw_data->CmdListsCount; ++i) {
				const ImDrawList* cmd_list = draw_data->CmdLists[i];

				if(cmd_list->IdxBuffer.Size + index_offset >= indexes.size()) {
					y_fatal("Index buffer overflow.");
				}

				if(cmd_list->VtxBuffer.Size + vertex_offset >= vertices.size()) {
					y_fatal("Vertex buffer overflow.");
				}

				std::copy(cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Data + cmd_list->IdxBuffer.Size, &indexes[index_offset]);
				std::copy(cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Data + cmd_list->VtxBuffer.Size, reinterpret_cast<ImDrawVert*>(&vertices[vertex_offset]));

				u32 drawn_index_offset = index_offset;
				for(auto i = 0; i != cmd_list->CmdBuffer.Size; ++i) {
					const ImDrawCmd& cmd = cmd_list->CmdBuffer[i];

					vk::Offset2D offset(cmd.ClipRect.x, cmd.ClipRect.y);
					vk::Extent2D extent(cmd.ClipRect.z - cmd.ClipRect.x, cmd.ClipRect.w - cmd.ClipRect.y);
					recorder.vk_cmd_buffer().setScissor(0, vk::Rect2D(offset, extent));

					if(cmd.UserCallback) {
						void* ptr = reinterpret_cast<void*>(cmd.UserCallback);
						reinterpret_cast<UIDrawCallback>(ptr)(render_pass, cmd.UserCallbackData);
						current_tex = nullptr;
					}

					if(cmd.ElemCount) {
						if(current_tex != cmd.TextureId) {
							current_tex = cmd.TextureId;
							render_pass.bind_buffers(self->resources()->get_buffer<BufferUsage::IndexBit>(index_buffer), {self->resources()->get_buffer<BufferUsage::AttributeBit>(vertex_buffer)});
							const Material& material = create_imgui_material(recorder.device());
							if(current_tex) {
								y_fatal("Not supported.");
								//render_pass.bind_material(material, current_tex ???);
							} else {
								render_pass.bind_material(material, {self->descriptor_sets()[0]});
							}
						}
						render_pass.draw(vk::DrawIndexedIndirectCommand()
								.setFirstIndex(drawn_index_offset)
								.setVertexOffset(vertex_offset)
								.setIndexCount(cmd.ElemCount)
								.setInstanceCount(1)
							);

						drawn_index_offset += cmd.ElemCount;
					}
				}

				vertex_offset += cmd_list->VtxBuffer.Size;
				index_offset += cmd_list->IdxBuffer.Size;
			}
		});

	return output;
}*/

}
