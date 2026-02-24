#ifndef RTCW_MEMORY_INCLUDED
#define RTCW_MEMORY_INCLUDED

#include <cstddef>
#include <new>

namespace rtcw {
namespace mem {

void* allocate(int size);
void deallocate(void* pointer);

// =====================================

template<typename T>
struct NewObjectArg
{
	typedef T& Type;
};

template<typename T>
struct NewObjectArg<const T>
{
	typedef T Type;
};

template<typename T>
inline T* new_object()
{
	T* const object = static_cast<T*>(allocate(sizeof(T)));
	if (object != NULL)
	{
		new (static_cast<void*>(object)) T();
	}
	return object;
}

template<typename T, typename A1>
inline T* new_object_1(A1 a1)
{
	T* const object = static_cast<T*>(allocate(sizeof(T)));
	if (object != NULL)
	{
		typedef typename NewObjectArg<A1>::Type B1;
		new (static_cast<void*>(object)) T(static_cast<B1>(a1));
	}
	return object;
}

template<typename T, typename A1, typename A2>
inline T* new_object_2(A1 a1, A2 a2)
{
	T* const object = static_cast<T*>(allocate(sizeof(T)));
	if (object != NULL)
	{
		typedef typename NewObjectArg<A1>::Type B1;
		typedef typename NewObjectArg<A2>::Type B2;
		new (static_cast<void*>(object)) T(static_cast<B1>(a1), static_cast<B2>(a2));
	}
	return object;
}

template<typename T, typename A1, typename A2, typename A3>
inline T* new_object_3(A1 a1, A2 a2, A3 a3)
{
	T* const object = static_cast<T*>(allocate(sizeof(T)));
	if (object != NULL)
	{
		typedef typename NewObjectArg<A1>::Type B1;
		typedef typename NewObjectArg<A2>::Type B2;
		typedef typename NewObjectArg<A3>::Type B3;
		new (static_cast<void*>(object)) T(static_cast<B1>(a1), static_cast<B2>(a2), static_cast<B3>(a3));
	}
	return object;
}

// =====================================

template<typename T>
inline void delete_object_unchecked(T* object)
{
	object->~T();
	deallocate(object);
}

template<typename T>
inline void delete_object(T* object)
{
	if (object != NULL)
	{
		delete_object_unchecked(object);
	}
}

template<typename T>
inline void destroy_object(T* object)
{
	if (object != NULL)
	{
		object->destroy();
	}
}

} // namespace mem
} // namespace rtcw

#endif // RTCW_MEMORY_INCLUDED
