#pragma once

#include "vuk/Config.hpp"
#include "vuk/Exception.hpp"
#include "vuk/FixedVector.hpp"
#include "vuk/runtime/vk/Image.hpp"
#include "vuk/runtime/vk/PipelineInstance.hpp"
#include "vuk/runtime/vk/Query.hpp"
#include "vuk/Result.hpp"
#include "vuk/Types.hpp"
#include "vuk/vuk_fwd.hpp"

#include <optional>
#include <utility>

namespace vuk {
	class Runtime;

	struct Ignore {
		Ignore(size_t bytes) : format(Format::eUndefined), bytes((uint32_t)bytes) {}
		Ignore(Format format) : format(format) {}
		Format format;
		uint32_t bytes = 0;

		uint32_t to_size();
	};

	struct FormatOrIgnore {
		FormatOrIgnore(Format format);
		FormatOrIgnore(Ignore ign);

		bool ignore;
		Format format;
		uint32_t size;
	};

	struct Packed {
		Packed() {}
		Packed(std::initializer_list<FormatOrIgnore> ilist) : list(ilist) {}
		fixed_vector<FormatOrIgnore, VUK_MAX_ATTRIBUTES> list;
	};

	enum class VertexInputRate {
		eVertex = VK_VERTEX_INPUT_RATE_VERTEX,
		eInstance = VK_VERTEX_INPUT_RATE_INSTANCE,
	};

	struct DrawIndirectCommand {
		uint32_t vertexCount = {};
		uint32_t instanceCount = {};
		uint32_t firstVertex = {};
		uint32_t firstInstance = {};

		operator VkDrawIndirectCommand const&() const noexcept {
			return *reinterpret_cast<const VkDrawIndirectCommand*>(this);
		}

		operator VkDrawIndirectCommand&() noexcept {
			return *reinterpret_cast<VkDrawIndirectCommand*>(this);
		}

		bool operator==(VkDrawIndirectCommand const& rhs) const noexcept {
			return (vertexCount == rhs.vertexCount) && (instanceCount == rhs.instanceCount) && (firstVertex == rhs.firstVertex) &&
			       (firstInstance == rhs.firstInstance);
		}

		bool operator!=(VkDrawIndirectCommand const& rhs) const noexcept {
			return !operator==(rhs);
		}
	};
	static_assert(sizeof(DrawIndirectCommand) == sizeof(VkDrawIndirectCommand), "struct and wrapper have different size!");
	static_assert(std::is_standard_layout_v<DrawIndirectCommand>, "struct wrapper is not a standard layout!");

	struct DrawIndexedIndirectCommand {
		uint32_t indexCount = {};
		uint32_t instanceCount = {};
		uint32_t firstIndex = {};
		int32_t vertexOffset = {};
		uint32_t firstInstance = {};

		operator VkDrawIndexedIndirectCommand const&() const noexcept {
			return *reinterpret_cast<const VkDrawIndexedIndirectCommand*>(this);
		}

		operator VkDrawIndexedIndirectCommand&() noexcept {
			return *reinterpret_cast<VkDrawIndexedIndirectCommand*>(this);
		}

		bool operator==(DrawIndexedIndirectCommand const& rhs) const noexcept {
			return (indexCount == rhs.indexCount) && (instanceCount == rhs.instanceCount) && (firstIndex == rhs.firstIndex) && (vertexOffset == rhs.vertexOffset) &&
			       (firstInstance == rhs.firstInstance);
		}

		bool operator!=(DrawIndexedIndirectCommand const& rhs) const noexcept {
			return !operator==(rhs);
		}
	};
	static_assert(sizeof(DrawIndexedIndirectCommand) == sizeof(VkDrawIndexedIndirectCommand), "struct and wrapper have different size!");
	static_assert(std::is_standard_layout_v<DrawIndexedIndirectCommand>, "struct wrapper is not a standard layout!");

	struct ImageSubresourceLayers {
		ImageAspectFlags aspectMask = {};
		uint32_t mipLevel = 0;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;

		operator VkImageSubresourceLayers const&() const noexcept {
			return *reinterpret_cast<const VkImageSubresourceLayers*>(this);
		}

		operator VkImageSubresourceLayers&() noexcept {
			return *reinterpret_cast<VkImageSubresourceLayers*>(this);
		}

		bool operator==(ImageSubresourceLayers const& rhs) const noexcept {
			return (aspectMask == rhs.aspectMask) && (mipLevel == rhs.mipLevel) && (baseArrayLayer == rhs.baseArrayLayer) && (layerCount == rhs.layerCount);
		}

