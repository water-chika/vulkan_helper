#pragma once

#include <vulkan/vulkan.hpp>

#include "spirv_helper.hpp"

#include <concepts>
#include <map>
#include <numeric>

namespace vulkan_hpp_helper {
    namespace concept_helper {
        template<class Instance>
        concept instance = requires (Instance instance) {
            { instance.get_instance() }->std::convertible_to<vk::Instance>;
        };
        namespace instance_helper {
            template<class T>
            concept get_extensions = requires(T t) {
                t.get_extensions();
            };
        }
        template<class T>
        concept physical_device = requires (T t) {
            { t.get_physical_device() } -> std::convertible_to<vk::PhysicalDevice>;
        };
        template<class T>
        concept device = requires (T t) {
            { t.get_device() } -> std::convertible_to<vk::Device>;
        };
    }
    class empty_class {};
    template<concept_helper::instance_helper::get_extensions T>
    class add_instance : public T {
    public:
        using parent = T;
        add_instance() {
            auto app_info = vk::ApplicationInfo{}.setApiVersion(vk::ApiVersion13);
            auto exts = parent::get_extensions();
            auto create_info = vk::InstanceCreateInfo{}.setPApplicationInfo(&app_info).setPEnabledExtensionNames(exts);
            m_instance = vk::createInstance(create_info);
        }
        ~add_instance() {
            m_instance.destroy();
        }
        auto get_instance() {
            return m_instance;
        }
    private:
        vk::Instance m_instance;
    };
    template<class T>
    class add_empty_extensions : public T {
    public:
        auto get_extensions() {
            return std::vector<const char*>{};
        }
    };
    template<class T>
    class add_surface_extension : public T {
    public:
        auto get_extensions() {
            auto ext = T::get_extensions();
            ext.push_back(vk::KHRSurfaceExtensionName);
            return ext;
        }
    };
    template<class T>
    class add_win32_surface_extension : public T {
    public:
        auto get_extensions() {
            auto ext = T::get_extensions();
            ext.push_back(vk::KHRWin32SurfaceExtensionName);
            return ext;
        }
    };
    template<class T>
    class add_swapchain_extension : public T {
    public:
        auto get_extensions() {
            auto ext = T::get_extensions();
            ext.push_back(vk::KHRSwapchainExtensionName);
            return ext;
        }
    };
    template<concept_helper::instance instance>
    class add_first_physical_device : public instance {
    public:
        using parent = instance;
        add_first_physical_device() : instance{}
        {
            m_physical_device = parent::get_instance().enumeratePhysicalDevices()[0];
        }
        auto get_physical_device() {
            return m_physical_device;
        }
    private:
        vk::PhysicalDevice m_physical_device;
    };

    template<concept_helper::device Device>
    class add_fence : public Device {
    public:
        using parent = Device;
        add_fence() : Device{} {
            vk::Device device = parent::get_device();
            device.createFence(vk::FenceCreateInfo{});
        }
        ~add_fence() {
            vk::Device device = parent::get_device();
            device.destroyFence(m_fence);
        }
        auto get_fence() {
            return m_fence;
        }
    private:
        vk::Fence m_fence;
    };

