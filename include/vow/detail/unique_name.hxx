#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_UNIQUE_NAME
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_UNIQUE_NAME

#include <vow/detail/cat.hxx>

#define VOW_DETAIL_UNIQUE_NAME(seed) \
    VOW_DETAIL_CAT(_vow_detail_, seed, _, __LINE__, _)

#endif