		bool operator!=(ImageSubresourceLayers const& rhs) const noexcept {
			return !operator==(rhs);
		}
	};
	static_assert(sizeof(ImageSubresourceLayers) == sizeof(VkImageSubresourceLayers), "struct and wrapper have different size!");
	static_assert(std::is_standard_layout_v<ImageSubresourceLayers>, "struct wrapper is not a standard layout!");

	struct ImageBlit {
		ImageSubresourceLayers srcSubresource = {};
		std::array<Offset3D, 2> srcOffsets = {};
		ImageSubresourceLayers dstSubresource = {};
		std::array<Offset3D, 2> dstOffsets = {};

		operator VkImageBlit const&() const noexcept {
			return *reinterpret_cast<const VkImageBlit*>(this);
		}

		operator VkImageBlit&() noexcept {
			return *reinterpret_cast<VkImageBlit*>(this);
		}

		bool operator==(ImageBlit const& rhs) const noexcept {
			return (srcSubresource == rhs.srcSubresource) && (srcOffsets == rhs.srcOffsets) && (dstSubresource == rhs.dstSubresource) &&
			       (dstOffsets == rhs.dstOffsets);
		}

		bool operator!=(ImageBlit const& rhs) const noexcept {
			return !operator==(rhs);
		}
	};
	static_assert(sizeof(ImageBlit) == sizeof(VkImageBlit), "struct and wrapper have different size!");
	static_assert(std::is_standard_layout_v<ImageBlit>, "struct wrapper is not a standard layout!");

	struct ImageCopy {
		ImageSubresourceLayers srcSubresource = {};
		Offset3D srcOffsets = {};
		ImageSubresourceLayers dstSubresource = {};
		Offset3D dstOffsets = {};
		Extent3D imageExtent = {};

		operator VkImageCopy const&() const noexcept {
			return *reinterpret_cast<const VkImageCopy*>(this);
		}

		operator VkImageCopy&() noexcept {
			return *reinterpret_cast<VkImageCopy*>(this);
		}

		bool operator==(ImageCopy const& rhs) const noexcept {
			return (srcSubresource == rhs.srcSubresource) && (srcOffsets == rhs.srcOffsets) && (dstSubresource == rhs.dstSubresource) &&
			       (dstOffsets == rhs.dstOffsets) && (imageExtent == rhs.imageExtent);
		}

		bool operator!=(ImageCopy const& rhs) const noexcept {
			return !operator==(rhs);
		}
	};

	static_assert(sizeof(ImageCopy) == sizeof(VkImageCopy), "struct and wrapper have different size!");
	static_assert(std::is_standard_layout_v<ImageCopy>, "struct wrapper is not a standard layout!");

	struct BufferImageCopy {
		VkDeviceSize bufferOffset = {};
		uint32_t bufferRowLength = {};
		uint32_t bufferImageHeight = {};
		ImageSubresourceLayers imageSubresource = {};
		Offset3D imageOffset = {};
		Extent3D imageExtent = {};

		operator VkBufferImageCopy const&() const noexcept {
			return *reinterpret_cast<const VkBufferImageCopy*>(this);
		}

		operator VkBufferImageCopy&() noexcept {
			return *reinterpret_cast<VkBufferImageCopy*>(this);
		}

		bool operator==(BufferImageCopy const& rhs) const noexcept {
			return (bufferOffset == rhs.bufferOffset) && (bufferRowLength == rhs.bufferRowLength) && (bufferImageHeight == rhs.bufferImageHeight) &&
			       (imageSubresource == rhs.imageSubresource) && (imageOffset == rhs.imageOffset) && (imageExtent == rhs.imageExtent);
		}

		bool operator!=(BufferImageCopy const& rhs) const noexcept {
			return !operator==(rhs);
		}
	};
	static_assert(sizeof(BufferImageCopy) == sizeof(VkBufferImageCopy), "struct and wrapper have different size!");
	static_assert(std::is_standard_layout_v<BufferImageCopy>, "struct wrapper is not a standard layout!");

	struct ExecutableRenderGraph;
	struct ImageAttachment;
	struct PassInfo;
	struct Query;
	class Allocator;

	class CommandBuffer {
	protected:
		friend struct ExecutableRenderGraph;
		Runtime& ctx;
		Allocator* allocator;
		CommandBufferAllocation command_buffer_allocation;
		VkCommandBuffer command_buffer;

		struct RenderPassInfo {
			VkRenderPass render_pass;
			uint32_t subpass;
			Extent2D extent;
			SampleCountFlagBits samples;
			VkAttachmentReference const* depth_stencil_attachment;
			std::array<ImageView, VUK_MAX_COLOR_ATTACHMENTS> color_attachment_ivs = {};
			std::span<const VkAttachmentReference> color_attachments;
		};
		std::optional<RenderPassInfo> ongoing_render_pass;
		struct Stream* stream;

