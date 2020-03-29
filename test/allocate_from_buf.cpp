#include <sleip/buffer_resource.hpp>

#include <cstdint>
#include <array>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

TEST_CASE("round_up_aligned")
{
  CHECK(sleip::detail::round_up_aligned(9, 4) == 12);
  CHECK(sleip::detail::round_up_aligned(13, 4) == 16);
  CHECK(sleip::detail::round_up_aligned(12, 4) == 12);
  CHECK(sleip::detail::round_up_aligned(13, 1) == 13);
  CHECK(sleip::detail::round_up_aligned(8, 8) == 8);
  CHECK(sleip::detail::round_up_aligned(16, 32) == 32);
  CHECK(sleip::detail::round_up_aligned(16, 512) == 512);
}

TEST_CASE("pointer, mis-aligned origin, not aligned to free_list_node, sufficient space")
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

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + sleip::detail::round_up_aligned(
                                next_aligned + num_bytes, alignof(sleip::detail::free_list_node)));
  CHECK(p.end != mem.data() + next_aligned + num_bytes);
  CHECK(capacity ==
        mem.size() - sleip::detail::round_up_aligned(next_aligned + num_bytes,
                                                     alignof(sleip::detail::free_list_node)));
}

TEST_CASE("pointer, mis-aligned origin, aligned to free_list_node, sufficient space")
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

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + sleip::detail::round_up_aligned(
                                next_aligned + num_bytes, alignof(sleip::detail::free_list_node)));
  CHECK(p.end == mem.data() + next_aligned + num_bytes);
  CHECK(capacity == mem.size() - next_aligned - num_bytes);
}

TEST_CASE("pointer, aligned origin, not aligned to free list node, sufficient capacity")
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

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + sleip::detail::round_up_aligned(
                                next_aligned + num_bytes, alignof(sleip::detail::free_list_node)));
  CHECK(p.end != mem.data() + next_aligned + num_bytes);
  CHECK(capacity ==
        mem.size() - sleip::detail::round_up_aligned(next_aligned + num_bytes,
                                                     alignof(sleip::detail::free_list_node)));
}

TEST_CASE("pointer, aligned origin, aligned to free list node, sufficient capacity")
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

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + sleip::detail::round_up_aligned(
                                next_aligned + num_bytes, alignof(sleip::detail::free_list_node)));
  CHECK(p.end == mem.data() + next_aligned + num_bytes);
  CHECK(capacity == mem.size() - next_aligned - num_bytes);
}

TEST_CASE("pointer, aligned origin, not aligned to free list node, use full capacity")
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

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + mem.size());
  CHECK(capacity == 0);
}

TEST_CASE("pointer, aligned origin, not aligned to free list node, no room for free list node")
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

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + mem.size());
  CHECK(capacity == 0);
}

TEST_CASE("throwing bad_alloc")
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

  CHECK_THROWS_AS(sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity),
                  std::bad_alloc);
}

TEST_CASE("char allocation unaligned to free_list_node, room for list node")
{
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const num_bytes = 70;

  static_assert(num_bytes % alignof(sleip::detail::free_list_node) > 0);

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data();

  auto capacity = mem.size();
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data());
  CHECK(p.end == mem.data() + sleip::detail::round_up_aligned(
                                num_bytes, alignof(sleip::detail::free_list_node)));
  CHECK(capacity == mem.size() - num_bytes);
}

TEST_CASE("char allocation unaligned to free_list_node, no room for list node")
{
  static_assert(alignof(sleip::detail::free_list_node) == 8);

  auto const num_bytes = 73;

  static_assert(num_bytes % alignof(sleip::detail::free_list_node) > 0);

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data();

  auto capacity = mem.size();
  auto p        = sleip::detail::alloc_from_buf(origin, num_bytes, alignof(char), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data());
  CHECK(p.end == mem.data() + mem.size());
  CHECK(capacity == 0);
}
