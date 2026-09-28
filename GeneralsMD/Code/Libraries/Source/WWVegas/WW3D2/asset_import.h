// Linux source-owner scope for bounded native asset imports. No legacy layout
// or virtual/serialized interface is changed; ordinary unscoped loads are intact.
#ifndef WW3D_ASSET_IMPORT_H
#define WW3D_ASSET_IMPORT_H
#if defined(__linux__)
#include <cstddef>
#include <memory>
#include <stdexcept>
#include "hashtemplate.h"
#include "Vector.H"
#include "texturefilter.h"
class WW3DAssetManager;
class HTreeManagerClass;
class HAnimManagerClass;
class HTreeClass;
class HAnimClass;
class HashTableClass;
class HashableClass;
class PrototypeClass;
class TextureClass;
class FileClass;
struct W3DAssetImportProbeAccess;
namespace ww3d_import {
struct PrototypeDelete { void operator()(PrototypeClass*) const noexcept; };
class Attempt {
	friend struct ::W3DAssetImportProbeAccess;
	struct Impl;
	std::unique_ptr<Impl> impl;
	static thread_local Attempt* current;
	static thread_local int fault_ordinal;
	static thread_local unsigned fault_count;
	static void fault();
	static void ini_boundary(std::size_t,std::size_t);
public:
	explicit Attempt(WW3DAssetManager&);
	~Attempt() noexcept;
	Attempt(const Attempt&)=delete;
	Attempt& operator=(const Attempt&)=delete;
	static Attempt* active(const WW3DAssetManager*) noexcept;
	static Attempt* active(const HTreeManagerClass*) noexcept;
	static Attempt* active(const HAnimManagerClass*) noexcept;
	static bool is_active() noexcept { return current!=nullptr; }
	static bool load(WW3DAssetManager&,const char*);
	static void name(const char*,std::size_t suffix=0);
	static void reserve(std::size_t count,std::size_t element);
	static void boundary();
	static bool preflight(FileClass&);
	static void file_provider(const void*,const void* (*)() noexcept);
	template<class T> static void append_candidate(DynamicVectorClass<T>& target,const T& value)
	{
		if (!target.IsValid || target.ActiveCount<0 || target.VectorMax<target.ActiveCount ||
			target.VectorMax>65536 || target.ActiveCount==65536 || (target.VectorMax && !target.Vector))
			throw std::runtime_error("original import candidate vector ownership is malformed");
		int capacity=target.VectorMax;
		if (target.ActiveCount==capacity) {
			if (target.GrowthStep<=0 || target.GrowthStep>65536-capacity)
				throw std::runtime_error("original import candidate vector capacity is exhausted");
			capacity+=target.GrowthStep;
		}
		reserve(capacity,sizeof(T));boundary();std::unique_ptr<T[]> array(new T[capacity]);
		for (int i=0;i<target.ActiveCount;++i) { boundary();array[i]=target.Vector[i]; }
		boundary();array[target.ActiveCount]=value;
		T* old=target.Vector;const bool owned=target.IsAllocated;
		target.Vector=array.release();target.VectorMax=capacity;++target.ActiveCount;
		target.IsValid=true;target.IsAllocated=true;
		if (owned) delete[] old;
	}
	void validate() const;
	bool commit() noexcept;
	PrototypeClass* find_prototype(const char*) const;
	void add_prototype(PrototypeClass*);
	TextureClass* find_texture(const StringClass&) const;
	void add_texture(const StringClass&,TextureClass*);
	void remember_filter(TextureClass*);
	int tree_count() const noexcept;
	int tree_id(const char*) const;
	HTreeClass* tree(int) const noexcept;
	HTreeClass* tree(const StringClass&) const;
	void add_tree(HTreeClass*);
	HashTableClass& animations() noexcept;
	HashTableClass& missing() noexcept;
	void add_animation(HAnimClass*);
	void add_missing(HashableClass*);
	class FileScope {
		Attempt* owner=nullptr;
	public:
		explicit FileScope(const char*);
		~FileScope() noexcept;
		FileScope(const FileScope&)=delete;
	};
};
}
#endif
#endif
