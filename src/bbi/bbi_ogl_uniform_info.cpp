//
// Custom library.
// Copyright (C) 2012-2013 Boris I. Bendovsky
//
// Container for information about OpenGL uniform variable.
//


#include "bbi_ogl_uniform_info.h"


namespace bbi
{


OglUniformInfo::OglUniformInfo () :
    index (-1),
    block_index (-1),
    item_type (0),
    item_count (0),
    buffer_offset (0),
    array_stride (0),
    matrix_stride (0)
{
}

bool OglUniformInfo::is_default_block () const
{
    return block_index == -1;
}

bool OglUniformInfo::is_named_block () const
{
    return block_index != -1;
}


} // namespace bbi
