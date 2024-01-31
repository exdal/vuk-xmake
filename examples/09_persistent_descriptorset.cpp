#include "example_runner.hpp"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/mat4x4.hpp>
#include <numeric>
#include <optional>
#include <random>
#include <stb_image.h>

/* 09_persistent_descriptorset
 * In this example we will see how to create persistent descriptorsets.
 * Normal descriptorsets are completely managed by vuk, and are cached based on their contents.
 * However, this behaviour is not helpful if you plan to keep the descriptorsets around, or if they have many elements (such as "bindless").
 * For these scenarios, you can create and explicitly manage descriptorsets.
 * Here we first generate two additional textures from the one we load: the first by Y flipping using blitting and the second by
 * running a compute shader on it. Afterwards we create the persistent set and write the three images into it.
 * Later, we draw three cubes and fetch the texture based on the base instance.
 *
 * These examples are powered by the example framework, which hides some of the code required, as that would be repeated for each example.
 * Furthermore it allows launching individual examples and all examples with the example same code.
 * Check out the framework (example_runner_*) files if interested!
 */

namespace {
	// The Y rotation angle of our cube
	float angle = 0.f;
	// Generate vertices and indices for the cube
	auto box = util::generate_cube();
	vuk::Unique<vuk::Buffer> verts, inds;
	vuk::Unique<vuk::Image> image_of_doge;
	vuk::Unique<vuk::ImageView> image_view_of_doge;
	vuk::ImageAttachment texture_of_doge;
	vuk::Unique<vuk::Image> image_of_doge_v1;
	vuk::Unique<vuk::ImageView> image_view_of_doge_v1;
	vuk::ImageAttachment texture_of_doge_v1;
	vuk::Unique<vuk::Image> image_of_doge_v2;
	vuk::Unique<vuk::ImageView> image_view_of_doge_v2;
	vuk::ImageAttachment texture_of_doge_v2;
	vuk::Unique<vuk::PersistentDescriptorSet> pda;

