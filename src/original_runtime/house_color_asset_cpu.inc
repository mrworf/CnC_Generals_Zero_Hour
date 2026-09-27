// Linux source-owner transaction for the authored house-color route.
// Included after W3DPrototypeClass; not a generic hash/vector implementation.
#include "clone_graph.h"
namespace {
template<class T> struct HouseColorRef {
	T* value;
	explicit HouseColorRef(T* p=nullptr) noexcept : value(p) {}
	~HouseColorRef() { if (value) value->Release_Ref(); }
	HouseColorRef(const HouseColorRef&)=delete;
	HouseColorRef& operator=(const HouseColorRef&)=delete;
	T* release() noexcept { T* p=value;value=nullptr;return p; }
};
std::size_t house_color_name_size(const char* name, std::size_t minimum=1)
{
	if (!name) throw std::runtime_error("original color name is missing");
	const std::size_t size=strnlen(name,255);
	if (size<minimum || size>=255)
		throw std::runtime_error("original color name extent is not admitted");
	return size;
}
void house_color_idle(bool requires_physical=true)
{
	auto* current=zh::original_runtime::OriginalGpuEdge::active();
	if (!current && !requires_physical) return;
	auto& edge=zh::original_runtime::OriginalGpuEdge::required();
	edge.guard_nonstage_mutation();
	if (!edge.source_buffers_retirable())
		throw std::runtime_error("original color mutation requires idle source ownership");
}
}

struct W3DAssetManager::HouseColorAttempt {
	// Destroy last: later Q0 publication faults must restore mapper-local RNG too.
	ww3d_clone::Attempt clones;
	W3DAssetManager* owner;
	HouseColorAttempt* previous;
	HashTemplateClass<StringClass,TextureClass*> cache;
	std::vector<TextureClass*> acquired;
	std::vector<TextureClass*> rehydrated;
	bool started=false;
	bool committed=false;
	explicit HouseColorAttempt(W3DAssetManager* manager) : owner(manager),previous(house_color_attempt())
	{
		if (previous) throw std::runtime_error("original color attempt overlap is not admitted");
		house_color_attempt()=this;
	}
	~HouseColorAttempt()
	{
		house_color_attempt()=previous;
		if (!committed) for (auto i=acquired.rbegin();i!=acquired.rend();++i) (*i)->Release_Ref();
		for (auto i=rehydrated.rbegin();i!=rehydrated.rend();++i) {
			if (!committed && (*i)->Is_Initialized()) (*i)->Invalidate();
			(*i)->Release_Ref();
		}
	}
	void commit() noexcept
	{
		if (started) owner->swap_house_color_cache(cache);
		clones.commit();
		committed=true;
	}
};

W3DAssetManager::HouseColorAttempt*& W3DAssetManager::house_color_attempt()
{ static thread_local HouseColorAttempt* attempt=nullptr;return attempt; }
int& W3DAssetManager::house_color_fault_ordinal()
{ static thread_local int ordinal=-1;return ordinal; }
unsigned& W3DAssetManager::house_color_fault_count()
{ static thread_local unsigned count=0;return count; }
void W3DAssetManager::house_color_fault()
{
	++house_color_fault_count();
	int& ordinal=house_color_fault_ordinal();
	if (ordinal==0) { ordinal=-1;throw std::bad_alloc(); }
	if (ordinal>0) --ordinal;
}

void W3DAssetManager::ensure_house_color_resident(RenderObjClass* object,unsigned depth)
{
	if (!object || depth>=64)
		throw std::runtime_error("original color resident graph is not admitted");
	auto* attempt=house_color_attempt();
	if (!attempt || attempt->owner!=this)
		throw std::runtime_error("original color resident attempt is not admitted");
	if (object->Class_ID()==RenderObjClass::CLASSID_MESH) {
		HouseColorRef<MaterialInfoClass> material(static_cast<MeshClass*>(object)->Get_Material_Info());
		if (!material.value || material.value->Texture_Count()>4096)
			throw std::runtime_error("original color resident material is not admitted");
		for (int i=0;i<material.value->Texture_Count();++i) {
			auto* texture=material.value->Peek_Texture(i);
			if (texture && texture->HouseColorPixels)
				(void)zh::original_runtime::OriginalGpuEdge::required();
			if (texture && texture->HouseColorPixels && texture->Is_Initialized())
				(void)zh::original_runtime::OriginalGpuEdge::required().texture_handle(texture);
			if (texture && texture->HouseColorPixels && !texture->Is_Initialized()) {
				if (attempt->rehydrated.size()>=4096)
					throw std::runtime_error("original color resident texture bound is exceeded");
				house_color_fault();attempt->rehydrated.push_back(texture);texture->Add_Ref();
				texture->Init();
			}
		}
	} else if (object->Class_ID()==RenderObjClass::CLASSID_HLOD) {
		const int count=object->Get_Num_Sub_Objects();
		if (count<0 || count>4096)
			throw std::runtime_error("original color resident child bound is exceeded");
		for (int i=0;i<count;++i) {
			HouseColorRef<RenderObjClass> child(object->Get_Sub_Object(i));
			ensure_house_color_resident(child.value,depth+1);
		}
	}
}

