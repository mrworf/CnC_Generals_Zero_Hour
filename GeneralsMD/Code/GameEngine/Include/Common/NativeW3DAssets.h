// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "Common/NativeW3DAnimation.h"
#include "Common/NativeW3DPose.h"
#include <exception>
#include <memory>
#include <string_view>

class FileSystem;

// Redacted data-boundary diagnostics: never embed a selector or physical root.
class NativeW3DAssetFailure final : public std::exception {
public:
  enum class Code { MissingFile, MissingDefinition, DependencyCycle, InvalidRelationship };
  explicit NativeW3DAssetFailure(Code value)noexcept:m_code(value){}
  Code code()const noexcept{return m_code;}
  const char* what()const noexcept override;
private:
  Code m_code;
};

class NativeW3DAssets final {
  struct State;
public:
  // Logic-thread admission scope; the parent outlives this nonmovable lease.
  // Coupled consumers may reject after bind without retaining partial caches.
  class Transaction final {
    NativeW3DAssets& m_owner;
    std::shared_ptr<const State> m_previous;
    bool m_committed=false;
  public:
    explicit Transaction(NativeW3DAssets&);
    ~Transaction() noexcept;
    Transaction(const Transaction&)=delete;
    Transaction& operator=(const Transaction&)=delete;
    void commit() noexcept {m_committed=true;}
  };
  using Document=std::shared_ptr<const NativeW3DDocument>;
  using Hierarchy=std::shared_ptr<const NativeW3DDocument::Hierarchy>;
  struct AnimationAsset {
    std::shared_ptr<const NativeW3DAnimation> motion;
    Hierarchy hierarchy;
    Document document;
  };
  using Animation=std::shared_ptr<const AnimationAsset>;
  struct Prototype;
  using Model=std::shared_ptr<const Prototype>;
  struct Attachment {std::uint32_t bone;Model model;};
  struct Lod {float maxScreenSize;std::vector<Attachment> objects;};
  struct Prototype {
    enum class Kind { Mesh, Hlod, Null, AlignedBox, OrientedBox };
    Kind kind;
    std::string name;
    Document document;
    // Mesh/HLOD/box: index into the corresponding document metadata table.
    // Named null: chunk index. Built-in NULL: no document.
    std::size_t metadataIndex=0;
    Hierarchy hierarchy;
    std::vector<Lod> levels;
    std::vector<Attachment> aggregates;
    std::vector<NativeW3DDocument::Subobject> proxies;
  };
  struct Counts {
    std::size_t documents=0,hierarchies=0,definitions=0,animationDefinitions=0,models=0,animations=0;
    bool operator==(const Counts&)const=default;
  };
  struct Binding { Model model; Animation animation; NativeW3DPose pose; };
  // Model, optional clip and query pose are one admission transaction. Even a
  // late pose allocation failure preserves the complete accepted cache state.
  Binding bind(std::string_view model, std::string_view animation, bool finalFrame,
               const NativeW3DPose::Matrix& root, float treeScale);
  // FileSystem is borrowed through admission and must outlive this owner. User
  // art, when supplied, is an explicit read-only directory admitted by that VFS.
  NativeW3DAssets(FileSystem&,std::string language,std::string userModelDirectory={});
  NativeW3DAssets(const NativeW3DAssets&)=delete;
  NativeW3DAssets& operator=(const NativeW3DAssets&)=delete;
  Model model(std::string_view);
  Hierarchy hierarchy(std::string_view);
  Animation animation(std::string_view);
  Counts counts()const noexcept;
  FileSystem& files()const noexcept{return m_files;}
  void clear()noexcept;
  // This is the exact admitted named/default HLOD tree, not a fake tree for an
  // unrecognized/missing model. Mesh/null prototypes have no bone hierarchy.
  static NativeW3DPose createPose(const Model&);
  static void validateMotionBinding(const Model&,const Animation&);
private:
  FileSystem& m_files;
  std::string m_language,m_userDirectory;
  std::shared_ptr<const State> m_state;
  void load(State&,const std::string& filename);
  Hierarchy resolveHierarchy(State&,std::string_view);
  Model resolveModel(State&,std::string_view,std::vector<std::string_view>& ancestors);
  Animation resolveAnimation(State&,std::string_view);
};