	template<class T>
	class add_physical_device : public T {
	public:
		using parent = T;
		add_physical_device() {
			vk::Instance instance = parent::get_instance();
			m_physical_device = instance.enumeratePhysicalDevices()[0];
		}
		auto get_physical_device() {
			return m_physical_device;
		}
	private:
		vk::PhysicalDevice m_physical_device;
	};
	template<class T>
	class add_queue_family_index : public T {
	public:
		uint32_t get_queue_family_index() {
			return 0;
		}
	};
	template<class T>
	class add_device : public T {
	public:
		using parent = T;
		add_device() {
			vk::PhysicalDevice physical_device = parent::get_physical_device();
			auto priorities = std::vector{
				1.0f
			};
			uint32_t queue_family_index = parent::get_queue_family_index();
			auto queue_create_infos = std::vector{
				vk::DeviceQueueCreateInfo{}
				.setQueueCount(priorities.size())
				.setQueuePriorities(priorities)
				.setQueueFamilyIndex(queue_family_index)
			};
			auto exts = parent::get_extensions();
			m_device = physical_device.createDevice(
				vk::DeviceCreateInfo{}
				.setQueueCreateInfos(queue_create_infos)
				.setPEnabledExtensionNames(exts)
			);
		}
		~add_device() {
			m_device.destroy();
		}
		auto get_device() {
			return m_device;
		}
	private:
		vk::Device m_device;
	};
	template<class T>
	class add_queue : public T {
	public:
		using parent = T;
		add_queue() {
			vk::Device device = parent::get_device();
			uint32_t queue_family_index = parent::get_queue_family_index();
			m_queue = device.getQueue(queue_family_index, 0);
		}
		auto get_queue() {
			return m_queue;
		}
	private:
		vk::Queue m_queue;
	};
	template<class T>
	class add_swapchain_image_format : public T {
	public:
		using parent = T;
		add_swapchain_image_format() {
			vk::PhysicalDevice physical_device = parent::get_physical_device();
			vk::SurfaceKHR surface = parent::get_surface();
			std::vector<vk::SurfaceFormatKHR> formats = physical_device.getSurfaceFormatsKHR(surface);
			m_format = vk::Format::eR8G8B8A8Unorm;
		}
		auto get_swapchain_image_format() {
			return m_format;
		}
	private:
		vk::Format m_format;
	};
	template<class T>
	class add_swapchain_image_extent_equal_surface_current_extent : public T {
	public:
		using parent = T;
		auto get_swapchain_image_extent() {
			vk::SurfaceCapabilitiesKHR cap = parent::get_surface_capabilities();
			return cap.currentExtent;
		}
	};
	template<class T>
	class add_recreate_surface_for_swapchain : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::destroy_swapchain();
			parent::recreate_surface();
			parent::create_swapchain();
		}
	};
	template<class T>
	class add_swapchain : public T {
	public:
		using parent = T;
		add_swapchain() {
			create_swapchain();
		}
		~add_swapchain() {
			destroy_swapchain();
		}
		void create_swapchain() {
			vk::Device device = parent::get_device();
			vk::SurfaceKHR surface = parent::get_surface();
			vk::Format format = parent::get_swapchain_image_format();
			vk::Extent2D swapchain_image_extent = parent::get_swapchain_image_extent();

			vk::SurfaceCapabilitiesKHR cap = parent::get_surface_capabilities();
			m_swapchain = device.createSwapchainKHR(
				vk::SwapchainCreateInfoKHR{}
				.setMinImageCount(cap.minImageCount)
				.setImageExtent(swapchain_image_extent)
				.setImageFormat(format)
				.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
				.setImageUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eColorAttachment)
				.setImageArrayLayers(1)
				.setSurface(surface)
			);
		}
		void destroy_swapchain() {
			vk::Device device = parent::get_device();
			device.destroySwapchainKHR(m_swapchain);
		}
		auto get_swapchain() {
			return m_swapchain;
		}
	private:
		vk::SwapchainKHR m_swapchain;
	};
	template<class T>
	class add_recreate_surface_for_swapchain_images_views : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::destroy_swapchain_images_views();
			parent::recreate_surface();
			parent::create_swapchain_images_views();
		}
	};
	template<class T>
	class add_images_views : public T {
	public:
		using parent = T;
		add_images_views() {
			create();
		}
		~add_images_views() {
			destroy();
		}
		void create() {
			vk::Device device = parent::get_device();
			auto images = parent::get_images();
			vk::Format format = parent::get_image_format();

			m_views.resize(images.size());
			std::ranges::transform(
				images,
				m_views.begin(),
				[device, format](auto image) {
					return device.createImageView(
						vk::ImageViewCreateInfo{}
						.setImage(image)
						.setFormat(format)
						.setSubresourceRange(
							vk::ImageSubresourceRange{}
							.setAspectMask(vk::ImageAspectFlagBits::eColor)
							.setLayerCount(1)
							.setLevelCount(1)
						)
						.setViewType(vk::ImageViewType::e2D)
					);
				}
			);
		}
		void destroy() {
			vk::Device device = parent::get_device();
			std::ranges::for_each(
				m_views,
				[device](auto view) {
					device.destroyImageView(view);
				}
			);
		}
		auto get_images_views() {
			return m_views;
		}
	private:
		std::vector<vk::ImageView> m_views;
	};
	template<class T>
	class rename_images_views_to_depth_images_views : public T {
	public:
		using parent = T;
		auto get_depth_images_views() {
			return parent::get_images_views();
		}
	};
	template<class T>
	class add_swapchain_images_views : public T {
	public:
		using parent = T;
		add_swapchain_images_views() {
			create_swapchain_images_views();
		}
		~add_swapchain_images_views() {
			destroy_swapchain_images_views();
		}
		void create_swapchain_images_views() {
			vk::Device device = parent::get_device();
			auto images = parent::get_swapchain_images();
			vk::Format format = parent::get_swapchain_image_format();

			m_views.resize(images.size());
			std::ranges::transform(
				images,
				m_views.begin(),
				[device, format](auto image) {
					return device.createImageView(
						vk::ImageViewCreateInfo{}
						.setImage(image)
						.setFormat(format)
						.setSubresourceRange(
							vk::ImageSubresourceRange{}
							.setAspectMask(vk::ImageAspectFlagBits::eColor)
							.setLayerCount(1)
							.setLevelCount(1)
						)
						.setViewType(vk::ImageViewType::e2D)
					);
				}
			);
		}
		void destroy_swapchain_images_views() {
			vk::Device device = parent::get_device();
			std::ranges::for_each(
				m_views,
				[device](auto view) {
					device.destroyImageView(view);
				}
			);
		}
		auto get_swapchain_image_views() {
			return m_views;
		}
	private:
		std::vector<vk::ImageView> m_views;
	};
	template<class T>
	class add_recreate_surface_for_swapchain_images : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::recreate_surface();
			parent::flush_swapchain_images();
		}
	};
	template<class T>
	class add_swapchain_images : public T {
	public:
		using parent = T;
		add_swapchain_images() {
			flush_swapchain_images();
		}
		void flush_swapchain_images() {
			vk::Device device = parent::get_device();
			vk::SwapchainKHR swapchain = parent::get_swapchain();
			m_images = device.getSwapchainImagesKHR(swapchain);
		}
		auto get_swapchain_image(uint32_t index) {
			return m_images[index];
		}
		auto get_swapchain_images() {
			return m_images;
		}
	private:
		std::vector<vk::Image> m_images;
	};
	template<class T>
	class add_image_extent_equal_swapchain_image_extent : public T {
	public:
		using parent = T;
		auto get_image_extent() {
			return vk::Extent3D{ parent::get_swapchain_image_extent(), 1 };
		}
	};
	template<class T>
	class add_image_count_equal_swapchain_image_count : public T {
	public:
		using parent = T;
		auto get_image_count() {
			return parent::get_swapchain_images().size();
		}
	};
	template<class T>
	class rename_image_format_to_depth_image_format : public T {
	public:
		using parent = T;
		auto get_depth_image_format() {
			return parent::get_image_format();
		}
	};
	template<vk::Format f, class T>
	class add_image_format : public T {
	public:
		auto get_image_format() {
			return f;
		}
	};
	template<vk::ImageType ImageType, class T>
	class add_image_type : public T {
	public:
		auto get_image_type() {
			return ImageType;
		}
	};
	template<uint32_t ImageCount, class T>
	class add_image_count : public T {
	public:
		auto get_image_count() {
			return ImageCount;
		}
	};
	template<class T>
	class add_empty_image_usages : public T {
	public:
		auto get_image_usages() {
			return vk::ImageUsageFlags{};
		}
	};
	template<vk::ImageUsageFlagBits Usage, class T>
	class add_image_usage : public T {
	public:
		using parent = T;
		auto get_image_usages() {
			vk::ImageUsageFlags usages = parent::get_image_usages();
			return usages | Usage;
		}
	};
	template<vk::SampleCountFlagBits Samples, class T>
	class set_image_samples : public T {
	public:
		using parent = T;
		auto get_image_samples() {
			return Samples;
		}
	};
	template<vk::ImageTiling Tiling, class T>
	class set_image_tiling : public T {
	public:
		using parent = T;
		auto get_image_tiling() {
			return Tiling;
		}
	};
	template<class T>
	class add_recreate_surface_for_images : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::destroy_images();
			parent::recreate_surface();
			parent::create_images();
		}
	};
	template<class T>
	class add_images : public T {
	public:
		using parent = T;
		add_images() {
			create_images();
		}
		~add_images() {
			destroy_images();
		}
		void create_images() {
			vk::Device device = parent::get_device();
			vk::Extent3D extent = parent::get_image_extent();
			vk::Format format = parent::get_image_format();
			vk::ImageType image_type = parent::get_image_type();
			uint32_t queue_family_index = parent::get_queue_family_index();
			vk::ImageUsageFlags image_usage = parent::get_image_usages();
			vk::SampleCountFlagBits samples = parent::get_image_samples();
			vk::ImageTiling tiling = parent::get_image_tiling();

			uint32_t image_count = parent::get_image_count();

			m_images.resize(image_count);

			std::ranges::for_each(
				m_images,
				[device, extent, format, image_type, queue_family_index, image_usage, samples, tiling](vk::Image& image) {
					image = device.createImage(
						vk::ImageCreateInfo{}
						.setArrayLayers(1)
						.setExtent(extent)
						.setFormat(format)
						.setImageType(image_type)
						.setInitialLayout(vk::ImageLayout::eUndefined)
						.setMipLevels(1)
						.setQueueFamilyIndices(queue_family_index)
						.setUsage(image_usage)
						.setSamples(samples)
						.setTiling(tiling)
						.setSharingMode(vk::SharingMode::eExclusive)
					);
				}
			);
		}
		void destroy_images() {
			vk::Device device = parent::get_device();
			std::ranges::for_each(
				m_images,
				[device](vk::Image image) {
					device.destroyImage(image);
				}
			);
		}
		auto get_images() {
			return m_images;
		}
	private:
		std::vector<vk::Image> m_images;
	};

	template<class T>
	class add_image_memory : public T {
	public:
		using parent = T;
		add_image_memory() {
			vk::Device device = parent::get_device();
			vk::Image image = parent::get_image();
			vk::MemoryPropertyFlags memory_properties = parent::get_image_memory_properties();

			auto memory_requirements =
				device.getImageMemoryRequirements(
					image
				);
			uint32_t memory_type_index = parent::find_properties(memory_requirements.memoryTypeBits, memory_properties);
			auto memory = device.allocateMemory(
				vk::MemoryAllocateInfo{}
				.setAllocationSize(memory_requirements.size)
				.setMemoryTypeIndex(memory_type_index)
			);
			device.bindImageMemory(image, memory, 0);
			m_memory = memory;
		}
		~add_image_memory() {
			vk::Device device = parent::get_device();
			device.freeMemory(m_memory);
		}
	private:
		vk::DeviceMemory m_memory;
	};
	template<class T>
	class add_buffer_memory_create : public T {
	public:
		using parent = T;
		auto create() {
			vk::Device device = parent::get_device();
			vk::Buffer buffer = parent::get_buffer();
			vk::MemoryPropertyFlags memory_properties = parent::get_buffer_memory_properties();

			auto memory_requirements =
				device.getBufferMemoryRequirements(
					buffer
				);
			uint32_t memory_type_index = parent::find_properties(memory_requirements.memoryTypeBits, memory_properties);
			auto memory = device.allocateMemory(
				vk::MemoryAllocateInfo{}
				.setAllocationSize(memory_requirements.size)
				.setMemoryTypeIndex(memory_type_index)
			);
			device.bindBufferMemory(buffer, memory, 0);
			return memory;
		}
	};
	template<class T>
	class map_buffer_memory_vector : public T {
	public:
		using parent = T;
		map_buffer_memory_vector() {
			vk::Device device = parent::get_device();
			std::vector<vk::DeviceMemory> memory_vector = parent::get_buffer_memory_vector();
			m_ptrs.resize(memory_vector.size());
			std::ranges::transform(
				memory_vector,
				m_ptrs.begin(),
				[device](auto& memory) {
					return device.mapMemory(memory, 0, vk::WholeSize);
				}
			);
		}
		~map_buffer_memory_vector() {
			vk::Device device = parent::get_device();
			std::vector<vk::DeviceMemory> memory_vector = parent::get_buffer_memory_vector();
			std::ranges::for_each(
				memory_vector,
				[device](auto& memory) {
					device.unmapMemory(memory);
				}
			);
		}
		auto get_buffer_memory_ptr_vector() {
			return m_ptrs;
		}
	private:
		std::vector<void*> m_ptrs;
	};
	template<class T>
	class add_buffer_memory_vector : public T {
	public:
		using parent = T;
		add_buffer_memory_vector() {
			vk::Device device = parent::get_device();
			std::vector<vk::Buffer> buffers = parent::get_vector();
			vk::MemoryPropertyFlags memory_properties = parent::get_buffer_memory_properties();

			auto memory_requirements =
				device.getBufferMemoryRequirements(
					buffers[0]
				);
			uint32_t memory_type_index = parent::find_properties(memory_requirements.memoryTypeBits, memory_properties);
			m_memory.resize(buffers.size());
			std::ranges::transform(
				buffers,
				m_memory.begin(),
				[device, memory_type_index, memory_requirements, buffers](auto& buffer) {
					auto memory = device.allocateMemory(
						vk::MemoryAllocateInfo{}
						.setAllocationSize(memory_requirements.size)
						.setMemoryTypeIndex(memory_type_index)
					);
					device.bindBufferMemory(buffer, memory, 0);
					return memory;
				}
			);
		}
		~add_buffer_memory_vector() {
			vk::Device device = parent::get_device();
			std::ranges::for_each(
				m_memory,
				[device](auto& memory) {
					device.freeMemory(memory);
				}
			);
		}
		auto get_buffer_memory_vector() {
			return m_memory;
		}
	private:
		std::vector<vk::DeviceMemory> m_memory;
	};
	template<class T>
	class add_buffer_memory : public T {
	public:
		using parent = T;
		add_buffer_memory() {
			vk::Device device = parent::get_device();
			vk::Buffer buffer = parent::get_buffer();
			vk::MemoryPropertyFlags memory_properties = parent::get_buffer_memory_properties();

			auto memory_requirements =
				device.getBufferMemoryRequirements(
					buffer
				);
			uint32_t memory_type_index = parent::find_properties(memory_requirements.memoryTypeBits, memory_properties);
			auto memory = device.allocateMemory(
				vk::MemoryAllocateInfo{}
				.setAllocationSize(memory_requirements.size)
				.setMemoryTypeIndex(memory_type_index)
			);
			device.bindBufferMemory(buffer, memory, 0);
			m_memory = memory;
		}
		~add_buffer_memory() {
			vk::Device device = parent::get_device();
			device.freeMemory(m_memory);
		}
		auto get_buffer_memory() {
			return m_memory;
		}
	private:
		vk::DeviceMemory m_memory;
	};
	template<class T>
	class add_recreate_surface_for_images_memories : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::destroy_images_memories();
			parent::recreate_surface();
			parent::create_images_memories();
		}
	};
	template<class T>
	class add_images_memories : public T {
	public:
		using parent = T;
		add_images_memories() {
			create_images_memories();
		}
		~add_images_memories() {
			destroy_images_memories();
		}
		void create_images_memories() {
			vk::Device device = parent::get_device();
			auto images = parent::get_images();
			vk::MemoryPropertyFlags memory_properties = parent::get_image_memory_properties();

			m_memories.resize(images.size());
			std::ranges::transform(images, m_memories.begin(),
				[device, memory_properties, this](vk::Image image) {
					auto memory_requirements = device.getImageMemoryRequirements(
						image
					);
					uint32_t memory_type_index = parent::find_properties(memory_requirements.memoryTypeBits, memory_properties);
					auto memory = device.allocateMemory(
						vk::MemoryAllocateInfo{}
						.setAllocationSize(memory_requirements.size)
						.setMemoryTypeIndex(memory_type_index)
					);
					device.bindImageMemory(image, memory, 0);
					return memory;
				});
		}
		void destroy_images_memories() {
			vk::Device device = parent::get_device();
			std::ranges::for_each(m_memories,
				[device](vk::DeviceMemory memory) {
					device.freeMemory(memory);
				});
		}
		auto get_images_memories() {
			return m_memories;
		}
	private:
		std::vector<vk::DeviceMemory> m_memories;
	};
	template<vk::MemoryPropertyFlagBits Property, class T>
	class add_image_memory_property : public T {
	public:
		using parent = T;
		auto get_image_memory_properties() {
			auto properties = parent::get_image_memory_properties();
			return properties | Property;
		}
	};
	template<class T>
	class add_empty_image_memory_properties : public T {
	public:
		auto get_image_memory_properties() {
			return vk::MemoryPropertyFlagBits{};
		}
	};
	template<class T>
	class cache_physical_device_memory_properties : public T {
	public:
		using parent = T;
		cache_physical_device_memory_properties() {
			vk::PhysicalDevice physical_device = parent::get_physical_device();
			m_properties = physical_device.getMemoryProperties();
		}
		auto get_physical_device_memory_properties() {
			return m_properties;
		}
	private:
		vk::PhysicalDeviceMemoryProperties m_properties;
	};
	template<class T>
	class add_find_properties : public T {
	public:
		using parent = T;
		uint32_t find_properties(uint32_t memory_type_bits_requirements, vk::MemoryPropertyFlags required_property) {
			vk::PhysicalDeviceMemoryProperties memory_properties = parent::get_physical_device_memory_properties();
			const uint32_t memory_count = memory_properties.memoryTypeCount;
			for (uint32_t memoryIndex = 0; memoryIndex < memory_count; memoryIndex++) {
				const uint32_t memoryTypeBits = (1 << memoryIndex);
				const bool is_required_memory_type = memory_type_bits_requirements & memoryTypeBits;
				const vk::MemoryPropertyFlags properties =
					memory_properties.memoryTypes[memoryIndex].propertyFlags;
				const bool has_required_properties =
					(properties & required_property) == required_property;
				if (is_required_memory_type && has_required_properties)
					return memoryIndex;
			}
			throw std::runtime_error{ "failed find memory property" };
		}
	};
	template<class T>
	class add_command_pool : public T {
	public:
		using parent = T;
		add_command_pool() {
			vk::Device device = parent::get_device();
			uint32_t queue_family_index = parent::get_queue_family_index();
			m_pool = device.createCommandPool(
				vk::CommandPoolCreateInfo{}
				.setQueueFamilyIndex(queue_family_index)
			);
		}
		~add_command_pool() {
			vk::Device device = parent::get_device();
			device.destroyCommandPool(m_pool);
		}
		auto get_command_pool() {
			return m_pool;
		}
	private:
		vk::CommandPool m_pool;
	};
	template<class T>
	class add_command_buffer : public T {
	public:
		using parent = T;
		add_command_buffer() {
			vk::Device device = parent::get_device();
			vk::CommandPool pool = parent::get_command_pool();

			m_buffer = device.allocateCommandBuffers(
				vk::CommandBufferAllocateInfo{}
				.setCommandPool(pool)
				.setCommandBufferCount(1)
			)[0];
		}
		~add_command_buffer() {
			vk::Device device = parent::get_device();
			vk::CommandPool pool = parent::get_command_pool();
			device.freeCommandBuffers(pool, m_buffer);
		}
		auto get_command_buffer() {
			return m_buffer;
		}
	private:
		vk::CommandBuffer m_buffer;
	};
	template<class T>
	class add_recreate_surface_for : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::destroy();
			parent::recreate_surface();
			parent::create();
		}
	};
	template<class T>
	class add_swapchain_command_buffers : public T {
	public:
		using parent = T;
		add_swapchain_command_buffers() {
			create();
		}
		~add_swapchain_command_buffers() {
			destroy();
		}
		void create() {
			vk::Device device = parent::get_device();
			vk::CommandPool pool = parent::get_command_pool();
			uint32_t swapchain_image_count = parent::get_swapchain_images().size();
			m_buffers = device.allocateCommandBuffers(
				vk::CommandBufferAllocateInfo{}
				.setCommandPool(pool)
				.setCommandBufferCount(swapchain_image_count)
			);
		}
		void destroy() {
			vk::Device device = parent::get_device();
			vk::CommandPool pool = parent::get_command_pool();
			device.freeCommandBuffers(pool, m_buffers);
		}
		auto get_swapchain_command_buffers() {
			return m_buffers;
		}
		auto get_swapchain_command_buffer(uint32_t i) {
			return m_buffers[i];
		}
	private:
		std::vector<vk::CommandBuffer> m_buffers;
	};
	template<class T>
	class add_get_format_clear_color_value_type : public T {
	public:
		enum class clear_color_value_type {
			eInt32,
			eUint32,
			eFloat32,
		};
		clear_color_value_type get_format_clear_color_value_type(vk::Format f) {
			std::map<vk::Format, clear_color_value_type> types{
				{vk::Format::eR8G8B8A8Unorm, clear_color_value_type::eFloat32},
				{vk::Format::eR32G32B32A32Sfloat, clear_color_value_type::eFloat32},
				{vk::Format::eR8G8B8A8Srgb, clear_color_value_type::eUint32},
				{vk::Format::eR32G32B32A32Uint, clear_color_value_type::eUint32},
			};
			if (!types.contains(f)) {
				throw std::runtime_error{ "this format does not support clear color value" };
			}
			return types[f];
		}
	};
	template<class T>
	class add_recreate_surface_for_framebuffers : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::destroy_framebuffers();
			parent::recreate_surface();
			parent::create_framebuffers();
		}
	};
	template<class T>
	class add_framebuffers : public T {
	public:
		using parent = T;
		add_framebuffers() {
			create_framebuffers();
		}
		~add_framebuffers() {
			destroy_framebuffers();
		}
		void create_framebuffers() {
			vk::Device device = parent::get_device();
			vk::RenderPass render_pass = parent::get_render_pass();
			auto extent = parent::get_swapchain_image_extent();
			uint32_t width = extent.width;
			uint32_t height = extent.height;
			auto swapchain_image_views = parent::get_swapchain_image_views();
			m_framebuffers.resize(swapchain_image_views.size());
			std::ranges::transform(swapchain_image_views, m_framebuffers.begin(),
				[device, render_pass, extent, width, height](auto& image_view) {
					return device.createFramebuffer(
						vk::FramebufferCreateInfo{}
						.setAttachments(image_view)
						.setRenderPass(render_pass)
						.setWidth(width)
						.setHeight(height)
						.setLayers(1)
					);
				});
		}
		void destroy_framebuffers() {
			vk::Device device = parent::get_device();
			std::ranges::for_each(m_framebuffers,
				[device](auto framebuffer) {
					device.destroyFramebuffer(framebuffer);
				});
		}
		auto get_framebuffers() {
			return m_framebuffers;
		}
	private:
		std::vector<vk::Framebuffer> m_framebuffers;
	};

	template<class T>
	class rename_vertex_buffer_to_buffer : public T {
	public:
		using parent = T;
		auto get_buffer() {
			return parent::get_vertex_buffer();
		}
	};
	template<vk::MemoryPropertyFlagBits Property, class T>
	class set_buffer_memory_properties : public T {
	public:
		using parent = T;
		auto get_buffer_memory_properties() {
			return Property;
		}
	};
	template<class T>
	class rename_vertex_buffer_data_to_buffer_data : public T {
	public:
		using parent = T;
		auto get_buffer_data() {
			return parent::get_vertex_buffer_data();
		}
	};
	template<class T>
	class copy_buffer_data : public T {
	public:
		using parent = T;
		copy_buffer_data() {
			vk::Device device = parent::get_device();
			vk::DeviceMemory memory = parent::get_buffer_memory();
			auto data = parent::get_buffer_data();
			void* ptr = device.mapMemory(memory, 0, vk::WholeSize);
			memcpy(ptr, data.data(), data.size() * sizeof(data[0]));
			device.unmapMemory(memory);
		}
	};
	template<class T>
	class add_buffer_memory_with_data_copy :
		public
		copy_buffer_data<
		add_buffer_memory<
		set_buffer_memory_properties<vk::MemoryPropertyFlagBits::eHostVisible,
		T>>>
	{
	};
	template<vk::BufferUsageFlagBits Usage, class T>
	class add_buffer_usage : public T {
	public:
		using parent = T;
		auto get_buffer_usage() {
			return parent::get_buffer_usage() | Usage;
		}
	};
	template<class T>
	class empty_buffer_usage : public T {
	public:
		auto get_buffer_usage() {
			return vk::BufferUsageFlags{};
		}
	};
	template<vk::BufferUsageFlagBits Usage, class T>
	class set_buffer_usage : public T {
	public:
		auto get_buffer_usage() {
			return Usage;
		}
	};
	template<class T>
	class rename_buffer_vector_to_vertex_buffer_vector : public T {
	public:
		using parent = T;
		auto get_vertex_buffer_vector() {
			return parent::get_buffer_vector();
		}
	};
	template<class T>
	class rename_buffer_vector_to_uniform_buffer_vector : public T {
	public:
		using parent = T;
		auto get_uniform_buffer_vector() {
			return parent::get_buffer_vector();
		}
	};
	template<class T>
	class rename_buffer_vector_to_uniform_upload_buffer_vector : public T {
	public:
		using parent = T;
		auto get_uniform_upload_buffer_vector() {
			return parent::get_buffer_vector();
		}
	};
	template<class T>
	class rename_buffer_memory_vector_to_vertex_buffer_memory_vector : public T {
	public:
		using parent = T;
		auto get_vertex_buffer_memory_vector() {
			return parent::get_buffer_memory_vector();
		}
	};
	template<class T>
	class rename_buffer_memory_vector_to_uniform_upload_buffer_memory_vector : public T {
	public:
		using parent = T;
		auto get_uniform_upload_buffer_memory_vector() {
			return parent::get_buffer_memory_vector();
		}
	};
	template<class T>
	class rename_buffer_memory_ptr_vector_to_vertex_buffer_memory_ptr_vector : public T {
	public:
		using parent = T;
		auto get_vertex_buffer_memory_ptr_vector() {
			return parent::get_buffer_memory_ptr_vector();
		}
	};
	template<class T>
	class rename_buffer_memory_ptr_vector_to_uniform_upload_buffer_memory_ptr_vector : public T {
	public:
		using parent = T;
		auto get_uniform_upload_buffer_memory_ptr_vector() {
			return parent::get_buffer_memory_ptr_vector();
		}
	};
	template<class T>
	class rename_buffer_to_index_buffer : public T {
	public:
		using parent = T;
		auto get_index_buffer() {
			return parent::get_buffer();
		}
	};
	template<class T>
	class rename_buffer_to_vertex_buffer : public T {
	public:
		using parent = T;
		auto get_vertex_buffer() {
			return parent::get_buffer();
		}
	};
	template<vk::DeviceSize Size, class T>
	class set_buffer_size : public T {
	public:
		auto get_buffer_size() {
			return Size;
		}
	};

	template<class T>
	class add_buffer_create : public T {
	public:
		using parent = T;
		using create_type = vk::Buffer;
		auto create() {
			vk::Device device = parent::get_device();
			uint32_t queue_family_index = parent::get_queue_family_index();
			vk::DeviceSize size = parent::get_buffer_size();
			auto usage = parent::get_buffer_usage();
			return device.createBuffer(
				vk::BufferCreateInfo{}
				.setQueueFamilyIndices(queue_family_index)
				.setSize(size)
				.setUsage(usage)
			);
		}
	};
	template<class T>
	class add_buffer_destroy : public T {
	public:
		using parent = T;
		auto destroy(vk::Buffer buffer) {
			vk::Device device = parent::get_device();
			device.destroyBuffer(buffer);
		}
	};
	template<class T>
	class add_buffer_create_destroy
		: public
		add_buffer_destroy<add_buffer_create<T>>
	{};
	template<class T>
	class add_buffer_as_member : public add_buffer_create_destroy<T> {
	public:
		using parent = add_buffer_create_destroy<T>;
		add_buffer_as_member() {
			m_buffer = parent::create();
		}
		~add_buffer_as_member() {
			parent::destroy(m_buffer);
		}
		auto get_buffer() {
			return m_buffer;
		}
	private:
		vk::Buffer m_buffer;
	};
	template<class T>
	class add_vector_for : public T {
	public:
		using parent = T;
		add_vector_for() {
			uint32_t count = parent::get_vector_size();
			m_members.resize(count);
			std::ranges::for_each(
				m_members,
				[this](auto& member) {
					member = parent::create();
				}
			);
		}
		~add_vector_for() {
			std::ranges::for_each(
				m_members,
				[this](auto& member) {
					parent::destroy(member);
				}
			);
		}
		auto get_vector() {
			return m_members;
		}
	private:
		std::vector<typename parent::create_type> m_members;
	};
	template<class T>
	class add_buffer_vector
		:
		public add_vector_for<add_buffer_create_destroy<T>>
	{
	public:
		using parent = add_vector_for<add_buffer_create_destroy<T>>;
		auto get_buffer_vector() {
			return parent::get_vector();
		}
	};
	template<class T>
	class jump_draw_if_window_minimized : public T {
	public:
		using parent = T;
		void draw() {
			if (!parent::is_window_minimized()) {
				parent::draw();
			}
		}
	};
	
	template<class T>
	class add_draw : public T {
	public:
		using parent = T;
		void draw() {
			vk::Device device = parent::get_device();
			vk::SwapchainKHR swapchain = parent::get_swapchain();
			vk::Queue queue = parent::get_queue();
			vk::Semaphore acquire_image_semaphore = parent::get_acquire_next_image_semaphore();
			bool need_recreate_surface = false;


			auto [res, index] = device.acquireNextImage2KHR(
				vk::AcquireNextImageInfoKHR{}
				.setSwapchain(swapchain)
				.setSemaphore(acquire_image_semaphore)
				.setTimeout(UINT64_MAX)
				.setDeviceMask(1)
			);
			if (res == vk::Result::eSuboptimalKHR) {
				need_recreate_surface = true;
			}
			else if (res != vk::Result::eSuccess) {
				throw std::runtime_error{ "acquire next image != success" };
			}
			parent::free_acquire_next_image_semaphore(index);

			vk::Fence acquire_next_image_semaphore_fence = parent::get_acquire_next_image_semaphore_fence(index);
			{
				vk::Result res = device.waitForFences(acquire_next_image_semaphore_fence, true, UINT64_MAX);
				if (res != vk::Result::eSuccess) {
					throw std::runtime_error{ "failed to wait fences" };
				}
			}
			device.resetFences(acquire_next_image_semaphore_fence);

			vk::Semaphore draw_image_semaphore = parent::get_draw_image_semaphore(index);
			vk::CommandBuffer buffer = parent::get_swapchain_command_buffer(index);
			vk::PipelineStageFlags wait_stage_mask{ vk::PipelineStageFlagBits::eTopOfPipe };
			queue.submit(
				vk::SubmitInfo{}
				.setCommandBuffers(buffer)
				.setWaitSemaphores(
					acquire_image_semaphore
				)
				.setWaitDstStageMask(wait_stage_mask)
				.setSignalSemaphores(
					draw_image_semaphore
				),
				acquire_next_image_semaphore_fence
			);
			try {
				auto res = queue.presentKHR(
					vk::PresentInfoKHR{}
					.setImageIndices(index)
					.setSwapchains(swapchain)
					.setWaitSemaphores(draw_image_semaphore)
				);
				if (res == vk::Result::eSuboptimalKHR) {
					need_recreate_surface = true;
				}
				else if (res != vk::Result::eSuccess) {
					throw std::runtime_error{ "present return != success" };
				}
			}
			catch (vk::OutOfDateKHRError e) {
				need_recreate_surface = true;
			}
			if (need_recreate_surface) {
				queue.waitIdle();
				parent::recreate_surface();
			}
		}
		~add_draw() {
			vk::Device device = parent::get_device();
			vk::Queue queue = parent::get_queue();
			queue.waitIdle();
		}
	};
	template<class T>
	class add_acquire_next_image_fences : public T {
	public:
		using parent = T;
		add_acquire_next_image_fences() {
			vk::Device device = parent::get_device();
			uint32_t swapchain_image_count = parent::get_swapchain_images().size();

			m_fences.resize(swapchain_image_count + 1);
			std::ranges::for_each(m_fences,
				[device](vk::Fence& fence) {
					fence = device.createFence(
						vk::FenceCreateInfo{}
					); }
			);
		}
		~add_acquire_next_image_fences() {
			vk::Device device = parent::get_device();
			std::ranges::for_each(m_fences,
				[device](vk::Fence fence) {
					device.destroyFence(fence);
				});
		}
		auto get_acquire_next_image_fences() {
			return m_fences;
		}
	private:
		std::vector<vk::Fence> m_fences;
	};
	template<class T>
	class add_draw_semaphores : public T {
	public:
		using parent = T;
		add_draw_semaphores() {
			vk::Device device = parent::get_device();
			uint32_t swapchain_image_count = parent::get_swapchain_images().size();

			m_semaphores.resize(swapchain_image_count);
			std::ranges::for_each(m_semaphores,
				[device](vk::Semaphore& semaphore) {
					semaphore = device.createSemaphore(
						vk::SemaphoreCreateInfo{}
					);
				});
		}
		~add_draw_semaphores() {
			vk::Device device = parent::get_device();

			std::ranges::for_each(m_semaphores,
				[device](vk::Semaphore semaphore) {
					device.destroySemaphore(semaphore);
				});
		}
		auto get_draw_image_semaphore(uint32_t i) {
			return m_semaphores[i];
		}
	private:
		std::vector<vk::Semaphore> m_semaphores;
	};
	template<class T>
	class add_acquire_next_image_semaphores : public T {
	public:
		using parent = T;
		add_acquire_next_image_semaphores() {
			vk::Device device = parent::get_device();
			uint32_t swapchain_image_count = parent::get_swapchain_images().size();

			m_semaphores.resize(swapchain_image_count + 1);
			std::ranges::for_each(m_semaphores,
				[device](vk::Semaphore& semaphore) {
					semaphore = device.createSemaphore(
						vk::SemaphoreCreateInfo{}
					);
				});

			m_semaphore_indices.resize(swapchain_image_count);
			std::ranges::iota(m_semaphore_indices, 0);
			m_free_semaphore_index = swapchain_image_count;
		}
		~add_acquire_next_image_semaphores() {
			vk::Device device = parent::get_device();

			std::ranges::for_each(m_semaphores,
				[device](vk::Semaphore semaphore) {
					device.destroySemaphore(semaphore);
				});
		}
		auto get_acquire_next_image_semaphore() {
			return m_semaphores[m_free_semaphore_index];
		}
		auto free_acquire_next_image_semaphore(uint32_t image_index) {
			vk::Device device = parent::get_device();


			std::swap(m_free_semaphore_index, m_semaphore_indices[image_index]);
		}
	private:
		std::vector<vk::Semaphore> m_semaphores;
		uint32_t m_free_semaphore_index;
		std::vector<uint32_t> m_semaphore_indices;
	};
	template<class T>
	class add_acquire_next_image_semaphore_fences : public T {
	public:
		using parent = T;
		add_acquire_next_image_semaphore_fences() {
			vk::Device device = parent::get_device();
			uint32_t swapchain_image_count = parent::get_swapchain_images().size();

			m_fences.resize(swapchain_image_count);
			std::ranges::for_each(m_fences,
				[device](vk::Fence& fence) {
					fence = device.createFence(
						vk::FenceCreateInfo{}
						.setFlags(vk::FenceCreateFlagBits::eSignaled)
					); }
			);
		}
		~add_acquire_next_image_semaphore_fences() {
			vk::Device device = parent::get_device();
			std::ranges::for_each(m_fences,
				[device](vk::Fence fence) {
					device.destroyFence(fence);
				});
		}
		auto get_acquire_next_image_semaphore_fence(uint32_t image_index) {
			return m_fences[image_index];
		}
	private:
		std::vector<vk::Fence> m_fences;
	};
	template<class T>
	class add_recreate_surface_for_pipeline : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::destroy_pipeline();
			parent::recreate_surface();
			parent::create_pipeline();
		}
	};
	template<class T>
	class add_graphics_pipeline : public T {
	public:
		using parent = T;
		add_graphics_pipeline() {
			create_pipeline();
		}
		~add_graphics_pipeline() {
			destroy_pipeline();
		}
		void create_pipeline() {
			vk::Device device = parent::get_device();
			vk::PipelineLayout pipeline_layout = parent::get_pipeline_layout();
			vk::PipelineColorBlendStateCreateInfo color_blend_state = parent::get_pipeline_color_blend_state_create_info();
			vk::PipelineDepthStencilStateCreateInfo depth_stencil_state = parent::get_pipeline_depth_stencil_state_create_info();
			vk::PipelineDynamicStateCreateInfo dynamic_state = parent::get_pipeline_dynamic_state_create_info();
			vk::PipelineInputAssemblyStateCreateInfo input_assembly_state = parent::get_pipeline_input_assembly_state_create_info();
			vk::PipelineMultisampleStateCreateInfo multisample_state =
				parent::get_pipeline_multisample_state_create_info();
			vk::PipelineRasterizationStateCreateInfo rasterization_state =
				parent::get_pipeline_rasterization_state_create_info();
			auto stages =
				parent::get_pipeline_stages();
			vk::PipelineTessellationStateCreateInfo tessellation_state =
				parent::get_pipeline_tessellation_state_create_info();
			vk::PipelineVertexInputStateCreateInfo vertex_input_state =
				parent::get_pipeline_vertex_input_state_create_info();
			vk::PipelineViewportStateCreateInfo viewport_state =
				parent::get_pipeline_viewport_state_create_info();
			vk::RenderPass render_pass =
				parent::get_render_pass();
			uint32_t subpass = parent::get_subpass();

			auto [res, pipeline] = device.createGraphicsPipeline(
				{},
				vk::GraphicsPipelineCreateInfo{}
				.setLayout(pipeline_layout)
				.setPColorBlendState(&color_blend_state)
				.setPDepthStencilState(&depth_stencil_state)
				.setPDynamicState(&dynamic_state)
				.setPInputAssemblyState(&input_assembly_state)
				.setPMultisampleState(&multisample_state)
				.setPRasterizationState(&rasterization_state)
				.setStages(stages)
				.setPTessellationState(&tessellation_state)
				.setPVertexInputState(&vertex_input_state)
				.setPViewportState(&viewport_state)
				.setRenderPass(render_pass)
				.setSubpass(subpass)

			);
			if (res != vk::Result::eSuccess) {
				throw std::runtime_error{ "failed to create graphics pipeline" };
			}
			m_pipeline = pipeline;
		}
		void destroy_pipeline() {
			vk::Device device = parent::get_device();
			device.destroyPipeline(m_pipeline);
		}
		auto get_pipeline() {
			return m_pipeline;
		}
	private:
		vk::Pipeline m_pipeline;
	};
	template<uint32_t Subpass, class T>
	class set_subpass : public T {
	public:
		auto get_subpass() {
			return Subpass;
		}
	};
	template<class T>
	class add_empty_attachments : public T {
	public:
		auto get_attachments() {
			return std::vector<vk::AttachmentDescription>{};
		}
	};
	template<class T>
	class add_depth_attachment : public T {
	public:
		using parent = T;
		auto get_attachments() {
			auto attachments = parent::get_attachments();
			vk::Format format = parent::get_depth_image_format();
			vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined;
			vk::ImageLayout final_layout = vk::ImageLayout::eGeneral;
			attachments.emplace_back(
				vk::AttachmentDescription{}
				.setInitialLayout(initial_layout)
				.setFinalLayout(final_layout)
				.setFormat(format)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			);
			return attachments;
		}
	};
	template<class T>
	class add_attachment : public T {
	public:
		using parent = T;
		auto get_attachments() {
			auto attachments = parent::get_attachments();
			vk::Format format = parent::get_swapchain_image_format();
			vk::ImageLayout initial_layout = vk::ImageLayout::eUndefined;
			vk::ImageLayout final_layout = vk::ImageLayout::ePresentSrcKHR;
			attachments.emplace_back(
				vk::AttachmentDescription{}
				.setInitialLayout(initial_layout)
				.setFinalLayout(final_layout)
				.setFormat(format)
				.setLoadOp(vk::AttachmentLoadOp::eClear)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			);
			return attachments;
		}
	};
	template<class T>
	class add_subpass_dependency : public T {
	public:
		using parent = T;
		auto get_subpass_dependencies() {
			auto depends = parent::get_subpass_dependencies();
			depends.emplace_back(
				vk::SubpassDependency{}
				.setSrcSubpass(vk::SubpassExternal)
				.setDstSubpass(0)
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
				.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead)
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			);
			return depends;
		}
	};
	template<class T>
	class add_empty_subpass_dependencies : public T {
	public:
		auto get_subpass_dependencies() {
			return std::vector<vk::SubpassDependency>{};
		}
	};
	template<class T>
	class add_subpasses : public T {
	public:
		auto get_subpasses() {
			return vk::SubpassDescription{}
				.setColorAttachments(
					vk::AttachmentReference{}
					.setAttachment(0)
					.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
				);
		}
	};
	template<class T>
	class add_render_pass : public T {
	public:
		using parent = T;
		add_render_pass() {
			vk::Device device = parent::get_device();
			auto attachments = parent::get_attachments();
			auto dependencies = parent::get_subpass_dependencies();
			auto subpasses = parent::get_subpasses();

			m_render_pass = device.createRenderPass(
				vk::RenderPassCreateInfo{}
				.setAttachments(attachments)
				.setDependencies(dependencies)
				.setSubpasses(subpasses)
			);
		}
		~add_render_pass() {
			vk::Device device = parent::get_device();
			device.destroyRenderPass(m_render_pass);
		}
		auto get_render_pass() {
			return m_render_pass;
		}
	private:
		vk::RenderPass m_render_pass;
	};
	template<class T>
	class add_pipeline_viewport_state : public T {
	public:
		using parent = T;
		auto get_pipeline_viewport_state_create_info() {
			m_viewports = parent::get_viewports();
			m_scissors = parent::get_scissors();
			if (m_viewports.size() != m_scissors.size()) {
				throw std::runtime_error{ "viewports count != scissors count" };
			}
			return vk::PipelineViewportStateCreateInfo{}
				.setViewports(m_viewports)
				.setScissors(m_scissors);
		}
	private:
		std::vector<vk::Viewport> m_viewports;
		std::vector<vk::Rect2D> m_scissors;
	};
	template<class T>
	class add_scissor_equal_surface_rect : public T {
	public:
		using parent = T;
		auto get_scissors() {
			vk::SurfaceCapabilitiesKHR surface_cap = parent::get_surface_capabilities();
			auto scissors = parent::get_scissors();

			scissors.emplace_back(
				vk::Rect2D{}
				.setOffset({})
				.setExtent(surface_cap.currentExtent)
			);
			return scissors;
		}
	};
	template<class T>
	class add_recreate_surface_for_cache_surface_capabilites : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::recreate_surface();
			parent::flush_surface_capabilities_cache();
		}
	};
	template<class T>
	class cache_surface_capabilities : public T {
	public:
		using parent = T;
		cache_surface_capabilities() {
			flush_surface_capabilities_cache();
		}
		void flush_surface_capabilities_cache() {
			vk::PhysicalDevice physical_device = parent::get_physical_device();
			vk::SurfaceKHR surface = parent::get_surface();
			m_capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
		}
		auto get_surface_capabilities() {
			return m_capabilities;
		}
	private:
		vk::SurfaceCapabilitiesKHR m_capabilities;
	};
	template<class T>
	class test_physical_device_support_surface : public T {
	public:
		using parent = T;
		test_physical_device_support_surface() {
			create();
		}
		void create() {
			vk::PhysicalDevice physical_device = parent::get_physical_device();
			vk::SurfaceKHR surface = parent::get_surface();
			uint32_t queue_family_index = parent::get_queue_family_index();
			m_support = physical_device.getSurfaceSupportKHR(queue_family_index, surface);
		}
		void destroy() {

		}
		auto get_physical_device_support_surface() {
			return m_support;
		}
	private:
		bool m_support;
	};
	template<class T>
	class add_empty_scissors : public T {
	public:
		auto get_scissors() {
			return std::vector<vk::Rect2D>{};
		}
	};
	template<class T>
	class add_viewport : public T {
	public:
		using parent = T;
		auto get_viewports() {
			auto viewports = parent::get_viewports();
			float x = parent::get_viewport_x();
			float y = parent::get_viewport_y();
			float width = parent::get_viewport_width();
			float height = parent::get_viewport_height();
			float min_depth = parent::get_viewport_min_depth();
			float max_depth = parent::get_viewport_max_depth();
			viewports.emplace_back(
				vk::Viewport{}
				.setX(x)
				.setY(y)
				.setWidth(width)
				.setHeight(height)
				.setMinDepth(min_depth)
				.setMaxDepth(max_depth)
			);
			return viewports;
		}
	};
	template<class T>
	class add_viewport_equal_swapchain_image_rect : public T {
	public:
		using parent = T;
		auto get_viewports() {
			auto viewports = parent::get_viewports();
			auto extent = parent::get_swapchain_image_extent();
			viewports.emplace_back(
				vk::Viewport{}
				.setWidth(extent.width)
				.setHeight(extent.height)
				.setMinDepth(0)
				.setMaxDepth(1)
			);
			return viewports;
		}
	};
	template<float X, class T>
	class set_viewport_x : public T {
	public:
		auto get_viewport_x() {
			return X;
		}
	};
	template<float Y, class T>
	class set_viewport_y : public T {
	public:
		auto get_viewport_y() {
			return Y;
		}
	};
	template<float Width, class T>
	class set_viewport_width : public T {
	public:
		auto get_viewport_width() {
			return Width;
		}
	};
	template<float Height, class T>
	class set_viewport_height : public T {
	public:
		auto get_viewport_height() {
			return Height;
		}
	};
	template<float Min_depth, class T>
	class set_viewport_min_depth : public T {
	public:
		auto get_viewport_min_depth() {
			return Min_depth;
		}
	};
	template<float Max_depth, class T>
	class set_viewport_max_depth : public T {
	public:
		auto get_viewport_max_depth() {
			return Max_depth;
		}
	};
	template<class T>
	class add_empty_viewports : public T {
	public:
		auto get_viewports() {
			return std::vector<vk::Viewport>{};
		}
	};
	template<class T>
	class add_pipeline_vertex_input_state : public T {
	public:
		using parent = T;
		auto get_pipeline_vertex_input_state_create_info() {
			m_attribute_descriptions = parent::get_vertex_attribute_descriptions();
			m_binding_descriptions = parent::get_vertex_binding_descriptions();
			return vk::PipelineVertexInputStateCreateInfo{}
				.setVertexAttributeDescriptions(m_attribute_descriptions)
				.setVertexBindingDescriptions(m_binding_descriptions);
		}
	private:
		std::vector<vk::VertexInputAttributeDescription> m_attribute_descriptions;
		std::vector<vk::VertexInputBindingDescription> m_binding_descriptions;
	};
	template<vk::Format Format, class T>
	class set_vertex_input_attribute_format : public T {
	public:
		auto get_vertex_input_attribute_format() {
			return Format;
		}
	};
	template<class T>
	class add_vertex_attribute_description : public T {
	public:
		using parent = T;
		auto get_vertex_attribute_descriptions() {
			auto descs = parent::get_vertex_attribute_descriptions();
			auto format = parent::get_vertex_input_attribute_format();
			descs.emplace_back(
				vk::VertexInputAttributeDescription{}
				.setLocation(0)
				.setBinding(0)
				.setOffset(0)
				.setFormat(format)
			);
			return descs;
		}
	};
	template<class T>
	class add_empty_vertex_attribute_descriptions : public T {
	public:
		auto get_vertex_attribute_descriptions() {
			return std::vector<vk::VertexInputAttributeDescription>{};
		}
	};
	template<vk::VertexInputRate InputRate, class T>
	class set_input_rate : public T {
	public:
		auto get_input_rate() {
			return InputRate;
		}
	};
	template<uint32_t Binding, class T>
	class set_binding : public T {
	public:
		auto get_binding() {
			return Binding;
		}
	};
	template<uint32_t Stride, class T>
	class set_stride : public T {
	public:
		auto get_stride() {
			return Stride;
		}
	};
	template<class T>
	class add_vertex_binding_description : public T {
	public:
		using parent = T;
		auto get_vertex_binding_descriptions() {
			auto descs = parent::get_binding_descriptions();
			uint32_t binding = parent::get_binding();
			vk::VertexInputRate input_rate = parent::get_input_rate();
			uint32_t stride = parent::get_stride();
			descs.emplace_back(
				vk::VertexInputBindingDescription{}
				.setBinding(binding)
				.setInputRate(input_rate)
				.setStride(stride)
			);
			return descs;
		}
	};
	template<class T>
	class add_empty_binding_descriptions : public T {
	public:
		auto get_binding_descriptions() {
			return std::vector<vk::VertexInputBindingDescription>{};
		}
	};
	template<uint32_t Count, class T>
	class set_tessellation_patch_control_point_count : public T {
	public:
		auto get_pipeline_tessellation_state_create_info() {
			return vk::PipelineTessellationStateCreateInfo{}
			.setPatchControlPoints(Count);
		}
	};
	template<class T>
	class add_pipeline_stage_to_stages : public T {
	public:
		using parent = T;
		auto get_pipeline_stages() {
			auto stages = parent::get_pipeline_stages();
			auto stage = parent::get_pipeline_stage();
			stages.emplace_back(stage);
			return stages;
		}
	};
	template< class T>
	class add_pipeline_stage : public T {
	public:
		using parent = T;
		auto get_pipeline_stage() {
			vk::ShaderModule shader_module = parent::get_shader_module();
			m_entry_name = parent::get_shader_entry_name();
			vk::ShaderStageFlagBits stage = parent::get_shader_stage();
			return vk::PipelineShaderStageCreateInfo{}
				.setModule(shader_module)
				.setPName(m_entry_name.data())
				.setStage(stage);
		}
	private:
		std::string m_entry_name;
	};
	template<vk::ShaderStageFlagBits Shader_stage, class T>
	class set_shader_stage : public T {
	public:
		auto get_shader_stage() {
			return Shader_stage;
		}
	};
	template<class T>
	class set_shader_entry_name_with_main : public T {
	public:
		auto get_shader_entry_name() {
			return std::string{ "main" };
		}
	};
	template<class T>
	class add_shader_module : public T {
	public:
		using parent = T;
		add_shader_module() {
			vk::Device device = parent::get_device();
			auto code = parent::get_spirv_code();
			m_module = device.createShaderModule(
				vk::ShaderModuleCreateInfo{}
				.setCode(code)
			);
		}
		~add_shader_module() {
			vk::Device device = parent::get_device();
			device.destroyShaderModule(m_module);
		}
		auto get_shader_module() {
			return m_module;
		}
	private:
		vk::ShaderModule m_module;
	};
	template<class T>
	class add_spirv_code : public T {
	public:
		using parent = T;
		auto get_spirv_code() {
			void* ptr = parent::get_code_pointer();
			std::uint32_t size = parent::get_size_in_bytes();
			return std::span{ reinterpret_cast<uint32_t*>(ptr), size / 4 };
		}
	};
	template<class T>
	class adapte_map_file_to_spirv_code : public T {
	public:
		using parent = T;
		auto get_code_pointer() {
			return parent::get_mapped_pointer();
		}
		auto get_size_in_bytes() {
			return parent::get_file_size();
		}
	};
	template<class T>
	class map_file_mapping : public T {
	public:
		using parent = T;
		map_file_mapping() {
			HANDLE mapping = parent::get_file_mapping();
			m_memory = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
			if (m_memory == INVALID_HANDLE_VALUE) {
				throw std::runtime_error{ "failed to map view of file" };
			}
		}
		~map_file_mapping() {
			UnmapViewOfFile(m_memory);
		}
		auto get_mapped_pointer() {
			return m_memory;
		}
	private:
		void* m_memory;
	};
	template<class T>
	class cache_file_size : public T {
	public:
		using parent = T;
		cache_file_size() {
			HANDLE file = parent::get_file();
			m_size = GetFileSize(file, NULL);
		}
		auto get_file_size() {
			return m_size;
		}
	private:
		uint32_t m_size;
	};
	template<class T>
	class add_file_mapping : public T {
	public:
		using parent = T;
		add_file_mapping() {
			uint64_t maximum_size{ 0 };
			HANDLE file = parent::get_file();
			m_mapping = CreateFileMapping(file, nullptr, PAGE_READONLY,
				static_cast<uint32_t>(maximum_size >> 32), static_cast<uint32_t>(maximum_size),
				nullptr);
			if (m_mapping == INVALID_HANDLE_VALUE) {
				throw std::runtime_error{ "failed to create file mapping" };
			}
		}
		~add_file_mapping() {
			CloseHandle(m_mapping);
		}
		auto get_file_mapping() {
			return m_mapping;
		}
	private:
		HANDLE m_mapping;
	};
	template<class T>
	class add_file : public T {
	public:
		using parent = T;
		add_file() {
			auto path = parent::get_file_path();
			m_file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_file == INVALID_HANDLE_VALUE) {
				throw std::runtime_error{ "failed to create file" };
			}
		}
		~add_file() {
			CloseHandle(m_file);
		}
		auto get_file() {
			return m_file;
		}
	private:
		HANDLE m_file;
	};
	template<class T>
	class add_vertex_shader_path : public T {
	public:
		auto get_file_path() {
			return std::filesystem::path{ "vert.spv" };
		}
	};
	template<class T>
	class add_fragment_shader_path : public T {
	public:
		auto get_file_path() {
			return std::filesystem::path{ "frag.spv" };
		}
	};
	template<class T>
	class add_empty_pipeline_stages : public T {
	public:
		auto get_pipeline_stages() {
			return std::vector<vk::PipelineShaderStageCreateInfo>{};
		}
	};
	template<vk::PolygonMode Polygon_mode, class T>
	class set_pipeline_rasterization_polygon_mode : public T {
	public:
		auto get_pipeline_rasterization_state_create_info() {
			return vk::PipelineRasterizationStateCreateInfo{}
				.setPolygonMode(Polygon_mode)
				.setLineWidth(1.0f)
				.setCullMode(vk::CullModeFlagBits::eNone);
		}
	};
	template<class T>
	class disable_pipeline_multisample : public T {
	public:
		auto get_pipeline_multisample_state_create_info() {
			return vk::PipelineMultisampleStateCreateInfo{}
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);
		}
	};
	template<vk::PrimitiveTopology Topology, class T>
	class set_pipeline_input_topology : public T {
	public:
		auto get_pipeline_input_assembly_state_create_info() {
			return vk::PipelineInputAssemblyStateCreateInfo{}
			.setTopology(Topology);
		}
	};
	template<class T>
	class disable_pipeline_dynamic : public T {
	public:
		auto get_pipeline_dynamic_state_create_info() {
			return vk::PipelineDynamicStateCreateInfo{};
		}
	};
	template<class T>
	class enable_pipeline_depth_test : public T {
	public:
		auto get_pipeline_depth_stencil_state_create_info() {
			return vk::PipelineDepthStencilStateCreateInfo{}
				.setDepthTestEnable(true)
				.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
				.setDepthWriteEnable(true);
		}
	};
	template<class T>
	class disable_pipeline_depth_stencil : public T {
	public:
		auto get_pipeline_depth_stencil_state_create_info() {
			return vk::PipelineDepthStencilStateCreateInfo{}
				.setDepthTestEnable(false)
				.setDepthCompareOp(vk::CompareOp::eAlways);
		}
	};
	template<class T>
	class add_pipeline_color_blend_state_create_info : public T {
	public:
		using parent = T;
		auto get_pipeline_color_blend_state_create_info() {
			m_attachments = parent::get_pipeline_color_blend_attachment_states();
			return vk::PipelineColorBlendStateCreateInfo{}
				.setAttachments(
					m_attachments
				);
		}
	private:
		std::vector<vk::PipelineColorBlendAttachmentState> m_attachments;
	};
	template<uint32_t AttachmentIndex, class T>
	class disable_pipeline_attachment_color_blend : public T {
	public:
		using parent = T;
		auto get_pipeline_color_blend_attachment_states() {
			auto attachments = parent::get_pipeline_color_blend_attachment_states();
			attachments[AttachmentIndex]
				.setBlendEnable(false);
			return attachments;
		}
	};
	template<uint32_t AttachmentCount, class T>
	class add_pipeline_color_blend_attachment_states : public T {
	public:
		auto get_pipeline_color_blend_attachment_states() {
			auto states = std::vector<vk::PipelineColorBlendAttachmentState>(AttachmentCount);
			std::ranges::for_each(states, [](auto& state) {
				state.setColorWriteMask(vk::ColorComponentFlagBits::eA |
					vk::ColorComponentFlagBits::eR |
					vk::ColorComponentFlagBits::eG |
					vk::ColorComponentFlagBits::eB);
				});
			return states;
		}
	};
	template<class T>
	class add_pipeline_layout : public T {
	public:
		using parent = T;
		add_pipeline_layout() {
			vk::Device device = parent::get_device();
			auto set_layouts = parent::get_descriptor_set_layouts();

			m_layout = device.createPipelineLayout(
				vk::PipelineLayoutCreateInfo{}
				.setSetLayouts(set_layouts)
			);
		}
		~add_pipeline_layout() {
			vk::Device device = parent::get_device();
			device.destroyPipelineLayout(m_layout);
		}
		auto get_pipeline_layout() {
			return m_layout;
		}
	private:
		vk::PipelineLayout m_layout;
	};
	template<class T>
	class add_single_descriptor_set_layout : public T {
	public:
		using parent = T;
		auto get_descriptor_set_layouts() {
			return parent::get_descriptor_set_layout();
		}
	};
	template<class T>
	class add_descriptor_set_layout : public T {
	public:
		using parent = T;
		add_descriptor_set_layout() {
			vk::Device device = parent::get_device();
			auto bindings = parent::get_descriptor_set_layout_bindings();
			m_layout = device.createDescriptorSetLayout(
				vk::DescriptorSetLayoutCreateInfo{}
				.setBindings(bindings)
			);
		}
		~add_descriptor_set_layout() {
			vk::Device device = parent::get_device();
			device.destroyDescriptorSetLayout(m_layout);
		}
		auto get_descriptor_set_layout() {
			return m_layout;
		}
	private:
		vk::DescriptorSetLayout m_layout;
	};
	template<class T>
	class add_descriptor_set_layout_binding : public T {
	public:
		using parent = T;
		add_descriptor_set_layout_binding() {
			uint32_t binding = parent::get_binding();
			uint32_t descriptor_count = parent::get_descriptor_count();
			vk::DescriptorType descriptor_type = parent::get_desciptor_type();
			auto immutable_samplers = parent::get_immutable_samplers();
			vk::PipelineStageFlags stage_flags = parent::get_pipeline_stage_flags();
			m_binding = vk::DescriptorSetLayoutBinding{}
				.setBinding(binding)
				.setDescriptorCount(descriptor_count)
				.setDescriptorType(descriptor_type)
				.setImmutableSamplers(immutable_samplers)
				.setStageFlags(stage_flags);
		}
		auto get_descriptor_set_layout_binding() {
			return m_binding;
		}
	private:
		vk::DescriptorSetLayoutBinding m_binding;
	};
	template<uint32_t Descriptor_count, class T>
	class set_descriptor_count : public T {
	public:
		auto get_descriptor_count() {
			return Descriptor_count;
		}
	};
	template<vk::DescriptorType Descriptor_type, class T>
	class set_descriptor_type : public T {
	public:
		auto get_descriptor_type() {
			return Descriptor_type;
		}
	};
	template<class T>
	class add_empty_immutable_samplers : public T {
	public:
		auto get_immutable_samplers() {
			return std::vector<vk::Sampler>{};
		}
	};
	template<class T>
	class add_recreate_surface : public T {
	public:
		using parent = T;
		void recreate_surface() {
			parent::destroy_surface();
			parent::create_surface();
		}
	};
}


