#include <sleip/buffer_resource.hpp>

#include <cstdint>
#include <array>

#include <boost/core/lightweight_test.hpp>

auto
inc_void_ptr(void* const ptr, std::size_t const offset)
{
  return reinterpret_cast<void*>(reinterpret_cast<std::size_t>(ptr) + offset);
}

auto
alloc_from_list_node(std::size_t const               num_bytes,
                     std::size_t const               alignment,
                     sleip::detail::free_list_node*& free_node) -> sleip::detail::pointer
{
  auto node     = *free_node;
  auto capacity = node.capacity;

  std::destroy_at(free_node);

  auto ptr = sleip::detail::alloc_from_buf(node.origin, num_bytes, alignment, capacity);

  if (ptr.end == inc_void_ptr(node.origin, node.capacity)) {
    free_node = node.next;
    return ptr;
  }

  auto node_pos = ptr.end;
  node_pos      = std::align(alignof(sleip::detail::free_list_node),
                        sizeof(sleip::detail::free_list_node), node_pos, capacity);

  free_node = ::new (node_pos) sleip::detail::free_list_node{ptr.end, capacity, node.next};
  return ptr;
}

void
test_allocation_from_free_list_node()
{
  auto mem = std::array<std::byte, 100>{};

  auto pos      = static_cast<void*>(mem.data());
  auto capacity = mem.size();
  pos = std::align(alignof(sleip::detail::free_list_node), sizeof(sleip::detail::free_list_node),
                   pos, capacity);

  BOOST_TEST_NE(pos, nullptr);
  BOOST_TEST_EQ(reinterpret_cast<std::uintptr_t>(mem.data()) % alignof(int), 0);

  auto list = ::new (pos) sleip::detail::free_list_node{mem.data(), mem.size(), nullptr};

  auto const num_bytes = 3 * sizeof(int);

  auto ptr = alloc_from_list_node(num_bytes, alignof(int), list);

  BOOST_TEST_EQ(ptr.origin, mem.data());
  BOOST_TEST_EQ(ptr.pos, mem.data());
  BOOST_TEST_EQ(ptr.end, mem.data() + num_bytes);

  BOOST_TEST_EQ(inc_void_ptr(ptr.pos, num_bytes), ptr.end);
  BOOST_TEST_NE(inc_void_ptr(ptr.pos, num_bytes), static_cast<void*>(list));
  BOOST_TEST_NE(ptr.end, static_cast<void*>(list));

  BOOST_TEST_EQ(list->origin,
                reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(ptr.pos) + num_bytes));

  BOOST_TEST_EQ(list->capacity, mem.size() - sleip::detail::round_up_aligned(
                                               num_bytes, alignof(sleip::detail::free_list_node)));
  BOOST_TEST_EQ(list->next, nullptr);

  std::destroy_at(list);
}

int
main()
{
  test_allocation_from_free_list_node();

  return boost::report_errors();
}