void W3DAssetManager::preflight_house_color_retirement()
{
	HashTemplateIterator<StringClass,TextureClass*> entries(TextureHash);
	for (;!entries.Is_Done();entries.Next()) {
		auto* texture=entries.Peek_Value();
		if (texture && texture->HouseColorPixels) {
			if (auto* edge=zh::original_runtime::OriginalGpuEdge::active()) {
				edge->guard_nonstage_mutation();
				if (!edge->source_buffers_retirable())
					throw std::runtime_error("original color cache retirement requires idle source ownership");
			} else if (texture->Is_Initialized())
				throw std::runtime_error("original color cache has stale residency");
		}
	}
}
void W3DAssetManager::Free_Assets()
{ preflight_house_color_retirement();WW3DAssetManager::Free_Assets(); }
void W3DAssetManager::Release_Unused_Assets()
{ preflight_house_color_retirement();WW3DAssetManager::Release_Unused_Assets(); }
void W3DAssetManager::Free_Assets_With_Exclusion_List(const DynamicVectorClass<StringClass>& list)
{ preflight_house_color_retirement();WW3DAssetManager::Free_Assets_With_Exclusion_List(list); }
void W3DAssetManager::Release_All_Textures()
{ preflight_house_color_retirement();WW3DAssetManager::Release_All_Textures(); }
void W3DAssetManager::Release_Unused_Textures()
{ preflight_house_color_retirement();WW3DAssetManager::Release_Unused_Textures(); }

void W3DAssetManager::swap_house_color_cache(
	HashTemplateClass<StringClass,TextureClass*>& cache) noexcept
{
	std::swap(TextureHash.Table,cache.Table);std::swap(TextureHash.Hash,cache.Hash);
	std::swap(TextureHash.First,cache.First);std::swap(TextureHash.Size,cache.Size);
}

