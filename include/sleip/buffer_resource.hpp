#ifndef SLEIP_BUFFER_RESOURCE_HPP_
#define SLEIP_BUFFER_RESOURCE_HPP_

#include <memory>
#include <cstddef>

namespace sleip
{
namespace detail
{
struct free_list_node
{
  void*           origin;
  std::size_t     capacity;
  free_list_node* next;
};

struct pointer
{
  void* origin;
  void* pos;
  void* end;
};

constexpr auto
round_up_aligned(std::size_t const num_bytes, std::size_t const alignment) -> std::size_t
{
  auto const remaining = num_bytes % alignment;
  if (remaining == 0) { return num_bytes; }
  return num_bytes + (alignment - remaining);
}

auto
alloc_from_buf(void*             buf,
               std::size_t const num_bytes,
               std::size_t const alignment,
               std::size_t&      capacity) -> pointer;

} // namespace detail

} // namespace sleip

#endif // SLEIP_BUFFER_RESOURCE_HPP_