		Result<void> current_error = { expected_value };

		// Pipeline state
		// Enabled dynamic state
		DynamicStateFlags dynamic_state_flags = {};

		// Current & next graphics & compute pipelines
		PipelineBaseInfo* next_pipeline = nullptr;
		PipelineBaseInfo* next_compute_pipeline = nullptr;
		PipelineBaseInfo* next_ray_tracing_pipeline = nullptr;
		std::optional<GraphicsPipelineInfo> current_graphics_pipeline;
		std::optional<ComputePipelineInfo> current_compute_pipeline;
		std::optional<RayTracingPipelineInfo> current_ray_tracing_pipeline;

		// Input assembly & fixed-function attributes
		PrimitiveTopology topology = PrimitiveTopology::eTriangleList;
		Bitset<VUK_MAX_ATTRIBUTES> set_attribute_descriptions = {};
		VertexInputAttributeDescription attribute_descriptions[VUK_MAX_ATTRIBUTES];
		Bitset<VUK_MAX_ATTRIBUTES> set_binding_descriptions = {};
		VkVertexInputBindingDescription binding_descriptions[VUK_MAX_ATTRIBUTES];

		// Specialization constant support
		struct SpecEntry {
			bool is_double;
			std::byte data[sizeof(double)];
		};
		std::unordered_map<uint32_t, SpecEntry> spec_map_entries; // constantID -> SpecEntry

		// Individual pipeline states
		std::optional<PipelineRasterizationStateCreateInfo> rasterization_state;
		std::optional<PipelineDepthStencilStateCreateInfo> depth_stencil_state;
		std::optional<PipelineRasterizationConservativeStateCreateInfo> conservative_state;
		bool broadcast_color_blend_attachment_0 = false;
		Bitset<VUK_MAX_COLOR_ATTACHMENTS> set_color_blend_attachments = {};
		fixed_vector<PipelineColorBlendAttachmentState, VUK_MAX_COLOR_ATTACHMENTS> color_blend_attachments;
		std::optional<std::array<float, 4>> blend_constants;
		float line_width = 1.0f;
		fixed_vector<VkViewport, VUK_MAX_VIEWPORTS> viewports;
		fixed_vector<VkRect2D, VUK_MAX_SCISSORS> scissors;

		// Push constants
		unsigned char push_constant_buffer[VUK_MAX_PUSHCONSTANT_SIZE];
		fixed_vector<VkPushConstantRange, VUK_MAX_PUSHCONSTANT_RANGES> pcrs;

		// Descriptor sets
		DescriptorSetStrategyFlags ds_strategy_flags = {};
		Bitset<VUK_MAX_SETS> sets_used = {};
		VkDescriptorSetLayout set_layouts_used[VUK_MAX_SETS] = {};
		Bitset<VUK_MAX_SETS> sets_to_bind = {};
		SetBinding set_bindings[VUK_MAX_SETS] = {};
		Bitset<VUK_MAX_SETS> persistent_sets_to_bind = {};
		std::pair<VkDescriptorSet, VkDescriptorSetLayout> persistent_sets[VUK_MAX_SETS] = {};

		CommandBuffer(Stream& stream, Runtime& ctx, Allocator& allocator, VkCommandBuffer cb, std::optional<RenderPassInfo> ongoing = {});
	public:
		/// @brief Retrieve parent runtime
		Runtime& get_context() {
			return ctx;
		}

		VkCommandBuffer get_underlying() const {
			return command_buffer;
		}
		/// @brief Retrieve information about the current renderpass
		const RenderPassInfo& get_ongoing_render_pass() const;

		Stream& get_scheduled_stream() {
			return *stream;
		}

		DomainFlagBits get_scheduled_domain() const;

		// command buffer state setting
		// when a state is set it is persistent for a pass (similar to Vulkan dynamic state) - see documentation

		/// @brief Set the strategy for allocating and updating ephemeral descriptor sets
		/// @param ds_strategy_flags Mask of strategy options
		///
		/// The default strategy is taken from the runtime when entering a new Pass
		CommandBuffer& set_descriptor_set_strategy(DescriptorSetStrategyFlags ds_strategy_flags);

		/// @brief Set mask of dynamic state in CommandBuffer
		/// @param dynamic_state_flags Mask of states (flag set = dynamic, flag clear = static)
		CommandBuffer& set_dynamic_state(DynamicStateFlags dynamic_state_flags);

