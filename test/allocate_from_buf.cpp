#include <sleip/buffer_resource.hpp>

#include <cstdint>
#include <array>

#include <boost/core/lightweight_test.hpp>

void
test_round_up_aligned()
{
  BOOST_TEST_EQ(sleip::detail::round_up_aligned(9, 4), 12);
  BOOST_TEST_EQ(sleip::detail::round_up_aligned(13, 4), 16);
  BOOST_TEST_EQ(sleip::detail::round_up_aligned(12, 4), 12);
  BOOST_TEST_EQ(sleip::detail::round_up_aligned(13, 1), 13);
  BOOST_TEST_EQ(sleip::detail::round_up_aligned(8, 8), 8);
  BOOST_TEST_EQ(sleip::detail::round_up_aligned(16, 32), 32);
  BOOST_TEST_EQ(sleip::detail::round_up_aligned(16, 512), 512);
}

void
test_pointer_misaligned_origin_not_aligned_to_free_list_node_sufficient_space()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const offset = 9;
  static_assert(offset % alignof(int) > 0);

  auto const next_aligned = sleip::detail::round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 4;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data() + next_aligned);
  BOOST_TEST_EQ(p.end, mem.data() +
                         sleip::detail::round_up_aligned(next_aligned + num_bytes,
                                                         alignof(sleip::detail::free_list_node)));
  BOOST_TEST_NE(p.end, mem.data() + next_aligned + num_bytes);
  BOOST_TEST_EQ(
    capacity, mem.size() - sleip::detail::round_up_aligned(next_aligned + num_bytes,
                                                           alignof(sleip::detail::free_list_node)));
}

void
test_pointer_misaligned_origin_aligned_to_free_list_node_sufficient_space()
{
  static_assert(alignof(double) == 8);
  static_assert(sizeof(double) == 8);
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const offset = 9;
  static_assert(offset % alignof(double) > 0);

  auto const next_aligned = sleip::detail::round_up_aligned(offset, alignof(double));
  static_assert(next_aligned % alignof(double) == 0);

  auto const num_bytes = sizeof(double) * 4;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(double), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data() + next_aligned);
  BOOST_TEST_EQ(p.end, mem.data() +
                         sleip::detail::round_up_aligned(next_aligned + num_bytes,
                                                         alignof(sleip::detail::free_list_node)));
  BOOST_TEST_EQ(p.end, mem.data() + next_aligned + num_bytes);
  BOOST_TEST_EQ(capacity, mem.size() - next_aligned - num_bytes);
}

void
test_pointer_aligned_oiring_not_aligned_to_free_list_node_sufficient_capacity()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const next_aligned = sleip::detail::round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 4;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data() + next_aligned);
  BOOST_TEST_EQ(p.end, mem.data() +
                         sleip::detail::round_up_aligned(next_aligned + num_bytes,
                                                         alignof(sleip::detail::free_list_node)));
  BOOST_TEST_NE(p.end, mem.data() + next_aligned + num_bytes);
  BOOST_TEST_EQ(
    capacity, mem.size() - sleip::detail::round_up_aligned(next_aligned + num_bytes,
                                                           alignof(sleip::detail::free_list_node)));
}

void
test_pointer_aligned_origin_aligned_to_free_list_node_sufficient_capacity()
{
  static_assert(alignof(double) == 8);
  static_assert(sizeof(double) == 8);
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const offset = 8;
  static_assert(offset % alignof(double) == 0);

  auto const next_aligned = sleip::detail::round_up_aligned(offset, alignof(double));
  static_assert(next_aligned % alignof(double) == 0);

  auto const num_bytes = sizeof(double) * 4;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(double), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data() + next_aligned);
  BOOST_TEST_EQ(p.end, mem.data() +
                         sleip::detail::round_up_aligned(next_aligned + num_bytes,
                                                         alignof(sleip::detail::free_list_node)));
  BOOST_TEST_EQ(p.end, mem.data() + next_aligned + num_bytes);
  BOOST_TEST_EQ(capacity, mem.size() - next_aligned - num_bytes);
}

void
test_pointer_aligned_origin_not_aligned_to_free_list_node_use_full_capacity()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const next_aligned = sleip::detail::round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 24;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data() + next_aligned);
  BOOST_TEST_EQ(p.end, mem.data() + mem.size());
  BOOST_TEST_EQ(capacity, 0);
}

void
test_pointer_aligned_origin_not_aligned_to_free_list_node_no_froom_for_free_list_node()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const next_aligned = sleip::detail::round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 23;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data() + next_aligned);
  BOOST_TEST_EQ(p.end, mem.data() + mem.size());
  BOOST_TEST_EQ(capacity, 0);
}

void
test_throwing_bad_alloc()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const next_aligned = sleip::detail::round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 24;

  auto mem = std::array<std::byte, 20>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;

  BOOST_TEST_THROWS(sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity),
                    std::bad_alloc);
}

void
test_char_allocation_unaligned_to_free_list_node_with_room_for_list_node()
{
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const num_bytes = 70;

  static_assert(num_bytes % alignof(sleip::detail::free_list_node) > 0);

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data();

  auto capacity = mem.size();
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data());
  BOOST_TEST_EQ(p.end, mem.data() + sleip::detail::round_up_aligned(
                                      num_bytes, alignof(sleip::detail::free_list_node)));
  BOOST_TEST_EQ(capacity, mem.size() - num_bytes);
}

void
test_char_allocation_unaligned_to_free_list_node_with_no_room_for_list_node()
{
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const num_bytes = 73;

  static_assert(num_bytes % alignof(sleip::detail::free_list_node) > 0);

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data();

  auto capacity = mem.size();
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(char), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data());
  BOOST_TEST_EQ(p.end, mem.data() + mem.size());
  BOOST_TEST_EQ(capacity, 0);
}

int
main()
{
  test_round_up_aligned();
  test_pointer_misaligned_origin_not_aligned_to_free_list_node_sufficient_space();
  test_pointer_misaligned_origin_aligned_to_free_list_node_sufficient_space();
  test_pointer_aligned_oiring_not_aligned_to_free_list_node_sufficient_capacity();
  test_pointer_aligned_origin_aligned_to_free_list_node_sufficient_capacity();
  test_pointer_aligned_origin_not_aligned_to_free_list_node_use_full_capacity();
  test_pointer_aligned_origin_not_aligned_to_free_list_node_no_froom_for_free_list_node();
  test_throwing_bad_alloc();
  test_char_allocation_unaligned_to_free_list_node_with_room_for_list_node();
  test_char_allocation_unaligned_to_free_list_node_with_no_room_for_list_node();

  return boost::report_errors();
}
