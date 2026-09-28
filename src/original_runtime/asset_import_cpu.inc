// Included by the native asset manager. Candidate registries are never installed
// into an accepted manager until the complete root import has succeeded.
#include "asset_import.h"
#include "original_gpu_edge.h"
#include "clone_graph.h"
#include "ini.h"
#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace ww3d_import {
void PrototypeDelete::operator()(PrototypeClass* p) const noexcept { if (p) p->DeleteSelf(); }
thread_local Attempt* Attempt::current=nullptr;
thread_local int Attempt::fault_ordinal=-1;
thread_local unsigned Attempt::fault_count=0;

struct Attempt::Impl {
	static constexpr std::size_t limit=64u*1024u*1024u;
	WW3DAssetManager* owner;
	FileFactoryClass* factory;
	zh::original_runtime::OriginalGpuEdge* edge;
	std::uint64_t generation;
	const void* file_system=nullptr;
	const void* (*peek_file_system)() noexcept=nullptr;
	ww3d_clone::Attempt clones;
	std::size_t bytes=0;
	unsigned nodes=0;
	bool committed=false;
	DynamicVectorClass<PrototypeClass*> prototypes;
	std::unique_ptr<PrototypeClass*[]> prototype_hash;
	HashTemplateClass<StringClass,TextureClass*> textures;
	HashTemplateClass<StringClass,HTreeClass*> trees;
	std::unique_ptr<HTreeClass*[]> tree_ptr;
	int tree_size=0;
	std::unique_ptr<HashTableClass> anim_table,missing_table;
	std::vector<PrototypeClass*> new_prototypes;
	std::vector<HTreeClass*> new_trees;
	std::vector<HAnimClass*> new_animations;
	std::vector<HashableClass*> new_missing;
	std::vector<TextureClass*> new_textures;
	struct Filter {
		TextureClass* texture;
		TextureFilterClass::FilterType min,mag,mip;
		TextureFilterClass::TxtAddrMode u,v;
	};
	std::vector<Filter> filters;
	std::vector<std::string> files;