		/// @brief Set the viewport transformation for the specified viewport index
		/// @param index viewport index to modify
		/// @param vp Viewport to be set
		CommandBuffer& set_viewport(unsigned index, Viewport vp);
		/// @brief Set the viewport transformation for the specified viewport index from a rect
		/// @param index viewport index to modify
		/// @param area Rect2D extents of the Viewport
		/// @param min_depth Minimum depth of Viewport
		/// @param max_depth Maximum depth of Viewport
		CommandBuffer& set_viewport(unsigned index, Rect2D area, float min_depth = 0.f, float max_depth = 1.f);
		/// @brief Set the scissor for the specified scissor index from a rect
		/// @param index scissor index to modify
		/// @param area Rect2D extents of the scissor
		CommandBuffer& set_scissor(unsigned index, Rect2D area);

		/// @brief Set the rasterization state
		CommandBuffer& set_rasterization(PipelineRasterizationStateCreateInfo rasterization_state);
		/// @brief Set the depth/stencil state
		CommandBuffer& set_depth_stencil(PipelineDepthStencilStateCreateInfo depth_stencil_state);
		/// @brief Set the conservative rasterization state
		CommandBuffer& set_conservative(PipelineRasterizationConservativeStateCreateInfo conservative_state);

		/// @brief Set one color blend state to use for all color attachments
		CommandBuffer& broadcast_color_blend(PipelineColorBlendAttachmentState color_blend_state);
		/// @brief Set one color blend preset to use for all color attachments
		CommandBuffer& broadcast_color_blend(BlendPreset blend_preset);
		/// @brief Set color blend state for a specific color attachment
		/// @param color_attachment the Name of the color_attachment to set the blend state for
		/// @param color_blend_state PipelineColorBlendAttachmentState to use
		CommandBuffer& set_color_blend(const ImageAttachment& color_attachment, PipelineColorBlendAttachmentState color_blend_state);
		/// @brief Set color blend preset for a specific color attachment
		/// @param color_attachment the Name of the color_attachment to set the blend preset for
		/// @param blend_preset BlendPreset to use
		CommandBuffer& set_color_blend(const ImageAttachment& color_attachment, BlendPreset blend_preset);
		/// @brief Set blend constants
		CommandBuffer& set_blend_constants(std::array<float, 4> blend_constants);

		/// @brief Bind a graphics pipeline for subsequent draws
		/// @param pipeline_base pointer to a pipeline base to bind
		CommandBuffer& bind_graphics_pipeline(PipelineBaseInfo* pipeline_base);
		/// @brief Bind a named graphics pipeline for subsequent draws
		/// @param named_pipeline graphics pipeline name
		CommandBuffer& bind_graphics_pipeline(Name named_pipeline);

		/// @brief Bind a compute pipeline for subsequent dispatches
		/// @param pipeline_base pointer to a pipeline base to bind
		CommandBuffer& bind_compute_pipeline(PipelineBaseInfo* pipeline_base);
		/// @brief Bind a named graphics pipeline for subsequent dispatches
		/// @param named_pipeline compute pipeline name
		CommandBuffer& bind_compute_pipeline(Name named_pipeline);

		/// @brief Bind a ray tracing pipeline for subsequent draws
		/// @param pipeline_base pointer to a pipeline base to bind
		CommandBuffer& bind_ray_tracing_pipeline(PipelineBaseInfo* pipeline_base);
		/// @brief Bind a named ray tracing pipeline for subsequent draws
		/// @param named_pipeline graphics pipeline name
		CommandBuffer& bind_ray_tracing_pipeline(Name named_pipeline);

		/// @brief Set specialization constants for the command buffer
		/// @param constant_id ID of the constant. All stages form a single namespace for IDs.
		/// @param value Value of the specialization constant
		CommandBuffer& specialize_constants(uint32_t constant_id, bool value);
		/// @brief Set specialization constants for the command buffer
		/// @param constant_id ID of the constant. All stages form a single namespace for IDs.
		/// @param value Value of the specialization constant
		CommandBuffer& specialize_constants(uint32_t constant_id, uint32_t value);
		/// @brief Set specialization constants for the command buffer
		/// @param constant_id ID of the constant. All stages form a single namespace for IDs.
		/// @param value Value of the specialization constant
		CommandBuffer& specialize_constants(uint32_t constant_id, int32_t value);
		/// @brief Set specialization constants for the command buffer
		/// @param constant_id ID of the constant. All stages form a single namespace for IDs.
		/// @param value Value of the specialization constant
		CommandBuffer& specialize_constants(uint32_t constant_id, float value);
		/// @brief Set specialization constants for the command buffer
		/// @param constant_id ID of the constant. All stages form a single namespace for IDs.
		/// @param value Value of the specialization constant
		CommandBuffer& specialize_constants(uint32_t constant_id, double value);

