#ifndef BBI_OGL_UNIFORM_INFO_H
#define BBI_OGL_UNIFORM_INFO_H


namespace bbi {


class OglUniformInfo {
public:
    int index;
    int block_index;
    int item_type;
    int item_count;
    int buffer_offset;
    int array_stride;
    int matrix_stride;


    OglUniformInfo ();


    bool is_default_block () const;

    bool is_named_block () const;
}; // class OglUniformInfo


} // namespace bbi


#endif // BBI_OGL_UNIFORM_INFO_H