void W3DAssetManager::insert_house_color_strong(
	HashTemplateClass<StringClass,TextureClass*>& cache,const StringClass& key,TextureClass* texture)
{
	using Cache=HashTemplateClass<StringClass,TextureClass*>;
	using Entry=Cache::Entry;
	auto* attempt=house_color_attempt();
	if (!attempt || attempt->owner!=this || &cache!=&attempt->cache || !texture)
		throw std::runtime_error("original color cache owner is not admitted");
	const auto& source=attempt->started ? cache : TextureHash;
	const unsigned size=source.Size;
	if ((size && ((!source.Hash || !source.Table) || (size&(size-1)))) || size>65536 ||
		(size==65536 && source.First==Cache::NIL))
		throw std::runtime_error("original color cache capacity or duplicate is not admitted");
	const unsigned next_size=source.First==Cache::NIL ? std::max(4U,size*2) : size;
	if (!next_size || next_size>65536)
		throw std::runtime_error("original color cache capacity is not admitted");
	std::size_t bytes=std::size_t(next_size)*(sizeof(Entry)+sizeof(int)*2+sizeof(unsigned char));
	house_color_fault();std::vector<unsigned char> seen(size,0);
	unsigned count=0;
	for (unsigned bucket=0;bucket<size;++bucket) {
		for (int h=source.Hash[bucket];h!=Cache::NIL;h=source.Table[h].Next) {
			if (h<0 || static_cast<unsigned>(h)>=size || seen[h] || !source.Table[h].Value)
				throw std::runtime_error("original color cache graph is malformed");
			seen[h]=1;++count;
			if (source.Table[h].Key==key)
				throw std::runtime_error("original color cache duplicate is not admitted");
			bytes+=std::strlen(source.Table[h].Key)+1;
			if (bytes>zh::original_runtime::house_color_byte_limit)
				throw std::runtime_error("original color cache byte bound is exceeded");
		}
	}
	for (int h=source.First;h!=Cache::NIL;h=source.Table[h].Next) {
		if (h<0 || static_cast<unsigned>(h)>=size || seen[h])
			throw std::runtime_error("original color free-entry graph is malformed");
		seen[h]=2;++count;
	}
	if (count!=size || bytes+std::strlen(key)+1>zh::original_runtime::house_color_byte_limit)
		throw std::runtime_error("original color cache ownership is incomplete");
	house_color_fault();std::unique_ptr<Entry[]> table(new Entry[next_size]);
	house_color_fault();std::unique_ptr<int[]> buckets(new int[next_size]);
	house_color_fault();std::vector<int> tails(next_size,Cache::NIL);
	std::fill_n(buckets.get(),next_size,Cache::NIL);
	for (unsigned i=0;i<next_size;++i) { table[i].Next=Cache::NIL;table[i].Value=nullptr; }
	// Preserve the relative order of every accepted bucket, including growth.
	for (unsigned bucket=0;bucket<size;++bucket)
		for (int h=source.Hash[bucket];h!=Cache::NIL;h=source.Table[h].Next) {
			house_color_fault();table[h].Key=source.Table[h].Key;
			table[h].Value=source.Table[h].Value;
			const unsigned b=Cache::Get_Hash_Val(table[h].Key,next_size);
			if (tails[b]==Cache::NIL) buckets[b]=h;else table[tails[b]].Next=h;
			tails[b]=h;
		}
	int first=source.First;
	if (first==Cache::NIL) first=static_cast<int>(size);
	for (unsigned i=0;i<size;++i) if (seen[i]==2) table[i].Next=source.Table[i].Next;
	for (unsigned i=size;i<next_size;++i) table[i].Next=i+1==next_size ? Cache::NIL : i+1;
	const int slot=first;
	first=table[slot].Next;
	house_color_fault();table[slot].Key=key;table[slot].Value=texture;
	const unsigned bucket=Cache::Get_Hash_Val(key,next_size);
	table[slot].Next=buckets[bucket];buckets[bucket]=slot;
	// All remaining operations are nonthrowing ownership publication.
	Entry* old_table=cache.Table;int* old_hash=cache.Hash;
	cache.Table=table.release();cache.Hash=buckets.release();cache.First=first;cache.Size=next_size;
	attempt->started=true;
	delete[] old_hash;delete[] old_table;
}

void W3DAssetManager::publish_house_color_prototype(PrototypeClass* prototype)
{
	if (!prototype || !PrototypeHashTable || !Prototypes.IsValid ||
		Prototypes.ActiveCount<0 || Prototypes.ActiveCount>Prototypes.VectorMax ||
		Prototypes.VectorMax<0 || Prototypes.VectorMax>65536 ||
		Prototypes.ActiveCount==65536 || Find_Prototype(prototype->Get_Name()))
		throw std::runtime_error("original color prototype ownership is not admitted");
	for (int i=0;i<Prototypes.ActiveCount;++i)
		if (!Prototypes.Vector[i] || Prototypes.Vector[i]==prototype)
			throw std::runtime_error("original color prototype array is malformed");
	int capacity=Prototypes.VectorMax;
	if (capacity==Prototypes.ActiveCount) {
		if ((!Prototypes.IsAllocated && capacity) || Prototypes.GrowthStep<=0 ||
			Prototypes.GrowthStep>65536-capacity)
			throw std::runtime_error("original color prototype capacity is not admitted");
		capacity+=Prototypes.GrowthStep;
	}
	house_color_fault();std::unique_ptr<PrototypeClass*[]> candidate(new PrototypeClass*[capacity]{});
	for (int i=0;i<Prototypes.ActiveCount;++i) {
		house_color_fault();candidate[i]=Prototypes.Vector[i];
	}
	candidate[Prototypes.ActiveCount]=prototype;
	const unsigned hash=CRC_Stringi(prototype->Get_Name())&PROTOTYPE_HASH_MASK;
	house_color_fault();
	// No callback/allocation follows. Publish both registries and staged cache.
	auto* old=Prototypes.Vector;const bool allocated=Prototypes.IsAllocated;
	Prototypes.Vector=candidate.release();Prototypes.VectorMax=capacity;
	Prototypes.IsValid=true;Prototypes.IsAllocated=true;++Prototypes.ActiveCount;
	prototype->friend_setNextHash(PrototypeHashTable[hash]);PrototypeHashTable[hash]=prototype;
	if (auto* attempt=house_color_attempt()) attempt->commit();
	if (allocated) delete[] old;
}