		/// @brief Set primitive topology
		CommandBuffer& set_primitive_topology(PrimitiveTopology primitive_topology);
		/// @brief Binds an index buffer with the given type
		/// @param buffer The buffer to be bound
		/// @param type The index type in the buffer
		CommandBuffer& bind_index_buffer(const Buffer& buffer, IndexType type);
		/// @brief Binds a vertex buffer to the given binding point and configures attributes sourced from this buffer based on a packed format list, the attribute
		/// locations are offset with first_location
		/// @param binding The binding point of the buffer
		/// @param buffer The buffer to be bound
		/// @param first_location First location assigned to the attributes
		/// @param format_list List of formats packed in buffer to generate attributes from
		CommandBuffer& bind_vertex_buffer(unsigned binding, const Buffer& buffer, unsigned first_location, Packed format_list, VertexInputRate input_rate = VertexInputRate::eVertex);
		/// @brief Binds a vertex buffer to the given binding point and configures attributes sourced from this buffer based on a span of attribute descriptions and
		/// stride
		/// @param binding The binding point of the buffer
		/// @param buffer The buffer to be bound
		/// @param attribute_descriptions Attributes that are sourced from this buffer
		/// @param stride Stride of a vertex sourced from this buffer
		CommandBuffer&
		bind_vertex_buffer(unsigned binding, const Buffer& buffer, std::span<VertexInputAttributeDescription> attribute_descriptions, uint32_t stride, VertexInputRate input_rate = VertexInputRate::eVertex);

		/// @brief Update push constants for the specified stages with bytes
		/// @param stages Pipeline stages that can see the updated bytes
		/// @param offset Offset into the push constant buffer
		/// @param data Pointer to data to be copied into push constants
		/// @param size Size of data
		CommandBuffer& push_constants(ShaderStageFlags stages, size_t offset, void* data, size_t size);
		/// @brief Update push constants for the specified stages with a span of values
		/// @tparam T type of values
		/// @param stages Pipeline stages that can see the updated bytes
		/// @param offset Offset into the push constant buffer
		/// @param span Values to write
		template<class T>
		CommandBuffer& push_constants(ShaderStageFlags stages, size_t offset, std::span<T> span);
		/// @brief Update push constants for the specified stages with a single value
		/// @tparam T type of value
		/// @param stages Pipeline stages that can see the updated bytes
		/// @param offset Offset into the push constant buffer
		/// @param value Value to write
		template<class T>
		CommandBuffer& push_constants(ShaderStageFlags stages, size_t offset, T value);

		/// @brief Bind a persistent descriptor set to the command buffer
		/// @param set The set bind index to be used
		/// @param desc_set The persistent descriptor set to be bound
		CommandBuffer& bind_persistent(unsigned set, PersistentDescriptorSet& desc_set);

		/// @brief Bind a buffer to the command buffer
		/// @param set The set bind index to be used
		/// @param binding The descriptor binding to bind the buffer to
		/// @param buffer The buffer to be bound
		CommandBuffer& bind_buffer(unsigned set, unsigned binding, const Buffer& buffer);

		/// @brief Bind a buffer to the command buffer
		/// @param set The set bind index to be used
		/// @param binding The descriptor binding to bind the buffer to
		/// @param buffer The buffer to be bound
		template<Access acc, class UniqueT>
		CommandBuffer& bind_buffer(unsigned set, unsigned binding, Arg<Buffer, acc, UniqueT> buffer) {
			return bind_buffer(set, binding, *buffer.ptr);
		}

		/// @brief Bind an image to the command buffer
		/// @param set The set bind index to be used
		/// @param binding The descriptor binding to bind the image to
		/// @param image_view The ImageView to bind
		/// @param layout layout of the image when the affected draws execute
		CommandBuffer& bind_image(unsigned set, unsigned binding, ImageView image_view, ImageLayout layout = ImageLayout::eReadOnlyOptimalKHR);

		/// @brief Bind an image to the command buffer
		/// @param set The set bind index to be used
		/// @param binding The descriptor binding to bind the image to
		/// @param image The ImageAttachment to bind
		CommandBuffer& bind_image(unsigned set, unsigned binding, const ImageAttachment& image);
		CommandBuffer& bind_image(unsigned set, unsigned binding, const ImageAttachment& image, Ref def);

		/// @brief Bind an image to the command buffer
		/// @param set The set bind index to be used
		/// @param binding The descriptor binding to bind the image to
		/// @param image The ImageAttachment to bind
		template<Access acc, class UniqueT>
		CommandBuffer& bind_image(unsigned set, unsigned binding, Arg<ImageAttachment, acc, UniqueT> image) {
			return bind_image(set, binding, *image.ptr, image.def);
		}

		/// @brief Bind a sampler to the command buffer from a Resource
		/// @param set The set bind index to be used
		/// @param binding The descriptor binding to bind the sampler to
		/// @param sampler_create_info Parameters of the sampler
		CommandBuffer& bind_sampler(unsigned set, unsigned binding, SamplerCreateInfo sampler_create_info);

