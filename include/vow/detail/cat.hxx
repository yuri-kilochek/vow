#ifndef VOW_DETAIL_INCLUDE_GUARD_DETAIL_CAT
#define VOW_DETAIL_INCLUDE_GUARD_DETAIL_CAT

#include <boost/preprocessor/variadic/to_list.hpp>
#include <boost/preprocessor/list/cat.hpp>

#define VOW_DETAIL_CAT(...) \
    BOOST_PP_LIST_CAT(BOOST_PP_VARIADIC_TO_LIST(__VA_ARGS__))

#endif
