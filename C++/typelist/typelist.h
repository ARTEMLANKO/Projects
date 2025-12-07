#pragma once
#include <cstddef>
#include <utility>

namespace ct::tl {

template <typename... Types>
struct TypeList;

template <typename T, typename... List>
struct contains_impl;

template <typename T, template <typename...> typename List>
struct contains_impl<T, List<>> {
  static constexpr bool value = false;
};

template <typename T, template <typename...> typename List, typename... Ts>
struct contains_impl<T, List<Ts...>> {
  static constexpr bool value = (false || ... || std::is_same_v<T, Ts>);
};

template <typename T, typename List>
constexpr bool contains = contains_impl<T, List>::value;

template <typename... Lists>
struct concat_impl;

template <typename List>
struct concat_impl<List> {
  using type = List;
};

template <
    template <typename...> typename List1,
    template <typename...> typename List2,
    typename... Args1,
    typename... Args2,
    typename... Rest>
struct concat_impl<List1<Args1...>, List2<Args2...>, Rest...> {
  using type = typename concat_impl<List1<Args1..., Args2...>, Rest...>::type;
};

template <typename... Lists>
using concat = typename concat_impl<Lists...>::type;

template <typename List>
struct flip_all_impl;

template <template <typename...> typename List>
struct flip_all_impl<List<>> {
  using type = List<>;
};

template <typename A, typename B, typename C, typename D, template <typename...> typename List2>
struct flip_all_impl<std::pair<List2<A, B>, List2<C, D>>> {
  using type = std::pair<List2<B, A>, List2<D, C>>;
};

template <
    typename A,
    typename B,
    typename... Rest,
    template <typename...> typename List1,
    template <typename...> typename List2>
struct flip_all_impl<List1<List2<A, B>, Rest...>> {
  using flipped_pair = List2<B, A>;
  using rest_result = typename flip_all_impl<List1<Rest...>>::type;
  using type = concat<List1<flipped_pair>, rest_result>;
};

template <typename List>
using flip_all = typename flip_all_impl<List>::type;

template <typename T, typename List>
struct index_of_unique_impl;

template <typename T, template <typename...> typename List, typename... Rest>
struct index_of_unique_impl<T, List<T, Rest...>> {
  static constexpr size_t value = 0;
};

template <typename T, typename T1, template <typename...> typename List, typename... Rest>
struct index_of_unique_impl<T, List<T1, Rest...>> {
  static constexpr size_t value = 1 + index_of_unique_impl<T, List<Rest...>>::value;
};

template <typename T, typename List>
constexpr size_t index_of_unique = index_of_unique_impl<T, List>::value;

template <typename List>
struct flatten_impl {
  using type = TypeList<List>;
};

template <typename List>
using flatten = typename flatten_impl<List>::type;

template <template <typename...> typename List, typename... Types>
struct flatten_impl<List<Types...>> {
  using type = concat<List<>, flatten<Types>...>;
};

template <typename List, typename Left, typename Right, bool Current = false>
struct split_impl;

template <template <typename...> typename List, typename Left, typename Right, bool Current>
struct split_impl<List<>, Left, Right, Current> {
  using left = Left;
  using right = Right;
};

template <template <typename...> typename List, typename Head, typename... Tail, typename Left, typename Right>
struct split_impl<List<Head, Tail...>, Left, Right, false> {
  using nextLeft = concat<Left, List<Head>>;
  using type = split_impl<List<Tail...>, nextLeft, Right, true>;
  using left = typename type::left;
  using right = typename type::right;
};

template <template <typename...> typename List, typename Head, typename... Tail, typename Left, typename Right>
struct split_impl<List<Head, Tail...>, Left, Right, true> {
  using nextRight = concat<Right, List<Head>>;
  using type = split_impl<List<Tail...>, Left, nextRight, false>;
  using left = typename type::left;
  using right = typename type::right;
};

template <typename List1, typename List2, template <typename, typename> class Compare>
struct merge_impl;

template <template <typename...> typename List, template <typename, typename> class Compare>
struct merge_impl<List<>, List<>, Compare> {
  using type = List<>;
};

template <typename L1, template <typename...> typename List, template <typename, typename> class Compare>
struct merge_impl<L1, List<>, Compare> {
  using type = L1;
};

template <typename L2, template <typename...> typename List, template <typename, typename> class Compare>
struct merge_impl<List<>, L2, Compare> {
  using type = L2;
};

template <
    typename H1,
    typename... T1,
    typename H2,
    typename... T2,
    template <typename...> typename List,
    template <typename, typename> class Compare>
struct merge_impl<List<H1, T1...>, List<H2, T2...>, Compare> {
  static constexpr bool predicate = Compare<H1, H2>::value;
  using tail_if_left = typename merge_impl<List<T1...>, List<H2, T2...>, Compare>::type;
  using tail_if_right = typename merge_impl<List<H1, T1...>, List<T2...>, Compare>::type;
  using type =
      typename std::conditional<predicate, concat<List<H1>, tail_if_left>, concat<List<H2>, tail_if_right>>::type;
};

template <typename List, template <typename, typename> class Compare>
struct mergesort_impl;

template <template <typename...> typename List, template <typename, typename> class Compare>
struct mergesort_impl<List<>, Compare> {
  using type = List<>;
};

template <typename T, template <typename...> typename List, template <typename, typename> class Compare>
struct mergesort_impl<List<T>, Compare> {
  using type = List<T>;
};

template <typename... Args, template <typename...> typename List, template <typename, typename> class Compare>
struct mergesort_impl<List<Args...>, Compare> {
private:
  using parts = split_impl<List<Args...>, List<>, List<>, false>;
  using L = typename parts::left;
  using R = typename parts::right;

public:
  using sortedL = typename mergesort_impl<L, Compare>::type;
  using sortedR = typename mergesort_impl<R, Compare>::type;
  using type = typename merge_impl<sortedL, sortedR, Compare>::type;
};

template <typename List, template <typename, typename> class Compare>
using merge_sort = typename mergesort_impl<List, Compare>::type;
} // namespace ct::tl

// namespace ct::tl
