// Linux-only bounded admission for terrain-prop composite construction.
// No existing layout, virtual, serialization or Windows interface changes.
#pragma once
#if defined(__linux__)
#include "clone_graph.h"
#include <array>
#include <typeinfo>

class WW3DAssetManager;
class PrototypeClass;
class RenderObjClass;
class HTreeClass;
class HModelDefClass;
class CollectionDefClass;

namespace ww3d_prop {
class Audit {
	WW3DAssetManager& manager;
	std::array<const void*,256> path{};
	unsigned depth=0,nodes=0;
	void enter(const void*);
	void leave() noexcept { --depth; }
	void prototype(PrototypeClass*);
public:
	explicit Audit(WW3DAssetManager& p):manager(p) {}
	void named(const char*,bool optional=false);
	void render(RenderObjClass*);
	void hierarchy(const HTreeClass*);
	void hmodel(const HModelDefClass&);
	void collection(const CollectionDefClass&);
};
bool inspect_hmodel(PrototypeClass*,Audit&);
bool inspect_collection(PrototypeClass*,Audit&);

class Attempt {
	static inline thread_local Attempt* current=nullptr;
	ww3d_clone::Attempt clone;
	Attempt* previous;
	WW3DAssetManager* owner;
	bool admitted=false;
public:
	explicit Attempt(WW3DAssetManager*);
	~Attempt() noexcept { current=previous; }
	Attempt(const Attempt&)=delete;
	Attempt& operator=(const Attempt&)=delete;
	void commit() noexcept { clone.commit(); }
	static void check(WW3DAssetManager&,PrototypeClass*,const char*);
};
}
#endif
