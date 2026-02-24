#include "rtcw_memory.h"
#include <cstdlib>

namespace rtcw {
namespace mem {

void* allocate(int size)
{
	return std::malloc(size);
}

void deallocate(void* pointer)
{
	std::free(pointer);
}

} // namespace mem
} // namespace rtcw