RenderObjClass* W3DAssetManager::create_house_color_object(const char* name,float scale,int color,
	const char* old_texture,const char* new_texture)
{
	house_color_name_size(name);
	if (!std::isfinite(scale))
		throw std::runtime_error("original color object scale is not admitted");
	const bool geometry=WWMath::Fabs(scale-ident_scale)>scale_epsilon;
	const bool colors=(color&0xffffff)!=0;
	const bool replacement=old_texture && new_texture;
	if (!geometry && !colors && !replacement) return WW3DAssetManager::Create_Render_Obj(name);
	house_color_idle(colors || replacement);
	if (old_texture) house_color_name_size(old_texture);
	if (new_texture) house_color_name_size(new_texture);
	char lower[255];std::strcpy(lower,name);_strlwr(lower);
	char key[512];
	const int key_size=std::snprintf(key,sizeof(key),"#%d!%g!%s#%s",color,scale,
		new_texture ? new_texture : "",lower);
	if (key_size<0 || static_cast<std::size_t>(key_size)>=sizeof(key))
		throw std::runtime_error("original color object key exceeds source extent");
	struct DemandRestore {
		bool& value;bool baseline;
		~DemandRestore() { value=baseline; }
	} restore{WW3D_Load_On_Demand,WW3D_Load_On_Demand};
	WW3D_Load_On_Demand=false;
	HouseColorRef<RenderObjClass> cached(WW3DAssetManager::Create_Render_Obj(key));
	if (cached.value) {
		HouseColorAttempt attempt(this);
		ensure_house_color_resident(cached.value);
		cached.value->Set_ObjectColor(color);attempt.commit();return cached.release();
	}
	if (!Prototypes.IsValid || Prototypes.ActiveCount<0 || Prototypes.ActiveCount>=65536 ||
		Prototypes.ActiveCount>Prototypes.VectorMax || Prototypes.VectorMax>65536)
		throw std::runtime_error("original color prototype capacity is not admitted");
	// Resolve the ordinary authored source provider before the custom publication attempt.
	WW3D_Load_On_Demand=true;
	HouseColorRef<RenderObjClass> original(WW3DAssetManager::Create_Render_Obj(name));
	if (!original.value) return nullptr;
	HouseColorAttempt attempt(this);
	// These guards must unwind cloned refs before the staged texture-cache owner.
	HouseColorRef<RenderObjClass> instance(original.release());
	// A geometry-only request may reference a previously recolored prototype.
	// Admit its exact private backing before any geometry or prototype mutation.
	ensure_house_color_resident(instance.value);
	house_color_fault();
	if (geometry) instance.value->Scale(scale);
	house_color_fault();Make_Unique(instance.value,geometry,colors);
	if (replacement) {
		HouseColorRef<TextureClass> old(Get_Texture(old_texture));
		HouseColorRef<TextureClass> changed(Get_Texture(new_texture));
		if (!old.value || !changed.value)
			throw std::runtime_error("original color replacement provider is missing");
		replaceAssetTexture(instance.value,old.value,changed.value);
	}
	if (colors) Recolor_Asset(instance.value,color);
	house_color_fault();
	struct PrototypeDelete {
		void operator()(PrototypeClass* p) const noexcept { if (p) p->DeleteSelf(); }
	};
	std::unique_ptr<PrototypeClass,PrototypeDelete> prototype(
		newInstance(W3DPrototypeClass)(instance.value,key));
	house_color_fault();HouseColorRef<RenderObjClass> result(prototype->Create());
	if (!result.value) throw std::runtime_error("original color prototype clone is missing");
	result.value->Set_ObjectColor(color);
	publish_house_color_prototype(prototype.get());
	prototype.release();
	return result.release();
}