	void take(std::size_t count,std::size_t element)
	{
		if (!element || count>limit/element || count*element>limit-bytes)
			throw std::runtime_error("original import byte extent is not admitted");
		bytes+=count*element;
	}
	void node()
	{
		if (nodes==65536) throw std::runtime_error("original import node capacity is exhausted");
		++nodes;fault();
	}
	template<class T> static void swap_cache(HashTemplateClass<StringClass,T*>& a,
		HashTemplateClass<StringClass,T*>& b) noexcept
	{
		std::swap(a.Hash,b.Hash);std::swap(a.Table,b.Table);
		std::swap(a.First,b.First);std::swap(a.Size,b.Size);
	}
	template<class T> void copy_cache(const HashTemplateClass<StringClass,T*>& source,
		HashTemplateClass<StringClass,T*>& destination,unsigned size)
	{
		using Cache=HashTemplateClass<StringClass,T*>;
		if (!source.Size) {
			if (source.Hash || source.Table || source.First!=Cache::NIL || size>65536 || (size && (size&(size-1))))
				throw std::runtime_error("original import empty cache ownership is malformed");
			if (!size) return;
			take(size,sizeof(typename Cache::Entry)+sizeof(int));
			fault();std::unique_ptr<typename Cache::Entry[]> table(new typename Cache::Entry[size]);
			fault();std::unique_ptr<int[]> buckets(new int[size]);
			std::fill_n(buckets.get(),size,Cache::NIL);
			for (unsigned i=0;i<size;++i) { table[i].Next=i+1==size ? Cache::NIL : int(i+1);table[i].Value=nullptr; }
			delete[] destination.Hash;delete[] destination.Table;
			destination.Hash=buckets.release();destination.Table=table.release();destination.Size=size;destination.First=0;
			return;
		}
		if (source.Size>65536 || (source.Size&(source.Size-1)) ||
			!source.Hash || !source.Table || size<source.Size || size>65536)
			throw std::runtime_error("original import cache ownership is malformed");
		take(size,sizeof(typename Cache::Entry)+sizeof(int)+sizeof(unsigned char));
		fault();std::vector<unsigned char> seen(source.Size,0);
		fault();std::unique_ptr<typename Cache::Entry[]> table(new typename Cache::Entry[size]);
		fault();std::unique_ptr<int[]> buckets(new int[size]);
		fault();std::vector<int> tails(size,Cache::NIL);
		std::fill_n(buckets.get(),size,Cache::NIL);
		for (unsigned i=0;i<size;++i) { table[i].Next=Cache::NIL;table[i].Value=nullptr; }
		unsigned count=0;
		for (unsigned b=0;b<source.Size;++b)
			for (int h=source.Hash[b];h!=Cache::NIL;h=source.Table[h].Next) {
				if (h<0 || unsigned(h)>=source.Size || seen[h] || !source.Table[h].Value)
					throw std::runtime_error("original import cache chain is malformed");
				seen[h]=1;++count;
				const char* key=source.Table[h].Key;
				const std::size_t n=strnlen(key,512);
				if (!n || n==512 || Cache::Get_Hash_Val(source.Table[h].Key,source.Size)!=b)
					throw std::runtime_error("original import cache key is malformed");
				take(n+1,1);fault();table[h].Key=source.Table[h].Key;
				table[h].Value=source.Table[h].Value;
				const unsigned next=Cache::Get_Hash_Val(table[h].Key,size);
				if (tails[next]==Cache::NIL) buckets[next]=h;else table[tails[next]].Next=h;
				tails[next]=h;
			}
		for (int h=source.First;h!=Cache::NIL;h=source.Table[h].Next) {
			if (h<0 || unsigned(h)>=source.Size || seen[h])
				throw std::runtime_error("original import free cache chain is malformed");
			seen[h]=2;++count;table[h].Next=source.Table[h].Next;
		}
		if (count!=source.Size) throw std::runtime_error("original import cache ownership is incomplete");
		int first=source.First;
		if (size>source.Size) {
			if (first!=Cache::NIL) throw std::runtime_error("original import unexpected cache growth");
			first=static_cast<int>(source.Size);
			for (unsigned i=source.Size;i<size;++i) table[i].Next=i+1==size ? Cache::NIL : int(i+1);
		}
		delete[] destination.Hash;delete[] destination.Table;
		destination.Hash=buckets.release();destination.Table=table.release();
		destination.Size=size;destination.First=first;
	}
	template<class T> void insert(HashTemplateClass<StringClass,T*>& cache,const StringClass& key,T* value)
	{
		using Cache=HashTemplateClass<StringClass,T*>;
		if (!value || cache.Exists(key)) throw std::runtime_error("original import duplicate cache admission");
		unsigned size=cache.Size ? cache.Size : 4;
		if (cache.Size && cache.First==Cache::NIL) {
			if (size==65536) throw std::runtime_error("original import cache capacity is exhausted");
			size*=2;
		}
		HashTemplateClass<StringClass,T*> candidate;
		copy_cache(cache,candidate,size);
		const int slot=candidate.First;
		fault();candidate.Table[slot].Key=key;
		candidate.First=candidate.Table[slot].Next;
		candidate.Table[slot].Value=value;
		const unsigned bucket=Cache::Get_Hash_Val(key,size);
		candidate.Table[slot].Next=candidate.Hash[bucket];candidate.Hash[bucket]=slot;
		swap_cache(cache,candidate);
	}
	std::unique_ptr<HashTableClass> copy_hash(HashTableClass* source)
	{
		if (!source || source->HashTableSize<=0 || source->HashTableSize>65536 ||
			(source->HashTableSize&(source->HashTableSize-1)) || !source->HashTable)
			throw std::runtime_error("original import animation table is malformed");
		take(source->HashTableSize,sizeof(HashableClass*));fault();
		std::unique_ptr<HashTableClass> result(new HashTableClass(source->HashTableSize));
		std::copy_n(source->HashTable,source->HashTableSize,result->HashTable);
		return result;
	}
	explicit Impl(WW3DAssetManager& manager) : owner(&manager),factory(_TheFileFactory),
		edge(zh::original_runtime::OriginalGpuEdge::active()),generation(edge ? edge->generation() : 0)
	{
		if (&manager!=WW3DAssetManager::Get_Instance() || !factory)
			throw std::runtime_error("original import provider ownership is not admitted");
		if (edge) {
			edge->guard_nonstage_mutation();
			if (!edge->source_buffers_retirable() || edge->tree_source_frame_pending())
				throw std::runtime_error("original import requires idle source ownership");
		}
		const auto& source=manager.Prototypes;
		if (!source.IsValid || source.ActiveCount<0 || source.VectorMax<source.ActiveCount ||
			source.VectorMax>65536 || (source.VectorMax && !source.Vector) || !manager.PrototypeHashTable)
			throw std::runtime_error("original import prototype ownership is malformed");
		take(source.VectorMax,sizeof(PrototypeClass*));fault();
		std::unique_ptr<PrototypeClass*[]> array(new PrototypeClass*[source.VectorMax]());
		for (int i=0;i<source.ActiveCount;++i) {
			if (!source.Vector[i]) throw std::runtime_error("original import prototype identity is missing");
			fault();array[i]=source.Vector[i];
		}
		prototypes.Vector=array.release();prototypes.VectorMax=source.VectorMax;
		prototypes.ActiveCount=source.ActiveCount;prototypes.GrowthStep=source.GrowthStep;
		prototypes.IsValid=true;prototypes.IsAllocated=true;
		take(manager.PROTOTYPE_HASH_TABLE_SIZE,sizeof(PrototypeClass*));fault();
		prototype_hash.reset(new PrototypeClass*[manager.PROTOTYPE_HASH_TABLE_SIZE]);
		std::copy_n(manager.PrototypeHashTable,manager.PROTOTYPE_HASH_TABLE_SIZE,prototype_hash.get());
		// Exact raw-pointer membership is established without dereferencing an
		// intrusive head. A bounded offside index also keeps full-capacity
		// admission linear instead of repeatedly searching the source vector.
		unsigned membership_size=1;
		while (membership_size<unsigned(source.ActiveCount)*2u) membership_size*=2;
		take(membership_size,sizeof(PrototypeClass*)+sizeof(int));fault();
		std::vector<PrototypeClass*> members(membership_size,nullptr);fault();
		std::vector<int> indices(membership_size,-1);
		const auto membership_slot=[membership_size](PrototypeClass* p) {
			return unsigned((reinterpret_cast<std::uintptr_t>(p)>>4)*std::uintptr_t(0x9e3779b9u))&(membership_size-1);
		};
		for (int index=0;index<source.ActiveCount;++index) {
			unsigned slot=membership_slot(source.Vector[index]);
			while (members[slot]) {
				if (members[slot]==source.Vector[index]) throw std::runtime_error("original import duplicate prototype identity");
				slot=(slot+1)&(membership_size-1);
			}
			members[slot]=source.Vector[index];indices[slot]=index;
		}
		take(source.ActiveCount,sizeof(unsigned char));fault();
		std::vector<unsigned char> linked(source.ActiveCount,0);
		unsigned linked_count=0;
		for (int bucket=0;bucket<manager.PROTOTYPE_HASH_TABLE_SIZE;++bucket) {
			for (auto* p=prototype_hash[bucket];p;) {
				unsigned slot=membership_slot(p);
				while (members[slot] && members[slot]!=p) slot=(slot+1)&(membership_size-1);
				const int index=indices[slot];
				// Compare membership before following an intrusive pointer.
				if (index<0 || linked[index])
					throw std::runtime_error("original import prototype registry identity is malformed");
				linked[index]=1;++linked_count;
				const char* name=p->Get_Name();Attempt::name(name);
				if ((CRC_Stringi(name)&manager.PROTOTYPE_HASH_MASK)!=unsigned(bucket))
					throw std::runtime_error("original import prototype registry key is malformed");
				p=p->friend_getNextHash();
			}
		}
		if (linked_count!=unsigned(source.ActiveCount))
			throw std::runtime_error("original import prototype registry membership is incomplete");
		copy_cache(manager.TextureHash,textures,manager.TextureHash.Size);
		auto& tree_owner=manager.HTreeManager;
		if (tree_owner.NumTrees<0 || tree_owner.NumTrees>tree_owner.MAX_TREES)
			throw std::runtime_error("original import hierarchy count is malformed");
		take(tree_owner.MAX_TREES,sizeof(HTreeClass*));fault();
		tree_ptr.reset(new HTreeClass*[tree_owner.MAX_TREES]());tree_size=tree_owner.NumTrees;
		std::copy_n(tree_owner.TreePtr,tree_owner.MAX_TREES,tree_ptr.get());
		copy_cache(tree_owner.TreeHash,trees,tree_owner.TreeHash.Size);
		anim_table=copy_hash(manager.HAnimManager.AnimPtrTable);
		missing_table=copy_hash(manager.HAnimManager.MissingAnimTable);
	}
	~Impl() noexcept
	{
		if (committed) return;
		for (auto i=filters.rbegin();i!=filters.rend();++i) {
			auto& f=i->texture->Get_Filter();f.TextureMinFilter=i->min;f.TextureMagFilter=i->mag;
			f.MipMapFilter=i->mip;f.UAddressMode=i->u;f.VAddressMode=i->v;
		}
		for (auto i=new_prototypes.rbegin();i!=new_prototypes.rend();++i) (*i)->DeleteSelf();
		for (auto i=new_animations.rbegin();i!=new_animations.rend();++i) (*i)->Release_Ref();
		for (auto i=new_missing.rbegin();i!=new_missing.rend();++i) delete *i;
		for (auto i=new_trees.rbegin();i!=new_trees.rend();++i) delete *i;
		for (auto i=new_textures.rbegin();i!=new_textures.rend();++i) (*i)->Release_Ref();
	}
};

