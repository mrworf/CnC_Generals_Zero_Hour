#include "PreRTS.h"

#include <new>

extern "C" void* m26_allocate_across_target(std::size_t size)
{
  return ::operator new(size);
}

extern "C" void m26_free_across_target(void* memory)
{
  ::operator delete(memory);
}

extern "C" void* m26_allocate_aligned(std::size_t size, std::size_t alignment)
{
  return ::operator new(size, static_cast<std::align_val_t>(alignment));
}

extern "C" void m26_free_aligned(void* memory, std::size_t alignment)
{
  ::operator delete(memory, static_cast<std::align_val_t>(alignment));
}

extern "C" void* m22_allocate_nothrow_across_target(std::size_t size, bool array)
{
  return array ? ::operator new[](size, std::nothrow) : ::operator new(size, std::nothrow);
}

extern "C" void m22_free_nothrow_across_target(void* memory, bool array, unsigned route)
{
  if (array) {
    if (route == 1) ::operator delete[](memory, std::size_t{4097});
    else if (route == 2) ::operator delete[](memory, std::nothrow);
    else ::operator delete[](memory);
  } else {
    if (route == 1) ::operator delete(memory, std::size_t{4097});
    else if (route == 2) ::operator delete(memory, std::nothrow);
    else ::operator delete(memory);
  }
}