	vuk::Example xample{
		.name = "09_persistent_descriptorset",
		.setup =
		    [](vuk::ExampleRunner& runner, vuk::Allocator& allocator) {
		      vuk::Context& ctx = allocator.get_context();
		      {
			      vuk::PipelineBaseCreateInfo pci;
			      pci.add_glsl(util::read_entire_file((root / "examples/bindless.vert").generic_string()), (root / "examples/bindless.vert").generic_string());
			      pci.add_glsl(util::read_entire_file((root / "examples/triangle_tex_bindless.frag").generic_string()),
			                   (root / "examples/triangle_tex_bindless.frag").generic_string());
			      // Flag this binding as partially bound, so that we don't need to set all the array elements
			      pci.set_binding_flags(1, 0, vuk::DescriptorBindingFlagBits::ePartiallyBound);
			      // Set the binding #0 in set #1 as a variable count binding, and set the maximum number of descriptors
			      pci.set_variable_count_binding(1, 0, 1024);
			      runner.context->create_named_pipeline("bindless_cube", pci);
		      }

		      // creating a compute pipeline that inverts an image
		      {
			      vuk::PipelineBaseCreateInfo pbci;
			      pbci.add_glsl(util::read_entire_file((root / "examples/invert.comp").generic_string()), "examples/invert.comp");
			      runner.context->create_named_pipeline("invert", pbci);
		      }

		      // Use STBI to load the image
		      int x, y, chans;
		      auto doge_image = stbi_load((root / "examples/doge.png").generic_string().c_str(), &x, &y, &chans, 4);

		      // Similarly to buffers, we allocate the image and enqueue the upload
		      texture_of_doge = vuk::ImageAttachment::from_preset(
		          vuk::ImageAttachment::Preset::eMap2D, vuk::Format::eR8G8B8A8Srgb, vuk::Extent3D{ (unsigned)x, (unsigned)y, 1u }, vuk::Samples::e1);
		      texture_of_doge.level_count = 1;
		      auto [image, view, doge_src] = vuk::create_image_and_view_with_data(allocator, vuk::DomainFlagBits::eTransferOnTransfer, texture_of_doge, doge_image);
		      image_of_doge = std::move(image);
		      image_view_of_doge = std::move(view);
		      stbi_image_free(doge_image);

		      // We set up the cube data, same as in example 02_cube
		      auto [vert_buf, vert_fut] = create_buffer(allocator, vuk::MemoryUsage::eGPUonly, vuk::DomainFlagBits::eTransferOnGraphics, std::span(box.first));
		      verts = std::move(vert_buf);
		      auto [ind_buf, ind_fut] = create_buffer(allocator, vuk::MemoryUsage::eGPUonly, vuk::DomainFlagBits::eTransferOnGraphics, std::span(box.second));
		      inds = std::move(ind_buf);
		      // For the example, we just ask these that these uploads complete before moving on to rendering
		      // In an engine, you would integrate these uploads into some explicit system
		      runner.enqueue_setup(std::move(vert_fut));
		      runner.enqueue_setup(std::move(ind_fut));

		      // Let's create two variants of the doge image
		      texture_of_doge_v1 = texture_of_doge;
		      texture_of_doge_v1.usage = vuk::ImageUsageFlagBits::eTransferDst | vuk::ImageUsageFlagBits::eSampled;
		      image_of_doge_v1 = *vuk::allocate_image(allocator, texture_of_doge_v1);
		      texture_of_doge_v1.image = *image_of_doge_v1;
		      image_view_of_doge_v1 = *vuk::allocate_image_view(allocator, texture_of_doge_v1);
		      texture_of_doge_v2 = texture_of_doge;
		      texture_of_doge_v2.format = vuk::Format::eR8G8B8A8Unorm;
		      texture_of_doge_v2.usage = vuk::ImageUsageFlagBits::eStorage | vuk::ImageUsageFlagBits::eSampled;
		      image_of_doge_v2 = *vuk::allocate_image(allocator, texture_of_doge_v2);
		      texture_of_doge_v2.image = *image_of_doge_v2;
		      image_view_of_doge_v2 = *vuk::allocate_image_view(allocator, texture_of_doge_v2);

		      // Make a RenderGraph to process the loaded image
		      auto doge_v1 = vuk::declare_ia("09_doge_v1", texture_of_doge_v1);
		      auto doge_v2 = vuk::declare_ia("09_doge_v2", texture_of_doge_v2);

		      auto preprocess = vuk::make_pass(
		          "preprocess",
		          [x, y](vuk::CommandBuffer& command_buffer,
		                 VUK_IA(vuk::eTransferRead | vuk::eComputeSampled) src,
		                 VUK_IA(vuk::eTransferWrite) v1,
		                 VUK_IA(vuk::eComputeWrite) v2) {
			          // For the first image, flip the image on the Y axis using a blit
			          vuk::ImageBlit blit;
			          blit.srcSubresource.aspectMask = vuk::ImageAspectFlagBits::eColor;
			          blit.srcSubresource.baseArrayLayer = 0;
			          blit.srcSubresource.layerCount = 1;
			          blit.srcSubresource.mipLevel = 0;
			          blit.srcOffsets[0] = vuk::Offset3D{ 0, 0, 0 };
			          blit.srcOffsets[1] = vuk::Offset3D{ x, y, 1 };
			          blit.dstSubresource = blit.srcSubresource;
			          blit.dstOffsets[0] = vuk::Offset3D{ x, y, 0 };
			          blit.dstOffsets[1] = vuk::Offset3D{ 0, 0, 1 };
			          command_buffer.blit_image(src, v1, blit, vuk::Filter::eLinear);
			          // For the second image, invert the colours in compute
			          command_buffer.bind_image(0, 0, src).bind_sampler(0, 0, {}).bind_image(0, 1, v2).bind_compute_pipeline("invert").dispatch_invocations(x, y);

			          return std::make_tuple(src, v1, v2);
		          });
		      // Bind the resources for the variant generation
		      // We specify the initial and final access
		      // The texture we have created is already in ShaderReadOptimal, but we need it in General during the pass, and we need it back to ShaderReadOptimal
		      // afterwards
		      auto [src, v1, v2] = preprocess(std::move(doge_src), std::move(doge_v1), std::move(doge_v2));
		      src.release(vuk::Access::eFragmentSampled, vuk::DomainFlagBits::eGraphicsQueue);
		      v1.release(vuk::Access::eFragmentSampled, vuk::DomainFlagBits::eGraphicsQueue);
		      v2.release(vuk::Access::eFragmentSampled, vuk::DomainFlagBits::eGraphicsQueue);
		      // enqueue running the preprocessing rendergraph and force 09_doge to be sampleable later
		      runner.enqueue_setup(std::move(src));
		      runner.enqueue_setup(std::move(v1));
		      runner.enqueue_setup(std::move(v2));
		      // Create persistent descriptorset for a pipeline and set index
		      pda = ctx.create_persistent_descriptorset(allocator, *runner.context->get_named_pipeline("bindless_cube"), 1, 64);
		      vuk::Sampler default_sampler = ctx.acquire_sampler({}, ctx.get_frame_count());
		      // Enqueue updates to the descriptors in the array
		      // This records the writes internally, but does not execute them
		      // Updating can be done in parallel from different threads, only the commit call has to be synchronized
		      pda->update_combined_image_sampler(0, 0, *image_view_of_doge, default_sampler, vuk::ImageLayout::eReadOnlyOptimalKHR);
		      pda->update_combined_image_sampler(0, 1, *image_view_of_doge_v1, default_sampler, vuk::ImageLayout::eReadOnlyOptimalKHR);
		      pda->update_combined_image_sampler(0, 2, *image_view_of_doge_v2, default_sampler, vuk::ImageLayout::eReadOnlyOptimalKHR);
		      // Execute the writes
		      pda->commit(ctx);
		    },
		.render =
		    [](vuk::ExampleRunner& runner, vuk::Allocator& frame_allocator, vuk::Value<vuk::ImageAttachment> target) {
		      struct VP {
			      glm::mat4 view;
			      glm::mat4 proj;
		      } vp;
		      vp.view = glm::lookAt(glm::vec3(0, 1.5, 3.5), glm::vec3(0), glm::vec3(0, 1, 0));
		      vp.proj = glm::perspective(glm::degrees(70.f), 1.f, 1.f, 10.f);
		      vp.proj[1][1] *= -1;

		      auto [buboVP, uboVP_fut] = create_buffer(frame_allocator, vuk::MemoryUsage::eCPUtoGPU, vuk::DomainFlagBits::eTransferOnGraphics, std::span(&vp, 1));
		      auto uboVP = *buboVP;

		      // Set up the pass to draw the textured cube, with a color and a depth attachment
		      auto forward_pass =
		          vuk::make_pass("forward", [uboVP](vuk::CommandBuffer& command_buffer, VUK_IA(vuk::eColorWrite) color, VUK_IA(vuk::eDepthStencilRW) depth) {
			          command_buffer.set_viewport(0, vuk::Rect2D::framebuffer())
			              .set_scissor(0, vuk::Rect2D::framebuffer())
			              .set_rasterization({}) // Set the default rasterization state
			              // Set the depth/stencil state
			              .set_depth_stencil(vuk::PipelineDepthStencilStateCreateInfo{
			                  .depthTestEnable = true,
			                  .depthWriteEnable = true,
			                  .depthCompareOp = vuk::CompareOp::eLessOrEqual,
			              })
			              .broadcast_color_blend({}) // Set the default color blend state
			              .bind_vertex_buffer(0,
			                                  *verts,
			                                  0,
			                                  vuk::Packed{ vuk::Format::eR32G32B32Sfloat,
			                                               vuk::Ignore{ offsetof(util::Vertex, uv_coordinates) - sizeof(util::Vertex::position) },
			                                               vuk::Format::eR32G32Sfloat })
			              .bind_index_buffer(*inds, vuk::IndexType::eUint32)
			              .bind_persistent(1, pda.get())
			              .bind_graphics_pipeline("bindless_cube")
			              .bind_buffer(0, 0, uboVP);
			          glm::mat4* model = command_buffer.scratch_buffer<glm::mat4>(0, 1);
			          *model = static_cast<glm::mat4>(glm::angleAxis(glm::radians(angle), glm::vec3(0.f, 1.f, 0.f)));
			          // Draw 3 cubes, assign them different base instance to identify them in the shader
			          command_buffer.draw_indexed(box.second.size(), 1, 0, 0, 0)
			              .draw_indexed(box.second.size(), 1, 0, 0, 1)
			              .draw_indexed(box.second.size(), 1, 0, 0, 2);
			          return color;
		          });

		      angle += 10.f * ImGui::GetIO().DeltaTime;

		      auto depth_img = vuk::declare_ia("09_depth");
		      depth_img->format = vuk::Format::eD32Sfloat;
		      depth_img = vuk::clear_image(std::move(depth_img), vuk::ClearDepthStencil{ 1.0f, 0 });

		      return forward_pass(std::move(target), std::move(depth_img));
		    },

		// Perform cleanup for the example
		.cleanup =
		    [](vuk::ExampleRunner& runner, vuk::Allocator& frame_allocator) {
		      // We release the resources manually
		      verts.reset();
		      inds.reset();
		      image_of_doge.reset();
		      image_view_of_doge.reset();
		      image_of_doge_v1.reset();
		      image_view_of_doge_v1.reset();
		      image_of_doge_v2.reset();
		      image_view_of_doge_v2.reset();
		      pda.reset();
		    }
	};

	REGISTER_EXAMPLE(xample);
} // namespace