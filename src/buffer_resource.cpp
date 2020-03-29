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

  auto end = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(pos) +
                                     round_up_aligned(num_bytes, alignment));

  capacity -= num_bytes;
  if (capacity == 0) { return {origin, pos, end}; }

  end = std::align(alignof(free_list_node), sizeof(free_list_node), end, capacity);
  if (end == nullptr) {
    end = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(pos) +
                                  round_up_aligned(num_bytes, alignment) + capacity);

    capacity = 0;
  }

  return {origin, pos, end};
}
} // namespace detail
} // namespace sleip