		/// @brief Allocate some CPUtoGPU memory and bind it as a buffer. Return a pointer to the mapped memory.
		/// @param set The set bind index to be used
		/// @param binding The descriptor binding to bind the buffer to
		/// @param size Amount of memory to allocate
		/// @return pointer to the mapped host-visible memory. Null pointer if the command buffer has errored out previously or the allocation failed
		void* _scratch_buffer(unsigned set, unsigned binding, size_t size);

		/// @brief Allocate some typed CPUtoGPU memory and bind it as a buffer. Return a pointer to the mapped memory.
		/// @tparam T Type of the uniform to write
		/// @param set The set bind index to be used
		/// @param binding The descriptor binding to bind the buffer to
		/// @return pointer to the mapped host-visible memory. Null pointer if the command buffer has errored out previously or the allocation failed
		template<class T>
		T* scratch_buffer(unsigned set, unsigned binding);

		/// @brief Bind a sampler to the command buffer from a Resource
		/// @param set The set bind index to be used
		/// @param binding The descriptor binding to bind the sampler to
		/// @param sampler_create_info Parameters of the sampler
		CommandBuffer& bind_acceleration_structure(unsigned set, unsigned binding, VkAccelerationStructureKHR tlas);

		/// @brief Issue a non-indexed draw
		/// @param vertex_count Number of vertices to draw
		/// @param instance_count Number of instances to draw
		/// @param first_vertex Index of the first vertex to draw
		/// @param first_instance Index of the first instance to draw
		CommandBuffer& draw(size_t vertex_count, size_t instance_count, size_t first_vertex, size_t first_instance);

		/// @brief Issue an indirect draw
		/// @param command_count Number of indirect commands to be used
		/// @param indirect_buffer Buffer of indirect commands
		CommandBuffer& draw_indirect(size_t command_count, const Buffer& indirect_buffer);
		/// @brief Issue an indirect draw
		/// @param commands Indirect commands to be uploaded and used for this draw
		CommandBuffer& draw_indirect(std::span<DrawIndirectCommand> commands);

		/// @brief Issue an indirect draw with count
		/// @param max_command_count Upper limit of commands that can be drawn
		/// @param indirect_buffer Buffer of indirect commands
		/// @param count_buffer Buffer of command count
		CommandBuffer& draw_indirect_count(size_t max_draw_count, const Buffer& indirect_buffer, const Buffer& count_buffer);

		/// @brief Isuse an indexed draw
		/// @param index_count Number of vertices to draw
		/// @param instance_count Number of instances to draw
		/// @param first_index Index of the first index in the index buffer
		/// @param vertex_offset value added to the vertex index before indexing into the vertex buffer(s)
		/// @param first_instance Index of the first instance to draw
		CommandBuffer& draw_indexed(size_t index_count, size_t instance_count, size_t first_index, int32_t vertex_offset, size_t first_instance);

		/// @brief Issue an indirect indexed draw
		/// @param command_count Number of indirect commands to be used
		/// @param indirect_buffer Buffer of indirect commands
		CommandBuffer& draw_indexed_indirect(size_t command_count, const Buffer& indirect_buffer);
		/// @brief Issue an indirect indexed draw
		/// @param commands Indirect commands to be uploaded and used for this draw
		CommandBuffer& draw_indexed_indirect(std::span<DrawIndexedIndirectCommand> commands);

		/// @brief Issue an indirect indexed draw with count
		/// @param max_command_count Upper limit of commands that can be drawn
		/// @param indirect_buffer Buffer of indirect commands
		/// @param count_buffer Buffer of command count
		CommandBuffer& draw_indexed_indirect_count(size_t max_command_count, const Buffer& indirect_buffer, const Buffer& count_buffer);

		/// @brief Issue a compute dispatch
		/// @param group_count_x Number of groups on the x-axis
		/// @param group_count_y Number of groups on the y-axis
		/// @param group_count_z Number of groups on the z-axis
		CommandBuffer& dispatch(size_t group_count_x, size_t group_count_y = 1, size_t group_count_z = 1);

		/// @brief Perform a dispatch while specifying the minimum invocation count
		/// Actual invocation count will be rounded up to be a multiple of local_size_{x,y,z}
		/// @param invocation_count_x Number of invocations on the x-axis
		/// @param invocation_count_y Number of invocations on the y-axis
		/// @param invocation_count_z Number of invocations on the z-axis
		CommandBuffer& dispatch_invocations(size_t invocation_count_x, size_t invocation_count_y = 1, size_t invocation_count_z = 1);

