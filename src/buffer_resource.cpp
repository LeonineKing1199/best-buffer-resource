#include <sleip/buffer_resource.hpp>

namespace sleip
{
namespace detail
{
auto
alloc_from_buf(void*             buf,
               std::size_t const num_bytes,
               std::size_t const alignment,
               std::size_t&      capacity) -> pointer
{
  auto const origin = buf;
  auto       pos    = std::align(alignment, num_bytes, buf, capacity);
  if (pos == nullptr) { throw std::bad_alloc{}; }

  auto const true_num_bytes = round_up_aligned(num_bytes, alignment);
  auto const end = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(pos) + true_num_bytes);

  capacity -= true_num_bytes;
  if (capacity == 0) { return {origin, pos, end}; }

  auto alloc_end      = end;
  auto alloc_capacity = capacity;

  alloc_end =
    std::align(alignof(free_list_node), sizeof(free_list_node), alloc_end, alloc_capacity);

  if (alloc_end == nullptr) {
    capacity = 0;
    return {origin, pos,
            reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(pos) + true_num_bytes +
                                    alloc_capacity)};
  }

  return {origin, pos, end};
}
} // namespace detail
} // namespace sleip
