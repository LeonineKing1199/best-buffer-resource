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

// Allocate from a misaligned location in the buffer
// i.e. given an int with an alignment requirement of 4, allocate from offset 9 in the memory buffer
//
void
test1()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);

  auto const offset = 9;
  static_assert(offset % alignof(int) > 0);

  auto const  next_aligned = sleip::detail::round_up_aligned(offset, alignof(int));
  auto const  num_bytes    = sizeof(int) * 4;
  auto        mem          = std::array<std::byte, 100>{};
  auto* const origin       = mem.data() + offset;
  auto        capacity     = mem.size() - offset;

  auto p = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data() + next_aligned);
  BOOST_TEST_EQ(p.end, mem.data() + next_aligned + num_bytes);
  BOOST_TEST_EQ(capacity, mem.size() - next_aligned - num_bytes);
}

// test an allocation where the offset is aligned to the type we're allocating
// i.e. allocate an int with alignment 4 at an offset of 4
void
test2()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const  next_aligned = sleip::detail::round_up_aligned(offset, alignof(int));
  auto const  num_bytes    = sizeof(int) * 4;
  auto        mem          = std::array<std::byte, 100>{};
  auto* const origin       = mem.data() + offset;
  auto        capacity     = mem.size() - offset;

  auto p = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, mem.data() + next_aligned);
  BOOST_TEST_EQ(p.end, mem.data() + next_aligned + num_bytes);
  BOOST_TEST_EQ(capacity, mem.size() - next_aligned - num_bytes);
}

// consume an entire memory block
//
void
test3()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 24;
  static_assert(offset + num_bytes == 100);

  auto        mem      = std::array<std::byte, 100>{};
  auto* const origin   = mem.data() + offset;
  auto        capacity = mem.size() - offset;

  auto p = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, origin);
  BOOST_TEST_EQ(p.end, mem.data() + mem.size());
  BOOST_TEST_EQ(capacity, 0);
}

// consume just enough of a memory block so as to not leave room for a free node
// doing this should consume the entire memory block
//
void
test4()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  // 100 - 4 - 22 * 4 => 100 - 92 => 8 remaining bytes which is < sizeof(free_list_node)
  //
  auto const  num_bytes = sizeof(int) * 22;
  auto        mem       = std::array<std::byte, 100>{};
  auto* const origin    = mem.data() + offset;
  auto        capacity  = mem.size() - offset;

  auto p = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, origin);
  BOOST_TEST_EQ(p.end, mem.data() + mem.size());
  BOOST_TEST_EQ(capacity, 0);
}

// test the case where we just don't have enough room outright, i.e. throw bad_alloc
//
void
test5()
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const  num_bytes = sizeof(int) * 24;
  auto        mem       = std::array<std::byte, 20>{};
  auto* const origin    = mem.data() + offset;
  auto        capacity  = mem.size() - offset;

  BOOST_TEST_THROWS(sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity),
                    std::bad_alloc);
}

// test allocation with an alignment of 1
void
test6()
{
  auto const  num_bytes = 70;
  auto        mem       = std::array<std::byte, 100>{};
  auto* const origin    = mem.data();
  auto        capacity  = mem.size();

  auto p = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(char), capacity);

  BOOST_TEST_EQ(p.origin, origin);
  BOOST_TEST_EQ(p.pos, origin);
  BOOST_TEST_EQ(p.end, mem.data() + num_bytes);
  BOOST_TEST_EQ(capacity, mem.size() - num_bytes);
}

int
main()
{
  test_round_up_aligned();

  test1();
  test2();
  test3();
  test4();
  test5();
  test6();

  return boost::report_errors();
}