		/// @brief Perform a dispatch with invocations per pixel
		/// The number of invocations per pixel can be scaled in all dimensions
		/// If the scale is == 1, then 1 invocations will be dispatched per pixel
		/// If the scale is larger than 1, then more invocations will be dispatched than pixels
		/// If the scale is smaller than 1, then fewer invocations will be dispatched than pixels
		/// Actual invocation count will be rounded up to be a multiple of local_size_{x,y,z} after scaling
		/// Width corresponds to the x-axis, height to the y-axis and depth to the z-axis
		/// @param ia ImageAttachment to use for extents
		/// @param invocations_per_pixel_scale_x Invocation count scale in x-axis
		/// @param invocations_per_pixel_scale_y Invocation count scale in y-axis
		/// @param invocations_per_pixel_scale_z Invocation count scale in z-axis
		CommandBuffer& dispatch_invocations_per_pixel(const ImageAttachment& ia,
		                                              float invocations_per_pixel_scale_x = 1.f,
		                                              float invocations_per_pixel_scale_y = 1.f,
		                                              float invocations_per_pixel_scale_z = 1.f);

		/// @brief Perform a dispatch with invocations per buffer element
		/// Actual invocation count will be rounded up to be a multiple of local_size_{x,y,z}
		/// The number of invocations per element can be scaled
		/// If the scale is == 1, then 1 invocations will be dispatched per element
		/// If the scale is larger than 1, then more invocations will be dispatched than element
		/// If the scale is smaller than 1, then fewer invocations will be dispatched than element
		/// The dispatch will be sized only on the x-axis
		/// @param buffer Buffer to use for calculating element count
		/// @param element_size Size of one element
		/// @param invocations_per_element_scale Invocation count scale
		CommandBuffer& dispatch_invocations_per_element(const Buffer& buffer, size_t element_size, float invocations_per_element_scale = 1.f);

		/// @brief Issue an indirect compute dispatch
		/// @param indirect_buffer Buffer of workgroup counts
		CommandBuffer& dispatch_indirect(const Buffer& indirect_buffer);

		/// @brief Perform ray trace query with a ray tracing pipeline
		/// @param width width of the ray trace query dimensions
		/// @param height height of the ray trace query dimensions
		/// @param depth depth of the ray trace query dimensions
		CommandBuffer& trace_rays(size_t width, size_t height, size_t depth);

		/// @brief Build acceleration structures
		CommandBuffer& build_acceleration_structures(uint32_t info_count,
		                                             const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
		                                             const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos);

		// commands for render pass-less command buffers

		/// @brief Clear an image
		/// @param dst the Name of the Resource to be cleared
		/// @param clear_value value to clear with
		CommandBuffer& clear_image(const ImageAttachment& dst, Clear clear_value);
		/// @brief Resolve an image
		/// @param src the Name of the multisampled Resource
		/// @param dst the Name of the singlesampled Resource
		CommandBuffer& resolve_image(const ImageAttachment& src, const ImageAttachment& dst);
		/// @brief Perform an image blit
		/// @param src the Name of the source Resource
		/// @param dst the Name of the destination Resource
		/// @param region parameters of the blit
		/// @param filter Filter to use if the src and dst extents differ
		CommandBuffer& blit_image(const ImageAttachment& src, const ImageAttachment& dst, ImageBlit region, Filter filter);
		/// @brief Perform an image copy
		/// @param src the Name of the source Resource
		/// @param dst the Name of the destination Resource
		/// @param region parameters of the copy
		CommandBuffer& copy_image(const ImageAttachment& src, const ImageAttachment& dst, ImageCopy region);
		/// @brief Copy a buffer resource into an image resource
		/// @param src the Name of the source Resource
		/// @param dst the Name of the destination Resource
		/// @param copy_params parameters of the copy
		CommandBuffer& copy_buffer_to_image(const Buffer& src, const ImageAttachment& dst, BufferImageCopy copy_params);
		/// @brief Copy an image resource into a buffer resource
		/// @param src the Name of the source Resource
		/// @param dst the Name of the destination Resource
		/// @param copy_params parameters of the copy
		CommandBuffer& copy_image_to_buffer(const ImageAttachment& src, const Buffer& dst, BufferImageCopy copy_params);
		/// @brief Copy between two buffer resource
		/// @param src the Name of the source Resource
		/// @param dst the Name of the destination Resource
		/// @param size number of bytes to copy (VK_WHOLE_SIZE to copy the entire "src" buffer)
		CommandBuffer& copy_buffer(const ImageAttachment& src, const ImageAttachment& dst, size_t size);
		/// @brief Copy between two Buffers
		/// @param src the source Buffer
		/// @param dst the destination Buffer
		CommandBuffer& copy_buffer(const Buffer& src, const Buffer& dst);
		/// @brief Fill a buffer with a fixed value
		/// @param dst the destination Buffer
		/// @param data the 4 byte value to fill with
		CommandBuffer& fill_buffer(const Buffer& dst, uint32_t data);
		/// @brief Fill a buffer with a host values
		/// @param dst the destination Buffer
		/// @param data pointer to host values
		CommandBuffer& update_buffer(const Buffer& dst, const void* data);