void Attempt::fault()
{
	++fault_count;
	if (fault_ordinal==0) { fault_ordinal=-1;throw std::bad_alloc(); }
	if (fault_ordinal>0) --fault_ordinal;
}
Attempt::Attempt(WW3DAssetManager& owner)
{
	if (current || INIClass::SourceImportGuard) throw std::runtime_error("original import overlap is not admitted");
	fault();impl.reset(new Impl(owner));current=this;INIClass::SourceImportGuard=ini_boundary;
}
Attempt::~Attempt() noexcept { if (current==this) { INIClass::SourceImportGuard=nullptr;current=nullptr; } }
Attempt* Attempt::active(const WW3DAssetManager* owner) noexcept
{ return current && current->impl->owner==owner ? current : nullptr; }
Attempt* Attempt::active(const HTreeManagerClass* owner) noexcept
{ return current && &current->impl->owner->HTreeManager==owner ? current : nullptr; }
Attempt* Attempt::active(const HAnimManagerClass* owner) noexcept
{ return current && &current->impl->owner->HAnimManager==owner ? current : nullptr; }
void Attempt::validate() const
{
	if (current!=this || WW3DAssetManager::Get_Instance()!=impl->owner || _TheFileFactory!=impl->factory ||
		(impl->peek_file_system && impl->peek_file_system()!=impl->file_system) ||
		zh::original_runtime::OriginalGpuEdge::active()!=impl->edge ||
		(impl->edge && (impl->edge->generation()!=impl->generation ||
			!impl->edge->source_buffers_retirable() || impl->edge->tree_source_frame_pending())))
		throw std::runtime_error("original import provider changed during attempt");
}
void Attempt::name(const char* value,std::size_t suffix)
{
	if (!value || suffix>254) throw std::runtime_error("original import name is missing");
	const std::size_t n=strnlen(value,255);
	if (!n || n>=255 || n>254-suffix)
		throw std::runtime_error("original import name exceeds source extent");
}
void Attempt::reserve(std::size_t count,std::size_t element)
{ if (current) current->impl->take(count,element); }
void Attempt::boundary() { if (current) { current->validate();fault(); } }
void Attempt::ini_boundary(std::size_t count,std::size_t size)
{
	if (count>65536) throw std::runtime_error("original import mapper argument extent is not admitted");
	reserve(count,size);boundary();
}
void Attempt::file_provider(const void* provider,const void* (*peek)() noexcept)
{
	if (!current) return;
	current->validate();
	if (!provider || !peek || peek()!=provider || (current->impl->peek_file_system &&
		(current->impl->peek_file_system!=peek || current->impl->file_system!=provider)))
		throw std::runtime_error("original import file-system ownership is not admitted");
	current->impl->file_system=provider;current->impl->peek_file_system=peek;
}
bool Attempt::preflight(FileClass& file)
{
	if (!current) return true;
	current->validate();
	const int first=file.Seek(0,SEEK_CUR),size=file.Size();
	if (first<0 || size<first || std::size_t(size-first)>Impl::limit) return false;
	// The native chunk reader uses a fixed stack. Admit the complete envelope
	// before any parser can confuse a truncated header with ordinary EOF.
	int ends[64];unsigned depth=0,count=0;ends[0]=size;int position=first;
	while (position<size) {
		while (position==ends[depth] && depth) --depth;
		if (position>ends[depth] || ends[depth]-position<int(sizeof(ChunkHeader)) || ++count>65536) return false;
		ChunkHeader header;
		boundary();
		if (file.Read(&header,sizeof(header))!=sizeof(header)) return false;
		position+=sizeof(header);
		const unsigned bytes=header.Get_Size();
		if (bytes>unsigned(ends[depth]-position)) return false;
		const int end=position+int(bytes);
		if (header.Get_Sub_Chunk_Flag() && bytes) {
			if (depth==63) return false;
			ends[++depth]=end;
		} else {
			if (file.Seek(end,SEEK_SET)!=end) return false;
			position=end;
		}
	}
	return file.Seek(first,SEEK_SET)==first;
}
bool Attempt::load(WW3DAssetManager& owner,const char* filename)
{
	name(filename);
	if (current) {
		if (current->impl->owner!=&owner) throw std::runtime_error("original import foreign owner is not admitted");
		current->validate();return owner.Load_3D_Assets(filename);
	}
	Attempt attempt(owner);
	const bool result=owner.Load_3D_Assets(filename);
	if (result) {
		attempt.validate();boundary();
		if (!attempt.commit()) throw std::runtime_error("original import publication ownership changed");
	}
	return result;
}
bool Attempt::commit() noexcept
{
	if (impl->committed || current!=this || WW3DAssetManager::Get_Instance()!=impl->owner ||
		(impl->peek_file_system && impl->peek_file_system()!=impl->file_system) ||
		_TheFileFactory!=impl->factory || zh::original_runtime::OriginalGpuEdge::active()!=impl->edge ||
		(impl->edge && (impl->edge->generation()!=impl->generation ||
			!impl->edge->source_buffers_retirable() || impl->edge->tree_source_frame_pending()))) return false;
	auto& a=*impl->owner;auto& p=a.Prototypes;auto& c=impl->prototypes;
	if (!impl->new_prototypes.empty()) {
		std::swap(p.Vector,c.Vector);std::swap(p.VectorMax,c.VectorMax);
		std::swap(p.ActiveCount,c.ActiveCount);std::swap(p.GrowthStep,c.GrowthStep);
		std::swap(p.IsValid,c.IsValid);std::swap(p.IsAllocated,c.IsAllocated);
		auto* old_hash=a.PrototypeHashTable;a.PrototypeHashTable=impl->prototype_hash.release();
		impl->prototype_hash.reset(old_hash);
	}
	if (!impl->new_textures.empty()) Impl::swap_cache(a.TextureHash,impl->textures);
	if (!impl->new_trees.empty()) {
		Impl::swap_cache(a.HTreeManager.TreeHash,impl->trees);
		std::copy_n(impl->tree_ptr.get(),a.HTreeManager.MAX_TREES,a.HTreeManager.TreePtr);
		a.HTreeManager.NumTrees=impl->tree_size;
	}
	if (!impl->new_animations.empty()) {
		auto* old_anims=a.HAnimManager.AnimPtrTable;
		a.HAnimManager.AnimPtrTable=impl->anim_table.release();impl->anim_table.reset(old_anims);
	}
	if (!impl->new_missing.empty()) {
		auto* old_missing=a.HAnimManager.MissingAnimTable;
		a.HAnimManager.MissingAnimTable=impl->missing_table.release();impl->missing_table.reset(old_missing);
	}
	impl->clones.commit();impl->committed=true;return true;
}
PrototypeClass* Attempt::find_prototype(const char* name) const
{
	validate();Attempt::name(name);
	const unsigned hash=CRC_Stringi(name)&impl->owner->PROTOTYPE_HASH_MASK;
	unsigned count=0;
	for (auto* p=impl->prototype_hash[hash];p;p=p->friend_getNextHash()) {
		if (++count>65536) throw std::runtime_error("original import prototype chain is malformed");
		if (!stricmp(p->Get_Name(),name)) return p;
	}
	return nullptr;
}
void Attempt::add_prototype(PrototypeClass* prototype)
{
	validate();
	if (!prototype || find_prototype(prototype->Get_Name()))
		throw std::runtime_error("original import prototype duplicate is not admitted");
	auto& p=impl->prototypes;
	if (p.ActiveCount==65536) throw std::runtime_error("original import prototype capacity is exhausted");
	int capacity=p.VectorMax;
	if (p.ActiveCount==capacity) {
		if (p.GrowthStep<=0 || p.GrowthStep>65536-capacity)
			throw std::runtime_error("original import prototype growth is not admitted");
		capacity+=p.GrowthStep;
	}
	impl->take(capacity,sizeof(PrototypeClass*));fault();
	std::unique_ptr<PrototypeClass*[]> array(new PrototypeClass*[capacity]());
	for (int i=0;i<p.ActiveCount;++i) { fault();array[i]=p.Vector[i]; }
	impl->node();impl->take(1,sizeof(PrototypeClass*));impl->new_prototypes.push_back(prototype);
	array[p.ActiveCount]=prototype;
	auto* old=p.Vector;p.Vector=array.release();p.VectorMax=capacity;++p.ActiveCount;delete[] old;
	const unsigned hash=CRC_Stringi(prototype->Get_Name())&impl->owner->PROTOTYPE_HASH_MASK;
	prototype->friend_setNextHash(impl->prototype_hash[hash]);impl->prototype_hash[hash]=prototype;
}
TextureClass* Attempt::find_texture(const StringClass& key) const
{ validate();return impl->textures.Get(key); }
void Attempt::add_texture(const StringClass& key,TextureClass* texture)
{
	validate();impl->node();impl->take(1,sizeof(TextureClass*));
	impl->new_textures.push_back(texture);
	try { impl->insert(impl->textures,key,texture); }
	catch (...) { impl->new_textures.pop_back();throw; }
}
void Attempt::remember_filter(TextureClass* texture)
{
	validate();
	if (!texture) throw std::runtime_error("original import texture identity is missing");
	if (std::find(impl->new_textures.begin(),impl->new_textures.end(),texture)!=impl->new_textures.end()) return;
	for (const auto& f:impl->filters) if (f.texture==texture) return;
	impl->take(1,sizeof(Impl::Filter));fault();const auto& f=texture->Get_Filter();
	impl->filters.push_back({texture,f.TextureMinFilter,f.TextureMagFilter,f.MipMapFilter,f.UAddressMode,f.VAddressMode});
}
int Attempt::tree_count() const noexcept { return impl->tree_size; }
int Attempt::tree_id(const char* name) const
{
	validate();Attempt::name(name);
	for (int i=0;i<impl->tree_size;++i)
		if (impl->tree_ptr[i] && !stricmp(name,impl->tree_ptr[i]->Get_Name())) return i;
	return -1;
}
HTreeClass* Attempt::tree(int i) const noexcept
{ return i>=0 && i<impl->tree_size ? impl->tree_ptr[i] : nullptr; }
HTreeClass* Attempt::tree(const StringClass& key) const
{ validate();return impl->trees.Get(key); }
void Attempt::add_tree(HTreeClass* tree)
{
	validate();
	if (!tree || impl->tree_size==impl->owner->HTreeManager.MAX_TREES || tree_id(tree->Get_Name())!=-1)
		throw std::runtime_error("original import hierarchy admission is not valid");
	StringClass key(tree->Get_Name(),true);_strlwr(key.Peek_Buffer());
	impl->node();impl->take(1,sizeof(HTreeClass*));impl->new_trees.push_back(tree);
	try { impl->insert(impl->trees,key,tree); }
	catch (...) { impl->new_trees.pop_back();throw; }
	impl->tree_ptr[impl->tree_size++]=tree;
}
HashTableClass& Attempt::animations() noexcept { return *impl->anim_table; }
HashTableClass& Attempt::missing() noexcept { return *impl->missing_table; }
void Attempt::add_animation(HAnimClass* animation)
{
	validate();
	if (!animation || animations().Find(animation->Get_Name()))
		throw std::runtime_error("original import animation duplicate is not admitted");
	impl->node();impl->take(1,sizeof(HAnimClass*));impl->new_animations.push_back(animation);
	// Native intrusive Add has no allocation; only the new node's link is changed.
	animation->Add_Ref();animations().Add(animation);
}
void Attempt::add_missing(HashableClass* value)
{
	validate();
	if (!value || missing().Find(value->Get_Key()))
		throw std::runtime_error("original import missing-animation duplicate is not admitted");
	impl->node();impl->take(1,sizeof(HashableClass*));impl->new_missing.push_back(value);
	missing().Add(value);
}
Attempt::FileScope::FileScope(const char* name)
{
	if (!current) return;
	owner=current;owner->validate();Attempt::name(name);
	if (owner->impl->files.size()==64) throw std::runtime_error("original import recursion depth is exhausted");
	std::string key(name);std::transform(key.begin(),key.end(),key.begin(),[](unsigned char c) {
		return c>='A' && c<='Z' ? char(c-'A'+'a') : char(c);
	});
	for (const auto& other:owner->impl->files) if (key==other)
		throw std::runtime_error("original import recursion cycle is not admitted");
	owner->impl->take(key.size()+1,1);fault();owner->impl->files.push_back(std::move(key));
}
Attempt::FileScope::~FileScope() noexcept { if (owner) owner->impl->files.pop_back(); }
}
