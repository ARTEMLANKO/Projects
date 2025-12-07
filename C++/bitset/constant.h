#include <cstdint>
#include <limits>

using Word = uint64_t;
inline constexpr Word Word_size = std::numeric_limits<Word>::digits;
inline constexpr Word ONE = 1ULL;
inline constexpr Word ZERO = 0ULL;
inline constexpr Word MAX_WORD = ~0ULL;