namespace vulkan_helper {
    namespace concept_helper {
        template<class Instance>
        concept instance = requires (Instance instance) {
            instance.get_vulkan_instance();
        };
        template<class Instance>
        concept instance_help_functions = requires (Instance instance) {
            instance.get_first_physical_device();
        };
        namespace instance_helper {
            template<class T>
            concept get_extensions = requires(T t) {
                t.get_extensions();
            };
        }
        template<class PhysicalDevice>
        concept physical_device = requires (PhysicalDevice physical_device) {
            physical_device.get_vulkan_physical_device();
        };
        template<class Device>
        concept device = requires (Device device) {
            device.get_vulkan_device();
        };
    }

    class empty_class {};

    template<concept_helper::instance instance>
    class add_instance_function_wrapper : public instance {
    public:
        VkPhysicalDevice get_first_physical_device() {
            uint32_t count = 1;
            VkPhysicalDevice physical_device;
            auto res = vkEnumeratePhysicalDevices(instance::get_vulkan_instance(), &count, &physical_device);
            return physical_device;
        }
        template<int NUM>
        uint32_t enumerate_limit_physical_devices(std::array<VkPhysicalDevice, NUM>& physical_devices) {
            uint32_t count = physical_devices.size();
            auto res = vkEnumeratePhysicalDevices(instance::get_vulkan_instance(), &count, physical_devices.data());
            if (res != VK_INCOMPLETE || res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to enumerate physical devices" };
            }
            return count;
        }
        void foreach_physical_device(std::invocable<VkPhysicalDevice> auto&& fun) {
            constexpr uint32_t COUNT = 8;
            std::array<VkPhysicalDevice, COUNT> physical_devices{};
            uint32_t count = COUNT;
            auto res = vkEnumeratePhysicalDevices(instance::get_vulkan_instance(), &count, physical_devices.data());
            if (res == VK_INCOMPLETE) {
                throw std::runtime_error("too more physical device");
            }
            else if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to enumerate physical devices" };
            }
            for (int i = 0; i < count; i++) {
                fun(physical_devices[i]);
            }
        }
    };

    template<class T>
    requires concept_helper::instance_helper::get_extensions<T>
	class instance : public T{
    public:
        using parent = T;
        instance() : T{} {
            VkApplicationInfo application_info{};
            {
                auto& info = application_info;
                info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                info.apiVersion = VK_API_VERSION_1_3;
            }
            VkInstanceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            create_info.pApplicationInfo = &application_info;
            auto exts = parent::get_extensions();
            create_info.enabledExtensionCount = exts.size();
            create_info.ppEnabledExtensionNames = exts.data();
            auto res = vkCreateInstance(&create_info, NULL, &m_instance);
            if (res != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance");
            }
        }
        ~instance() {
            vkDestroyInstance(m_instance, NULL);
        }
        
        auto get_vulkan_instance() {
            return m_instance;
        }
    private:
        VkInstance m_instance;
	};

    template<class T>
    class add_empty_extensions : public T {
    public:
        auto get_extensions() {
            return std::vector<const char*>{};
        }
    };

    template<class T>
    class info_chain {
    public:
        
    private:
        T info;
        std::unique_ptr<void> next;
    };

    class device_create_info {
    public:
        constexpr device_create_info() : m_create_info{}, m_queue_family_index{} {
            m_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        }
        void enable_synchronization2() {
            if (m_create_info.pNext == nullptr) {
                auto* vulkan_1_3_features = new VkPhysicalDeviceVulkan13Features;
                vulkan_1_3_features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
                vulkan_1_3_features->synchronization2 = VK_TRUE;
                auto* features2 = new VkPhysicalDeviceFeatures2;
                features2->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                features2->pNext = vulkan_1_3_features;
            }
            else {

            }
        }
        void enable_maintenance4() {
            if (m_create_info.pNext == nullptr) {
                auto* vulkan_1_3_features = new VkPhysicalDeviceVulkan13Features;
                vulkan_1_3_features->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
                vulkan_1_3_features->maintenance4 = VK_TRUE;
                auto* features2 = new VkPhysicalDeviceFeatures2;
                features2->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
                features2->pNext = vulkan_1_3_features;
            }
            else {

            }
        }
        auto set_queue_family_index(uint32_t index) {
            m_queue_family_index = index;
            return *this;
        }
        uint32_t get_queue_family_index() const {
            return m_queue_family_index;
        }
    private:
        VkDeviceCreateInfo m_create_info;
        int m_queue_family_index;
    };
    
    template<concept_helper::physical_device physical_device>
    class add_physical_device_wrapper_functions : public physical_device{
    public:
        uint32_t find_queue_family_if(std::predicate<VkQueueFamilyProperties> auto&& fun) {
            constexpr uint32_t COUNT = 8;
            std::array<VkQueueFamilyProperties, COUNT> properties{};
            uint32_t count = COUNT;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device::get_vulkan_physical_device(), &count, properties.data());
            assert(count > 0);
            for (uint32_t i = 0; i < count; i++) {
                auto& property = properties[i];
                if (fun(property)) {
                    return i;
                }
            }
            throw std::runtime_error{ "failed to find queue family" };
        }
        auto get_physical_device_memory_properties() {
            VkPhysicalDeviceMemoryProperties properties{};
            vkGetPhysicalDeviceMemoryProperties(physical_device::get_vulkan_physical_device(), &properties);
            return properties;
        }
        auto get_memory_properties() {
            return get_physical_device_memory_properties();
        }
        VkDevice create_device(const device_create_info& info) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = info.get_queue_family_index();
            queue_create_info.queueCount = 1;
            float priority = 1.0;
            queue_create_info.pQueuePriorities = &priority;

            VkPhysicalDeviceVulkan13Features vulkan_1_3_features{};
            vulkan_1_3_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
            vulkan_1_3_features.synchronization2 = VK_TRUE;
            vulkan_1_3_features.maintenance4 = VK_TRUE;

            VkPhysicalDeviceFeatures2 features2{};
            features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            features2.pNext = &vulkan_1_3_features;

            VkDeviceCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            create_info.pNext = &features2;
            create_info.queueCreateInfoCount = 1;
            create_info.pQueueCreateInfos = &queue_create_info;

            VkDevice device;
            auto res = vkCreateDevice(physical_device::get_vulkan_physical_device(), &create_info, NULL, &device);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to create device" };
            }
            return device;
        }
        
        VkPhysicalDevice operator()(VkPhysicalDevice) {
            return physical_device::get_vulkan_physical_device();
        }
    };

    uint32_t findProperties(VkPhysicalDeviceMemoryProperties memory_properties, uint32_t memoryTypeBitsRequirements, VkMemoryPropertyFlags requiredProperty) {
        const uint32_t memoryCount = memory_properties.memoryTypeCount;
        for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; memoryIndex++) {
            const uint32_t memoryTypeBits = (1 << memoryIndex);
            const bool isRequiredMemoryType = memoryTypeBitsRequirements & memoryTypeBits;
            const VkMemoryPropertyFlags properties =
                memory_properties.memoryTypes[memoryIndex].propertyFlags;
            const bool hasRequiredProperties =
                (properties & requiredProperty) == requiredProperty;
            if (isRequiredMemoryType && hasRequiredProperties)
                return memoryIndex;
        }
        throw std::runtime_error{ "failed find memory property" };
    }

    template<concept_helper::device device>
    class add_device_wrapper_functions : public device {
    public:
        VkQueue get_device_queue(uint32_t queue_family_index, uint32_t queue_index) {
            VkQueue queue;
            vkGetDeviceQueue(device::get_vulkan_device(), queue_family_index, queue_index, &queue);
            return queue;
        }
        VkFence create_fence() {
            VkFenceCreateInfo fence_create_info{};
            VkFence fence;
            {
                auto& info = fence_create_info;
                info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
                auto res = vkCreateFence(device::get_vulkan_device(), &fence_create_info, NULL, &fence);
                if (res != VK_SUCCESS) {
                    throw std::runtime_error{ "failed to create fence" };
                }
            }
            return fence;
        }
        void destroy_fence(VkFence fence) {
            vkDestroyFence(device::get_vulkan_device(), fence, nullptr);
        }
        VkShaderModule create_shader_module(const spirv_file& file) {
            VkShaderModuleCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.codeSize = file.size();
            create_info.pCode = file.data();
            VkShaderModule shader_module;
            auto res = vkCreateShaderModule(device::get_vulkan_device(), &create_info, NULL, &shader_module);
            if (res != VK_SUCCESS) {
                throw std::runtime_error("failed to create shader module");
            }
            return shader_module;
        }

        void destroy_shader_module(VkShaderModule shader_module) {
            vkDestroyShaderModule(device::get_vulkan_device(), shader_module, nullptr);
        }

        auto create_descriptor_set_layout(VkDescriptorSetLayoutCreateInfo* create_info) {


            VkDescriptorSetLayout descriptor_set_layout;
            auto res = vkCreateDescriptorSetLayout(device::get_vulkan_device(), create_info, NULL, &descriptor_set_layout);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to create descriptor set layout" };
            }
            return descriptor_set_layout;
        }
        void destroy_descriptor_set_layout(VkDescriptorSetLayout layout) {
            vkDestroyDescriptorSetLayout(device::get_vulkan_device(), layout, NULL);
        }

        auto create_pipeline_layout(VkDescriptorSetLayout descriptor_set_layout) {
            VkPipelineLayoutCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            create_info.flags = 0;
            create_info.setLayoutCount = 1;
            create_info.pSetLayouts = &descriptor_set_layout;
            VkPipelineLayout pipeline_layout;
            auto res = vkCreatePipelineLayout(device::get_vulkan_device(), &create_info, NULL, &pipeline_layout);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to create pipeline layout" };
            }
            return pipeline_layout;
        }
        void destroy_pipeline_layout(VkPipelineLayout pipeline_layout) {
            vkDestroyPipelineLayout(device::get_vulkan_device(), pipeline_layout, NULL);
        }
        auto create_pipeline(VkShaderModule shader_module, VkPipelineLayout pipeline_layout) {
            VkComputePipelineCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
            create_info.flags = 0;
            create_info.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            create_info.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
            create_info.stage.module = shader_module;
            create_info.stage.pName = "main";
            create_info.layout = pipeline_layout;

            VkPipeline pipeline;
            auto res = vkCreateComputePipelines(device::get_vulkan_device(), VK_NULL_HANDLE, 1, &create_info, NULL, &pipeline);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to create compute pipeline" };
            }
            return pipeline;
        }
        void destroy_pipeline(VkPipeline pipeline) {
            vkDestroyPipeline(device::get_vulkan_device(), pipeline, NULL);
        }

        auto create_command_pool(uint32_t queue_family_index) {
            VkCommandPoolCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            create_info.queueFamilyIndex = queue_family_index;
            VkCommandPool command_pool;
            auto res = vkCreateCommandPool(device::get_vulkan_device(), &create_info, NULL, &command_pool);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to create command pool" };
            }
            return command_pool;
        }
        void destroy_command_pool(VkCommandPool command_pool) {
            vkDestroyCommandPool(device::get_vulkan_device(), command_pool, NULL);
        }

        auto allocate_command_buffer(VkCommandPool command_pool) {
            VkCommandBufferAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            info.commandPool = command_pool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = 1;
            VkCommandBuffer command_buffer;
            auto ret = vkAllocateCommandBuffers(device::get_vulkan_device(), &info, &command_buffer);
            if (ret != VK_SUCCESS) {
                throw std::runtime_error{ "failed to allocate command buffer" };
            }
            return command_buffer;
        }

        VkBuffer create_buffer(uint32_t queue_family_index, VkDeviceSize size, VkBufferUsageFlags usage) {
            VkBufferCreateInfo create_info{};
            create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            create_info.size = size;
            create_info.usage = usage;
            create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.queueFamilyIndexCount = 1;
            create_info.pQueueFamilyIndices = &queue_family_index;

            VkBuffer buffer;
            auto res = vkCreateBuffer(device::get_vulkan_device(), &create_info, NULL, &buffer);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to create buffer" };
            }
            return buffer;
        }
        void destroy_buffer(VkBuffer buffer) {
            vkDestroyBuffer(device::get_vulkan_device(), buffer, NULL);
        }


        VkDeviceMemory alloc_device_memory(VkPhysicalDeviceMemoryProperties memory_properties, VkBuffer buffer, VkMemoryPropertyFlags property) {
            VkMemoryRequirements requirements;
            vkGetBufferMemoryRequirements(device::get_vulkan_device(), buffer, &requirements);
            uint32_t memoryType = findProperties(memory_properties, requirements.memoryTypeBits, property);

            VkDeviceMemory device_memory{};
            {
                VkMemoryAllocateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                info.allocationSize = requirements.size;
                info.memoryTypeIndex = memoryType;
                auto res = vkAllocateMemory(device::get_vulkan_device(), &info, NULL, &device_memory);
                if (res != VK_SUCCESS) {
                    throw std::runtime_error{ "failed to allocate device memory" };
                }
            }

            vkBindBufferMemory(device::get_vulkan_device(), buffer, device_memory, 0);
            return device_memory;
        }
        VkDeviceMemory alloc_device_memory(VkPhysicalDeviceMemoryProperties memory_properties, VkImage image, VkMemoryPropertyFlags property) {
            VkMemoryRequirements requirements;
            vkGetImageMemoryRequirements(device::get_vulkan_device(), image, &requirements);
            uint32_t memoryType = findProperties(memory_properties, requirements.memoryTypeBits, property);

            VkDeviceMemory device_memory{};
            {
                VkMemoryAllocateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
                info.allocationSize = requirements.size;
                info.memoryTypeIndex = memoryType;
                auto res = vkAllocateMemory(device::get_vulkan_device(), &info, NULL, &device_memory);
                if (res != VK_SUCCESS) {
                    throw std::runtime_error{ "failed to allocate device memory" };
                }
            }

            vkBindImageMemory(device::get_vulkan_device(), image, device_memory, 0);
            return device_memory;
        }
        void free_device_memory(VkDeviceMemory device_memory) {
            vkFreeMemory(device::get_vulkan_device(), device_memory, NULL);
        }
        void* map_device_memory(VkDeviceMemory device_memory, VkDeviceSize offset, VkDeviceSize size) {
            void* ptr{};
            auto res = vkMapMemory(device::get_vulkan_device(), device_memory, offset, size, 0, &ptr);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to map device memory" };
            }
            return ptr;
        }
        void unmap_device_memory(VkDeviceMemory device_memory) {
            vkUnmapMemory(device::get_vulkan_device(), device_memory);
        }

        auto create_descriptor_pool(VkDescriptorPoolCreateInfo* create_info) {
            VkDescriptorPool descriptor_pool;
            vkCreateDescriptorPool(device::get_vulkan_device(), create_info, NULL, &descriptor_pool);
            return descriptor_pool;
        }

        void destroy_descriptor_pool(VkDescriptorPool descriptor_pool) {
            vkDestroyDescriptorPool(device::get_vulkan_device(), descriptor_pool, NULL);
        }

        auto allocate_descriptor_set(VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout) {
            VkDescriptorSetAllocateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            info.descriptorPool = descriptor_pool;
            info.descriptorSetCount = 1;
            info.pSetLayouts = &descriptor_set_layout;
            VkDescriptorSet descriptor_set;
            auto res = vkAllocateDescriptorSets(device::get_vulkan_device(), &info, &descriptor_set);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to allocate descriptor set" };
            }
            return descriptor_set;
        }
        void update_descriptor_set(const VkWriteDescriptorSet& write) {
            vkUpdateDescriptorSets(device::get_vulkan_device(), 1, &write, 0, NULL);
        }

        void reset_fence(VkFence fence) {
            if (VK_SUCCESS != vkResetFences(device::get_vulkan_device(), 1, &fence)) {
                throw std::runtime_error{ "failed to reset fence" };
            }
        }

        void wait_for_fence(VkFence fence) {
            auto res = vkWaitForFences(device::get_vulkan_device(), 1, &fence, VK_TRUE, UINT64_MAX);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "wait fence fail" };
            }
        }

        void invalidate_mapped_memory_ranges(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size) {
            VkMappedMemoryRange memory_range{};
            memory_range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            memory_range.memory = memory;
            memory_range.offset = offset;
            memory_range.size = size;
            auto res = vkInvalidateMappedMemoryRanges(device::get_vulkan_device(), 1, &memory_range);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to invalidate mapped memory" };
            }
        }

        VkImage create_image(VkImageCreateInfo* create_info) {
            VkImage image{};
            VkResult res = vkCreateImage(device::get_vulkan_device(), create_info, nullptr, &image);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to create image" };
            }
            return image;
        }
        void destroy_image(VkImage image) {
            vkDestroyImage(device::get_vulkan_device(), image, nullptr);
        }
        VkImageView create_image_view(VkImageViewCreateInfo* create_info) {
            VkImageView view{};
            VkResult res = vkCreateImageView(device::get_vulkan_device(), create_info, nullptr, &view);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to create image view" };
            }
            return view;
        }
        void destroy_image_view(VkImageView view) {
            vkDestroyImageView(device::get_vulkan_device(), view, nullptr);
        }
    };

    template<concept_helper::physical_device physical_device>
    class device : public physical_device {
    public:
        device(const device_create_info& create_info)
            :
            m_device{ physical_device::create_device(create_info) }
        {}
        device() = delete;
        device(const device& device) = delete;
        device(device&& device) = delete;
        ~device() noexcept {
            vkDestroyDevice(m_device, nullptr);
        }
        device& operator=(const device& device) = delete;
        device& operator=(device&& device) = delete;

        VkDevice get_vulkan_device() {
            return m_device;
        }
    private:
        VkDevice m_device;
    };

    template<class D>
    class command_pool : public D{
    public:
        command_pool(std::invocable<D&> auto&& gen_command_pool) : m_command_pool { gen_command_pool(*this) }
        {}
        command_pool() = delete;
        command_pool(const command_pool&) = delete;
        command_pool(command_pool&&) = delete;
        ~command_pool() {
            D::destroy_command_pool(m_command_pool);
        }
        command_pool& operator=(const command_pool&) = delete;
        command_pool& operator=(command_pool&&) = delete;

        VkCommandPool get_command_pool() {
            return m_command_pool;
        }

    private:
        VkCommandPool m_command_pool;
    };

    template<class D>
    class fence : public D {
    public:
        fence() : m_fence{D::create_fence()}
        {}
        ~fence() {
            D::destroy_fence(m_fence);
        }
        VkFence get_fence() {
            return m_fence;
        }

        void reset() {
            D::reset_fence(m_fence);
        }
        void wait_for() {
            D::wait_for_fence(m_fence);
        }
    private:
        VkFence m_fence;
    };

    template<class D>
    class descriptor_set : public D {
    public:
    public:
        descriptor_set() : m_descriptor_set{ D::allocate_descriptor_set(D::get_descriptor_set_layout())}
        {}

        auto get_descriptor_set() {
            return m_descriptor_set;
        }
    private:
        VkDescriptorSet m_descriptor_set;
    };

    template<class D>
    class pipeline_layout : public D {
    public:
        pipeline_layout() : m_pipeline_layout{D::create_pipeline_layout(D::get_descriptor_set_layout())}
        {}
        ~pipeline_layout() {
            D::destroy_pipeline_layout(m_pipeline_layout);
        }
        auto get_pipeline_layout() const{
            return m_pipeline_layout;
        }
    private:
        VkPipelineLayout m_pipeline_layout;
    };

    template<class D>
    class shader_module {
    public:
        shader_module(D& device, const spirv_file& file) : m_device{ device }, m_shader_module { device.create_shader_module(file) }
        {}
        ~shader_module() {
            m_device.destroy_shader_module(m_shader_module);
        }

        auto get_shader_module() const{
            return m_shader_module;
        }
    private:
        D& m_device;
        VkShaderModule m_shader_module;
    };

    template<class D>
    class pipeline : public D {
    public:
        pipeline(std::invocable<D&> auto&& generate_shader_module) : m_pipeline{ D::create_pipeline(generate_shader_module(*this).get_shader_module(), D::get_pipeline_layout())}
        {}
        ~pipeline() {
            D::destroy_pipeline(m_pipeline);
        }
        auto get_pipeline() const {
            return m_pipeline;
        }
    private:
        VkPipeline m_pipeline;
    };

    template<class D>
    class command_buffer : public D {
    public:
        command_buffer() : m_command_buffer{D::allocate_command_buffer(D::get_command_pool())}
        {}
        auto get_command_buffer() const{
            return m_command_buffer;
        }
        void begin() {
            VkCommandBufferBeginInfo info{};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            auto res = vkBeginCommandBuffer(m_command_buffer, &info);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to begin command buffer" };
            }
        }
        void end() {
            auto res = vkEndCommandBuffer(m_command_buffer);
            if (res != VK_SUCCESS) {
                throw std::runtime_error{ "failed to end command buffer" };
            }
        }
        void bind_pipeline(VkPipelineBindPoint bind_point, VkPipeline pipeline) {
            vkCmdBindPipeline(m_command_buffer, bind_point, pipeline);
        }
        void bind_descriptor_sets(VkPipelineBindPoint bind_point, VkPipelineLayout layout, VkDescriptorSet descriptor_set) {
            vkCmdBindDescriptorSets(m_command_buffer, bind_point,
                layout, 0, 1, &descriptor_set, 0, NULL);
        }
        void dispatch(uint32_t x, uint32_t y, uint32_t z) {
            vkCmdDispatch(m_command_buffer, x, y, z);
        }
        void pipeline_barrier(
            VkPipelineStageFlags src_stage_mask,
            VkPipelineStageFlags dst_stage_mask,
            VkDependencyFlags dependency_flags,
            uint32_t memory_barrier_count,
            const VkMemoryBarrier* memory_barriers,
            uint32_t buffer_memory_barrier_count,
            const VkBufferMemoryBarrier* buffer_memory_barriers,
            uint32_t image_memory_barrier_count,
            const VkImageMemoryBarrier* image_memory_barriers) {
            vkCmdPipelineBarrier(m_command_buffer, src_stage_mask, dst_stage_mask, dependency_flags, memory_barrier_count, memory_barriers, buffer_memory_barrier_count, buffer_memory_barriers, image_memory_barrier_count, image_memory_barriers);
        }
        void clear_color_image(VkImage image, VkImageLayout layout, const VkClearColorValue* clear_color, uint32_t range_count, const VkImageSubresourceRange* ranges) {
            vkCmdClearColorImage(m_command_buffer, image, layout, clear_color, range_count, ranges);
        }
    private:
        VkCommandBuffer m_command_buffer;
    };

    template<class D>
    class add_storage_buffer : public D {
    public:
        add_storage_buffer() : m_storage_buffer{ D::create_buffer(D::get_queue_family_index(), D::get_storage_buffer_size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)}
        {}
        ~add_storage_buffer() {
            D::destroy_buffer(m_storage_buffer);
        }
        auto get_storage_buffer() const {
            return m_storage_buffer;
        }
    private:
        VkBuffer m_storage_buffer;
    };

    template<class D>
    class add_storage_memory : public D {
    public:
        add_storage_memory() : m_storage_memory{ D::alloc_device_memory(D::get_memory_properties(), D::get_storage_buffer(), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) }
        {}
        ~add_storage_memory() {
            D::free_device_memory(m_storage_memory);
        }
        auto get_storage_memory() const {
            return m_storage_memory;
        }
    private:
        VkDeviceMemory m_storage_memory;
    };

    template<class D>
    class add_storage_memory_ptr : public D {
    public:
        add_storage_memory_ptr() : m_storage_memory_ptr{ D::map_device_memory(D::get_storage_memory(), 0, VK_WHOLE_SIZE)}
        {}
        ~add_storage_memory_ptr() {
            D::unmap_device_memory(D::get_storage_memory());
        }
        auto get_storage_memory_ptr() const {
            return m_storage_memory_ptr;
        }
    private:
        void* m_storage_memory_ptr;
    };
};