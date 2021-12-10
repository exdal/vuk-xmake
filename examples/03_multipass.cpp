#include "example_runner.hpp"
#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

/* 03_multipass
* In this example we will build on the previous example (02_cube),
* but we will add in a second resource (a depth buffer). Furthermore we will see how to add multiple passes.
*
* These examples are powered by the example framework, which hides some of the code required, as that would be repeated for each example.
* Furthermore it allows launching individual examples and all examples with the example same code.
* Check out the framework (example_runner_*) files if interested!
*/

namespace {
	float angle = 0.f;
	auto box = util::generate_cube();

	vuk::Example x{
		.name = "03_multipass",
		.setup = [](vuk::ExampleRunner& runner, vuk::Allocator& allocator) {
			{
			vuk::PipelineBaseCreateInfo pci;
			pci.add_glsl(util::read_entire_file("../../examples/triangle.vert"), "triangle.vert");
			pci.add_glsl(util::read_entire_file("../../examples/triangle.frag"), "triangle.frag");
			runner.context->create_named_pipeline("triangle", pci);
			}
			{
			vuk::PipelineBaseCreateInfo pci;
			pci.add_glsl(util::read_entire_file("../../examples/ubo_test.vert"), "ubo_test.vert");
			pci.add_glsl(util::read_entire_file("../../examples/triangle_depthshaded.frag"), "triangle_depthshaded.frag");
			runner.context->create_named_pipeline("cube", pci);
			}
		},
		.render = [](vuk::ExampleRunner& runner, vuk::Allocator& frame_allocator) {
			vuk::Context& ctx = frame_allocator.get_context();

			// We set up the cube data, same as in example 02_cube
			auto [bverts, stub1] = ctx.create_buffer_gpu(frame_allocator, std::span(&box.first[0], box.first.size()));
			auto verts = *bverts;
			auto [binds, stub2] = ctx.create_buffer_gpu(frame_allocator, std::span(&box.second[0], box.second.size()));
			auto inds = *binds;
			struct VP {
				glm::mat4 view;
				glm::mat4 proj;
			} vp;
			vp.view = glm::lookAt(glm::vec3(0, 1.5, 3.5), glm::vec3(0), glm::vec3(0, 1, 0));
			vp.proj = glm::perspective(glm::degrees(70.f), 1.f, 1.f, 10.f);
			vp.proj[1][1] *= -1;

			auto [buboVP, stub3] = ctx.create_buffer_cross_device(frame_allocator, vuk::MemoryUsage::eCPUtoGPU, std::span(&vp, 1));
			auto uboVP = *buboVP;
			ctx.wait_all_transfers(frame_allocator);

			vuk::RenderGraph rg;
			// Add a pass to draw a triangle (from the first example) into the top left corner
			rg.add_pass({
				.resources = {"03_multipass_final"_image(vuk::eColorWrite)},
				.execute = [&](vuk::CommandBuffer& command_buffer) {
					command_buffer
						.set_viewport(0, vuk::Rect2D::relative(0, 0, 0.2f, 0.2f))
						.set_scissor(0, vuk::Rect2D::relative(0, 0, 0.2f, 0.2f))
						.set_rasterization({}) // Set the default rasterization state
						.broadcast_color_blend({}) // Set the default color blend state
						.bind_graphics_pipeline("triangle")
						.draw(3, 1, 0, 0);
					}
				}
			);

			// Add a pass to draw a triangle (from the first example) into the bottom right corner
			rg.add_pass({
				.resources = {"03_multipass_final"_image(vuk::eColorWrite)},
				.execute = [&](vuk::CommandBuffer& command_buffer) {
					command_buffer
						.set_viewport(0, vuk::Rect2D::relative(0.8f, 0.8f, 0.2f, 0.2f))
						.set_scissor(0, vuk::Rect2D::relative(0.8f, 0.8f, 0.2f, 0.2f))
						.set_rasterization({}) // Set the default rasterization state
						.broadcast_color_blend({}) // Set the default color blend state
						.bind_graphics_pipeline("triangle")
						.draw(3, 1, 0, 0);
					}
				}
			);

			// Add a pass to draw a cube (from the second example) in the middle, but with depth buffering
			rg.add_pass({
				// Here a second resource is added: a depth attachment
				// The example framework took care of our color image, but this attachment we will need bind later
				// Depth attachments are denoted by the use vuk::eDepthStencilRW
				.resources = {"03_multipass_final"_image(vuk::eColorWrite), "03_depth"_image(vuk::eDepthStencilRW)},
				.execute = [verts, uboVP, inds](vuk::CommandBuffer& command_buffer) {
					command_buffer
						.set_viewport(0, vuk::Rect2D::framebuffer())
						.set_scissor(0, vuk::Rect2D::framebuffer())
						.set_rasterization({}) // Set the default rasterization state
						// Set the depth/stencil state
						.set_depth_stencil(vuk::PipelineDepthStencilStateCreateInfo{
							.depthTestEnable = true,
							.depthCompareOp = vuk::CompareOp::eLessOrEqual
						})
						.broadcast_color_blend({}) // Set the default color blend state
						.bind_index_buffer(inds, vuk::IndexType::eUint32)
						.bind_graphics_pipeline("cube")
						.bind_vertex_buffer(0, verts, 0, vuk::Packed{vuk::Format::eR32G32B32Sfloat, vuk::Ignore{sizeof(util::Vertex) - sizeof(util::Vertex::position)}})
						.bind_uniform_buffer(0, 0, uboVP)
						.set_depth_stencil(vuk::PipelineDepthStencilStateCreateInfo{});
					glm::mat4* model = command_buffer.map_scratch_uniform_binding<glm::mat4>(0, 1);
					*model = static_cast<glm::mat4>(glm::angleAxis(glm::radians(angle), glm::vec3(0.f, 1.f, 0.f)));
					command_buffer.draw_indexed(box.second.size(), 1, 0, 0, 0);
					}
				}
			);

			angle += 360.f * ImGui::GetIO().DeltaTime;

			// The rendergraph has a reference to "03_depth" resource, so we must provide the attachment
			// In this case, the depth attachment is an "internal" attachment: 
			// we don't provide an input texture, nor do we want to save the results later
			// For an internal attachment, we need to provide the format, extents, sample count and clear value
			// This depth attachment will have extents matching the framebuffer (deduced from the color attachment)
			rg.attach_managed("03_depth", vuk::Format::eD32Sfloat, vuk::Dimension2D::framebuffer(), vuk::Samples::e1, vuk::ClearDepthStencil{ 1.0f, 0 });

			// Note that the three passes we given here are not ordered with respect to eachother
			// They all write to the color attachment, which gives no ordering
			// Since we render with no overlap, this is not a problem
			return rg;
		}
	};

	REGISTER_EXAMPLE(x);
}