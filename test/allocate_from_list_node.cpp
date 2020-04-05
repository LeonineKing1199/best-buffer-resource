#include <sleip/buffer_resource.hpp>

#include <boost/assert.hpp>

#include <cstdint>
#include <array>

#include <boost/core/lightweight_test.hpp>

auto
to_uintptr(void* p) -> std::uintptr_t
{
  return reinterpret_cast<std::uintptr_t>(p);
}

auto
to_void(std::uintptr_t p) -> void*
{
  return reinterpret_cast<void*>(p);
}

auto
inc_void_ptr(void* const ptr, std::size_t const offset)
{
  return to_void(to_uintptr(ptr) + offset);
}

auto
construct_list_node(void* pos, std::size_t capacity) -> sleip::detail::free_list_node*
{
  auto const origin        = pos;
  auto const true_capacity = capacity;

  pos = std::align(alignof(sleip::detail::free_list_node), sizeof(sleip::detail::free_list_node),
                   pos, capacity);

  BOOST_ASSERT(pos != nullptr);

  return ::new (pos) sleip::detail::free_list_node{origin, true_capacity, nullptr};
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

  auto node_pos       = ptr.end;
  auto alloc_capacity = capacity;
  node_pos            = std::align(alignof(sleip::detail::free_list_node),
                        sizeof(sleip::detail::free_list_node), node_pos, alloc_capacity);

  free_node = ::new (node_pos) sleip::detail::free_list_node{ptr.end, capacity, node.next};
  return ptr;
}

void
test_construct_list_node()
{
  {
    auto       mem      = std::array<std::byte, 100>{};
    auto const offset   = 13;
    auto const capacity = mem.size() - offset;

    auto const node = construct_list_node(mem.data() + offset, capacity);

    auto const expected_origin = mem.data() + offset;

    BOOST_TEST_EQ(node->origin, expected_origin);
    BOOST_TEST_EQ(node->capacity, capacity);
    BOOST_TEST_EQ(node->next, nullptr);
    BOOST_TEST_NE(to_uintptr(node), to_uintptr(expected_origin));
  }

  {
    auto       mem      = std::array<std::byte, 100>{};
    auto const offset   = alignof(sleip::detail::free_list_node);
    auto const capacity = mem.size() - offset;

    auto const node = construct_list_node(mem.data() + offset, capacity);

    auto const expected_origin = mem.data() + offset;

    BOOST_TEST_EQ(node->origin, expected_origin);
    BOOST_TEST_EQ(node->capacity, capacity);
    BOOST_TEST_EQ(node->next, nullptr);
    BOOST_TEST_EQ(to_uintptr(node), to_uintptr(expected_origin));
  }
}

void
test_allocation_from_free_list_node()
{
  auto mem  = std::array<std::byte, 100>{};
  auto list = construct_list_node(mem.data(), mem.size());

  // the alignment of our free_list_node is 8 which is misaligned to 12 which means our free list
  // node will wind up existing at offset 16 of the provided mem buffer
  //
  auto const num_bytes = 3 * sizeof(int);

  auto ptr = alloc_from_list_node(num_bytes, alignof(int), list);

  BOOST_TEST_EQ(ptr.origin, mem.data());
  BOOST_TEST_EQ(ptr.pos, ptr.origin);
  BOOST_TEST_EQ(ptr.end, inc_void_ptr(ptr.pos, num_bytes));

  BOOST_TEST_NE(static_cast<void*>(list), ptr.end);
  BOOST_TEST_EQ(list->origin, ptr.end);
  BOOST_TEST_EQ(list->capacity, to_uintptr(mem.data() + mem.size()) - to_uintptr(ptr.end));
  BOOST_TEST_EQ(list->next, nullptr);

  std::destroy_at(list);
}

int
main()
{
  test_construct_list_node();
  test_allocation_from_free_list_node();

  return boost::report_errors();
}
