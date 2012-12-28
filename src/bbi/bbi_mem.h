//
// Custom library.
// Copyright (C) 2012 Boris I. Bendovsky
//
// Usefull wrapper for standard memory routines.
//


#ifndef BBI_MEM_H
#define BBI_MEM_H


#include <cstring>

#include "bbi_core.h"


namespace bbi {


class Mem {
public:
    // Copies one element of "S" from the "source" into the "target".
    template<class S, class D>
    static D* copy (const S* source, D* target)
    {
        return static_cast <D*> (
            std::memcpy (target, source, sizeof (S)));
    }

    // Copies "count" elements of "S" from the "source" into the "target".
    template<class S, class D>
    static D* copy (const S* source, std::size_t count, D* target)
    {
        return static_cast <D*> (
            std::memcpy (target, source, count * sizeof (S)));
    }

    // Copies "count" bytes from the "source" into the "target".
    template<class D>
    static D* copy (const void* source, std::size_t count, D* target)
    {
        return static_cast <D*> (std::memcpy (target, source, count));
    }

    // Copies one element of "S" from the "source" into the "target".
    template<class S, class D>
    static D* move (const S* source, D* target)
    {
        return static_cast <D*> (
            std::memmove (target, source, sizeof (S)));
    }

    // Copies "count" elements of "S" from the "source" into the "target".
    template<class S, class D>
    static D* move (const S* source, std::size_t count, D* target)
    {
        return static_cast <D*> (
            std::memmove (target, source, count * sizeof (S)));
    }

    // Copies "count" bytes from the "source" into the "target".
    template<class D>
    static D* move (const void* source, std::size_t count, D* target)
    {
        return static_cast <D*> (std::memmove (target, source, count));
    }


    // Zeroizes one element of "D" pointed by the "target".
    template<class D>
    static D& zero (D& target)
    {
        return *static_cast <D*> (std::memset (&target, 0, sizeof (D)));
    }

    // Zeroizes one element of "D" pointed by the "target".
    template<class D>
    static D* zero (D* target)
    {
        return static_cast <D*> (std::memset (target, 0, sizeof (D)));
    }

    // Zeroizes "count" elements of "D" pointed by the "target".
    template<class D>
    static D* zero (D* target, std::size_t count)
    {
        return static_cast <D*> (
            std::memset (target, 0, count * sizeof (D)));
    }

    // Zeroizes "count" bytes pointed by the "target".
    static void* zero (void* target, std::size_t count)
    {
        return std::memset (target, 0, count);
    }


private:
    Mem ();

    Mem (const Mem& that);

    ~Mem ();

    Mem& operator = (const Mem& that);
}; // class Mem


} // namespace bbi


#endif // BBI_MEM_H
