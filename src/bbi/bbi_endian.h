//
// Custom library.
// Copyright (C) 2012 Boris I. Bendovsky
//
// Byte order (endianness) manipulation.
//


#ifndef BBI_ENDIAN_H
#define BBI_ENDIAN_H


#include "bbi_core.h"


namespace bbi {


class Endian {
public:
    // Returns "true" if system's endianness is big-endian.
    static bool is_big ()
    {
#if BBI_ENDIANNESS == BBI_LE
        return false;
#elif BBI_ENDIANNESS == BBI_BE
        return true;
#endif
    }

    // Returns "true" if system's endianness is little-endian.
    static bool is_little ()
    {
#if BBI_ENDIANNESS == BBI_LE
        return true;
#elif BBI_ENDIANNESS == BBI_BE
        return false;
#endif
    }

    // Swaps bytes on non little-endian system.
    template<class T>
    static T le (const T value)
    {
#if BBI_ENDIANNESS == BBI_LE
        return value;
#elif BBI_ENDIANNESS == BBI_BE
        return le_be (value);
#endif
    }

    // Copies an array of elements with swapped bytes into another array
    // on non little-endian system.
    template<class T, int N>
    static void le (const T (&src_data) [N], T (&dst_data) [N])
    {
        for (int i = 0; i < N; ++i)
            dst_data[i] = le (src_data[i]);
    }

    // Copies an array of elements with swapped bytes into another array
    // on non little-endian system.
    template<class T>
    static void le (const T* src_data, int count, T* dst_data)
    {
        for (int i = 0; i < count; ++i)
            dst_data[i] = le (src_data[i]);
    }

    // Swaps bytes on non big-endian system.
    template<class T>
    static T be (const T value)
    {
#if BBI_ENDIANNESS == BBI_BE
        return value;
#elif BBI_ENDIANNESS == BBI_LE
        return le_be (value);
#endif
    }

    // Copies an array of elements with swapped bytes into another array
    // on non big-endian system.
    template<class T, int N>
    static void be (const T (&src_data) [N], T (&dst_data) [N])
    {
        for (int i = 0; i < N; ++i)
            dst_data[i] = be (src_data[i]);
    }

    // Copies an array of elements with swapped bytes into another array
    // on non big-endian system.
    template<class T>
    static void be (const T* src_data, int count, T* dst_data)
    {
        for (int i = 0; i < count; ++i)
            dst_data[i] = be (src_data[i]);
    }

    // Swaps bytes in place on non little-endian system.
    template<class T>
    static void lei (T& value)
    {
#if BBI_ENDIANNESS == BBI_BE
        lei_bei (value);
#endif
    }

    // Swaps bytes in place of an array of elements
    // on non little-endian system.
    template<class T, int N>
    static void lei (T (&data) [N])
    {
        for (int i = 0; i < N; ++i)
            lei (data[i]);
    }

    // Swaps bytes in place of an array of elements
    // on non little-endian system.
    template<class T>
    static void lei (T* data, int count)
    {
        for (int i = 0; i < count; ++i)
            lei (data[i]);
    }

    // Swaps bytes in place on non big-endian system.
    template<class T>
    static void bei (T& value)
    {
#if BBI_ENDIANNESS == BBI_LE
        lei_bei (value);
#endif
    }

    // Swaps bytes in place of an array of elements
    // on non big-endian system.
    template<class T, int N>
    static void bei (T (&data) [N])
    {
        for (int i = 0; i < N; ++i)
            bei (data[i]);
    }

    // Swaps bytes in place of an array of elements
    // on non big-endian system.
    template<class T>
    static void bei (T* data, int count)
    {
        for (int i = 0; i < count; ++i)
            bei (data[i]);
    }


private:
    Endian ();

    Endian (const Endian& that);

    ~Endian ();

    Endian& operator = (const Endian& that);


    // Swaps bytes for static methods.
    template<class T>
    static T le_be (const T value)
    {
        T result;

        for (int i = 0, j = sizeof (T) - 1; i < sizeof (T); ++i, --j) {
            (reinterpret_cast<bbi::UInt8*> (&result)) [i] =
                (reinterpret_cast<const bbi::UInt8*> (&value)) [j];
        }

        return result;
    }

    // Swaps bytes in place for static methods.
    template<class T>
    static void lei_bei (T& value)
    {
        for (int i = 0, j = sizeof (T) - 1, n = sizeof (T) / 2; i < n; ++i, --j) {
            std::swap ((reinterpret_cast<bbi::UInt8*> (&value)) [i],
                (reinterpret_cast<bbi::UInt8*> (&value)) [j]);
        }
    }
}; // enum Endian


} // namespace bbi


#endif // BBI_ENDIAN_H
