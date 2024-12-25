#pragma once

#if defined(WIN32)
#define NOMINMAX
#include <Windows.h>
#endif
#include <vulkan/vulkan.hpp>


namespace vulkan_hpp_helper {

template <class T> class add_win32_surface_extension : public T {
public:
  auto get_extensions() {
#ifdef WIN32
    auto ext = T::get_extensions();
    ext.push_back(vk::KHRWin32SurfaceExtensionName);
    return ext;
#else
    throw std::runtime_error{"Win32 surface is not supported"};
#endif
  }
};

#if defined(WIN32)
template <class T> class map_file_mapping : public T {
public:
  using parent = T;
  map_file_mapping() {
    HANDLE mapping = parent::get_file_mapping();
    m_memory = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
    if (m_memory == INVALID_HANDLE_VALUE) {
      throw std::runtime_error{"failed to map view of file"};
    }
  }
  ~map_file_mapping() { UnmapViewOfFile(m_memory); }
  auto get_mapped_pointer() { return m_memory; }

private:
  void *m_memory;
};
template <class T> class cache_file_size : public T {
public:
  using parent = T;
  cache_file_size() {
    HANDLE file = parent::get_file();
    m_size = GetFileSize(file, NULL);
  }
  auto get_file_size() { return m_size; }

private:
  uint32_t m_size;
};
template <class T> class add_file_mapping : public T {
public:
  using parent = T;
  add_file_mapping() {
    uint64_t maximum_size{0};
    HANDLE file = parent::get_file();
    m_mapping = CreateFileMapping(file, nullptr, PAGE_READONLY,
                                  static_cast<uint32_t>(maximum_size >> 32),
                                  static_cast<uint32_t>(maximum_size), nullptr);
    if (m_mapping == INVALID_HANDLE_VALUE) {
      throw std::runtime_error{"failed to create file mapping"};
    }
  }
  ~add_file_mapping() { CloseHandle(m_mapping); }
  auto get_file_mapping() { return m_mapping; }

private:
  HANDLE m_mapping;
};
template <class T> class add_file : public T {
public:
  using parent = T;
  add_file() {
    auto path = parent::get_file_path();
    m_file = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_file == INVALID_HANDLE_VALUE) {
      throw std::runtime_error{"failed to create file"};
    }
  }
  ~add_file() { CloseHandle(m_file); }
  auto get_file() { return m_file; }

private:
  HANDLE m_file;
};
#endif

template <class T> class add_wayland_surface_extension : public T {
public:
  auto get_extensions()
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
  {
    auto ext = T::get_extensions();
    ext.push_back(vk::KHRWaylandSurfaceExtensionName);
    return ext;
  }
#else
  {
    throw std::runtime_error{"wayland surface is not supported"};
  }
#endif
};

#if linux
template <class T> class map_file_mapping : public T {
public:
  using parent = T;
  map_file_mapping() {
    int fd = parent::get_file_descriptor();
    auto size = parent::get_file_size();
    m_memory = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (m_memory == MAP_FAILED) {
      throw std::runtime_error{"failed to map view of file"};
    }
  }
  ~map_file_mapping() {
    auto size = parent::get_file_size();
    munmap(m_memory, size);
  }
  auto get_mapped_pointer() { return m_memory; }

private:
  void *m_memory;
};
template <class T> class cache_file_size : public T {
public:
  using parent = T;
  cache_file_size() {
    auto fd = parent::get_file_descriptor();
    m_size = lseek(fd, 0, SEEK_END);
  }
  auto get_file_size() { return m_size; }

private:
  uint32_t m_size;
};
template <class T> class add_file_mapping : public T {};
template <class T> class add_file : public T {
public:
  using parent = T;
  add_file() {
    auto path = parent::get_file_path();
    m_file_descriptor = open(path.c_str(), O_RDONLY);
    if (m_file_descriptor == -1) {
      throw std::runtime_error{"failed to create file"};
    }
  }
  ~add_file() { close(m_file_descriptor); }
  auto get_file_descriptor() { return m_file_descriptor; }

private:
  int m_file_descriptor;
};
#endif
}