		// explicit synchronisation

		/// @brief Issue a memory barrier
		/// @param src_access previous Access
		/// @param dst_access subsequent Access
		CommandBuffer& memory_barrier(Access src_access, Access dst_access);
		/// @brief Issue an image barrier for an image resource
		/// @param resource_name the Name of the image Resource
		/// @param src_access previous Access
		/// @param dst_access subsequent Access
		/// @param base_level base mip level affected by the barrier
		/// @param level_count number of mip levels affected by the barrier
		CommandBuffer& image_barrier(const ImageAttachment& resource_name,
		                             Access src_access,
		                             Access dst_access,
		                             uint32_t base_level = 0,
		                             uint32_t level_count = VK_REMAINING_MIP_LEVELS);

		// queries

		/// @brief Write a timestamp to given Query
		/// @param query the Query to hold the result
		/// @param stage the pipeline stage where the timestamp should latch the earliest
		CommandBuffer& write_timestamp(Query query, PipelineStageFlagBits stage = PipelineStageFlagBits::eBottomOfPipe);

		// error handling
		[[nodiscard]] Result<void> result();

		// explicit command buffer access

		/// @brief Bind all pending compute state and return a raw VkCommandBuffer for direct access
		[[nodiscard]] VkCommandBuffer bind_compute_state();
		/// @brief Bind all pending graphics state and return a raw VkCommandBuffer for direct access
		[[nodiscard]] VkCommandBuffer bind_graphics_state();
		/// @brief Bind all pending ray tracing state and return a raw VkCommandBuffer for direct access
		[[nodiscard]] VkCommandBuffer bind_ray_tracing_state();

	protected:
		enum class PipeType { eGraphics, eCompute, eRayTracing };

		[[nodiscard]] bool _bind_state(PipeType pipe_type);
		[[nodiscard]] bool _bind_compute_pipeline_state();
		[[nodiscard]] bool _bind_graphics_pipeline_state();
		[[nodiscard]] bool _bind_ray_tracing_pipeline_state();

		CommandBuffer& specialize_constants(uint32_t constant_id, void* data, size_t size);
	};

	template<class T>
	inline CommandBuffer& CommandBuffer::push_constants(ShaderStageFlags stages, size_t offset, std::span<T> span) {
		return push_constants(stages, offset, (void*)span.data(), sizeof(T) * span.size());
	}

	template<class T>
	inline CommandBuffer& CommandBuffer::push_constants(ShaderStageFlags stages, size_t offset, T value) {
		return push_constants(stages, offset, (void*)&value, sizeof(T));
	}

	inline CommandBuffer& CommandBuffer::specialize_constants(uint32_t constant_id, bool value) {
		return specialize_constants(constant_id, (uint32_t)value);
	}

	inline CommandBuffer& CommandBuffer::specialize_constants(uint32_t constant_id, uint32_t value) {
		return specialize_constants(constant_id, (void*)&value, sizeof(uint32_t));
	}

	inline CommandBuffer& CommandBuffer::specialize_constants(uint32_t constant_id, int32_t value) {
		return specialize_constants(constant_id, (void*)&value, sizeof(int32_t));
	}

	inline CommandBuffer& CommandBuffer::specialize_constants(uint32_t constant_id, float value) {
		return specialize_constants(constant_id, (void*)&value, sizeof(float));
	}

	inline CommandBuffer& CommandBuffer::specialize_constants(uint32_t constant_id, double value) {
		return specialize_constants(constant_id, (void*)&value, sizeof(double));
	}

	template<class T>
	inline T* CommandBuffer::scratch_buffer(unsigned set, unsigned binding) {
		return static_cast<T*>(_scratch_buffer(set, binding, sizeof(T)));
	}

	/// @brief RAII utility for creating a timed scope on the GPU
	struct TimedScope {
		TimedScope(CommandBuffer& cbuf, Query a, Query b) : cbuf(cbuf), a(a), b(b) {
			cbuf.write_timestamp(a, PipelineStageFlagBits::eBottomOfPipe);
		}

		~TimedScope() {
			cbuf.write_timestamp(b, PipelineStageFlagBits::eBottomOfPipe);
		}

		CommandBuffer& cbuf;
		Query a;
		Query b;
	};
} // namespace vuk
