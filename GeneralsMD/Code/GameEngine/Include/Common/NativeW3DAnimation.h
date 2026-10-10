// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/NativeW3DDocument.h"

// Game-owned decoded motion. Admission borrows input only for the duration of
// the call; subsequent sampling needs neither the document nor its hierarchy.
class NativeW3DAnimation final {
public:
  // Runtime single-motion raw channels have signed default-channel semantics.
  // Explicit bounded sampling/blending and compressed channels retain their
  // clip-domain admission; runtime is not a waiver for uint32 conversions.
  enum class RawSampling { GeneralsRounded, Interpolated, RuntimeGeneralsRounded };
  struct Motion {
    std::array<float,3> translation{};
    std::array<float,4> rotation{0,0,0,1};
    bool visible=true;
  };
  NativeW3DAnimation()=default;
  NativeW3DAnimation(const NativeW3DAnimation&)=delete;
  NativeW3DAnimation& operator=(const NativeW3DAnimation&)=delete;
  NativeW3DAnimation(NativeW3DAnimation&&) noexcept=default;
  NativeW3DAnimation& operator=(NativeW3DAnimation&&) noexcept=default;
  static NativeW3DAnimation decode(const NativeW3DDocument&,std::size_t animation,
                                  const NativeW3DDocument::Hierarchy&);
  void replace(const NativeW3DDocument&,std::size_t animation,
               const NativeW3DDocument::Hierarchy&);
  void swap(NativeW3DAnimation&) noexcept;
  Motion sample(std::size_t pivot,float frame,
                RawSampling=RawSampling::GeneralsRounded) const;
  static Motion blend(const Motion&,const Motion&,float percentage);
  std::size_t pivots() const noexcept{return m_nodes.size();}
  std::uint32_t frames() const noexcept{return m_frames;}
  std::uint32_t frameRate() const noexcept{return m_frameRate;}
  const std::string& name() const noexcept{return m_name;}
  const std::string& hierarchy() const noexcept{return m_hierarchy;}
private:
  struct Channel {
    bool present=false,sparse=false;
    std::uint32_t first=0,last=0;
    unsigned width=1;
    std::vector<float> values;
    std::vector<std::uint32_t> times;
  };
  struct Visibility {
    bool present=false,defaultValue=true,sparse=false;
    std::uint32_t first=0,last=0;
    std::vector<std::uint8_t> bits;
    std::vector<std::uint32_t> times;
  };
  struct Node {
    std::array<Channel,7> channels;
    Visibility visibility;
    Node(){channels[6].width=4;}
  };
  std::string m_name,m_hierarchy;
  std::uint32_t m_frames=0,m_frameRate=0;
  bool m_compressed=false;
  std::vector<Node> m_nodes;
  void readChannel(std::span<const std::uint8_t>,std::uint32_t version,unsigned flavor);
  void readVisibility(std::span<const std::uint8_t>,std::uint32_t version);
  std::array<float,4> sampleChannel(const Channel&,float,RawSampling) const;
};
