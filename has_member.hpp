
#include <type_traits>

// Credit: http://www.gockelhut.com/c++/articles/has_member

namespace detail {
    template <typename T, typename NameGetter>
    struct has_member_impl {
            typedef char matched_return_type;
            typedef long unmatched_return_type;

            template <typename C>
            static matched_return_type f(typename NameGetter::template get<C>*);

            template <typename C>
            static unmatched_return_type f(...);

        public:
            static const bool value = (sizeof(f<T>(0)) == sizeof(matched_return_type));
        };
    }


template <typename T, typename NameGetter>
struct has_member : std::integral_constant<bool, detail::has_member_impl<T, NameGetter>::value> {
    };