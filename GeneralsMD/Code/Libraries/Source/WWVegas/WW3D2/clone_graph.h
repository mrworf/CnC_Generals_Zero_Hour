// Linux-only construction scope for the original cloned mesh/material graph.
// No existing class layout, serialization or virtual interface is changed.
#ifndef WW3D_CLONE_GRAPH_H
#define WW3D_CLONE_GRAPH_H
#if defined(__linux__)
#include <random.h>
#include <cstddef>
#include <new>
#include <stdexcept>
#include <memory>

struct W3DCloneGraphProbeAccess;
namespace ww3d_clone {
Random4Class capture_mapper_random() noexcept;
void restore_mapper_random(const Random4Class&) noexcept;

class Attempt {
	friend struct ::W3DCloneGraphProbeAccess;
	struct Budget { std::size_t bytes=0; };
	static inline thread_local Budget* current=nullptr;
	static inline thread_local int fault_ordinal=-1;
	static inline thread_local unsigned fault_count=0;
	Budget local;
	Budget* previous;
	Random4Class random;
	bool accepted=false;
public:
	Attempt() noexcept : previous(current),random(capture_mapper_random())
	{ if (!current) current=&local; }
	Attempt(const Attempt&)=delete;
	Attempt& operator=(const Attempt&)=delete;
	~Attempt() noexcept
	{
		if (!accepted) restore_mapper_random(random);
		current=previous;
	}
	void commit() noexcept { accepted=true; }
	static bool active() noexcept { return current!=nullptr; }
	static void fault()
	{
		if (!current) return;
		++fault_count;
		if (fault_ordinal==0) { fault_ordinal=-1;throw std::bad_alloc(); }
		if (fault_ordinal>0) --fault_ordinal;
	}
	static std::size_t extent(int count,std::size_t element)
	{
		constexpr std::size_t limit=64u*1024u*1024u;
		if (count<0 || element==0 || static_cast<std::size_t>(count)>limit/element)
			throw std::runtime_error("original clone extent is not admitted");
		return static_cast<std::size_t>(count)*element;
	}
	static void reserve(int count,std::size_t element)
	{
		constexpr std::size_t limit=64u*1024u*1024u;
		const std::size_t bytes=extent(count,element);
		if (current && bytes>limit-current->bytes)
			throw std::runtime_error("original clone byte budget is exhausted");
		if (current) current->bytes+=bytes;
	}
};

template<class T> class Ref {
	T* value;
public:
	explicit Ref(T* p=nullptr) noexcept : value(p) {}
	~Ref() noexcept { if (value) value->Release_Ref(); }
	Ref(const Ref&)=delete;
	Ref& operator=(const Ref&)=delete;
	T* get() const noexcept { return value; }
	T* release() noexcept { T* p=value;value=nullptr;return p; }
};

template<class T> class RefArray {
	std::unique_ptr<T*[]> values;
	int count=0;
public:
	explicit RefArray(int capacity)
	{
		Attempt::reserve(capacity,sizeof(T*));
		if (capacity) { Attempt::fault();values.reset(new T*[capacity]()); }
	}
	~RefArray() noexcept { for (int i=count;i>0;--i) values[i-1]->Release_Ref(); }
	void append(T* p) noexcept { values[count++]=p; }
	T** release() noexcept { count=0;return values.release(); }
};
}
#endif
#endif
