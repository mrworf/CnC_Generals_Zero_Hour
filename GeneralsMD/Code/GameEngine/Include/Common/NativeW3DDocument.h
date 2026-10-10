// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>
class File;

// Game-owned W3D input and metadata, not a renderer or a resolved model.
// All unknown payloads remain in the owned bytes. Views borrow this document;
// replacement invalidates them only after a complete candidate is accepted.
class NativeW3DDocument final {
public:
  struct Chunk {
    std::uint32_t type=0;
    bool hasSubchunks=false;
    std::size_t offset=0,size=0;
    std::vector<std::size_t> children;
  };
  struct Pivot {
    std::string name;
    std::uint32_t parent=UINT32_MAX;
    std::array<float,3> translation{},euler{};
    std::array<float,4> rotation{};
  };
  struct Hierarchy {
    std::string name;
    std::uint32_t version=0;
    std::array<float,3> center{};
    std::vector<Pivot> pivots;
  };
  struct Subobject {std::uint32_t bone=0;std::string name;};
  struct Lod {float maxScreenSize=0;std::vector<Subobject> objects;};
  struct Hlod {
    std::uint32_t version=0;
    std::string name,hierarchy;
    std::vector<Lod> levels;
    std::vector<Subobject> aggregates,proxies;
  };
  struct Mesh {
    std::uint32_t version=0,attributes=0,triangles=0,vertices=0,materials=0,damageStages=0;
    std::int32_t sortLevel=0;
    std::uint32_t prelitVersion=0,futureCount=0,vertexChannels=0,faceChannels=0;
    std::string name,container;
    std::array<float,3> minimum{},maximum{},sphereCenter{};
    float sphereRadius=0;
    std::size_t chunk=0;
  };
  struct Animation {
    std::uint32_t version=0,frames=0,frameRate=0;
    std::string name,hierarchy;
    bool compressed=false;
    std::uint16_t flavor=0;
    std::size_t chunk=0;
  };
  NativeW3DDocument()=default;
  NativeW3DDocument(const NativeW3DDocument&)=delete;
  NativeW3DDocument& operator=(const NativeW3DDocument&)=delete;
  NativeW3DDocument(NativeW3DDocument&&) noexcept=default;
  NativeW3DDocument& operator=(NativeW3DDocument&&) noexcept=default;
  static NativeW3DDocument decode(std::span<const std::uint8_t>);
  static NativeW3DDocument read(File&);
  void replace(std::span<const std::uint8_t>);
  void swap(NativeW3DDocument&) noexcept;
  std::span<const std::uint8_t> bytes()const noexcept{return m_bytes;}
  std::span<const std::uint8_t> payload(std::size_t chunk)const;
  const auto& chunks()const noexcept{return m_chunks;}
  const auto& roots()const noexcept{return m_roots;}
  const auto& hierarchies()const noexcept{return m_hierarchies;}
  const auto& hlods()const noexcept{return m_hlods;}
  const auto& meshes()const noexcept{return m_meshes;}
  const auto& animations()const noexcept{return m_animations;}
private:
  std::vector<std::uint8_t> m_bytes;
  std::vector<Chunk> m_chunks;
  std::vector<std::size_t> m_roots;
  std::vector<Hierarchy> m_hierarchies;
  std::vector<Hlod> m_hlods;
  std::vector<Mesh> m_meshes;
  std::vector<Animation> m_animations;
  void parse();
  void parseChunks(std::size_t begin,std::size_t end,unsigned depth,std::size_t parent);
  void parseHierarchy(std::size_t);
  void parseHlod(std::size_t);
  void parseMesh(std::size_t);
  void parseAnimation(std::size_t);
};
