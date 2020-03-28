#include <sleip/buffer_resource.hpp>

#include <cstdint>
#include <array>

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

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

TEST_CASE("round_up_aligned")
{
  CHECK(round_up_aligned(9, 4) == 12);
  CHECK(round_up_aligned(13, 4) == 16);
  CHECK(round_up_aligned(12, 4) == 12);
  CHECK(round_up_aligned(13, 1) == 13);
  CHECK(round_up_aligned(8, 8) == 8);
  CHECK(round_up_aligned(16, 32) == 32);
  CHECK(round_up_aligned(16, 512) == 512);
}

TEST_CASE("pointer, mis-aligned origin, not aligned to free_list_node, sufficient space")
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(free_list_node) == 8);

  auto const offset = 9;
  static_assert(offset % alignof(int) > 0);

  auto const next_aligned = round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 4;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + round_up_aligned(next_aligned + num_bytes, alignof(free_list_node)));
  CHECK(p.end != mem.data() + next_aligned + num_bytes);
  CHECK(capacity ==
        mem.size() - round_up_aligned(next_aligned + num_bytes, alignof(free_list_node)));
}

TEST_CASE("pointer, mis-aligned origin, aligned to free_list_node, sufficient space")
{
  static_assert(alignof(double) == 8);
  static_assert(sizeof(double) == 8);
  static_assert(alignof(free_list_node) == 8);

  auto const offset = 9;
  static_assert(offset % alignof(double) > 0);

  auto const next_aligned = round_up_aligned(offset, alignof(double));
  static_assert(next_aligned % alignof(double) == 0);

  auto const num_bytes = sizeof(double) * 4;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = alloc_from_buf(origin, num_bytes, alignof(double), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + round_up_aligned(next_aligned + num_bytes, alignof(free_list_node)));
  CHECK(p.end == mem.data() + next_aligned + num_bytes);
  CHECK(capacity == mem.size() - next_aligned - num_bytes);
}

TEST_CASE("pointer, aligned origin, not aligned to free list node, sufficient capacity")
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(free_list_node) == 8);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const next_aligned = round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 4;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + round_up_aligned(next_aligned + num_bytes, alignof(free_list_node)));
  CHECK(p.end != mem.data() + next_aligned + num_bytes);
  CHECK(capacity ==
        mem.size() - round_up_aligned(next_aligned + num_bytes, alignof(free_list_node)));
}

TEST_CASE("pointer, aligned origin, aligned to free list node, sufficient capacity")
{
  static_assert(alignof(double) == 8);
  static_assert(sizeof(double) == 8);
  static_assert(alignof(free_list_node) == 8);

  auto const offset = 8;
  static_assert(offset % alignof(double) == 0);

  auto const next_aligned = round_up_aligned(offset, alignof(double));
  static_assert(next_aligned % alignof(double) == 0);

  auto const num_bytes = sizeof(double) * 4;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = alloc_from_buf(origin, num_bytes, alignof(double), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + round_up_aligned(next_aligned + num_bytes, alignof(free_list_node)));
  CHECK(p.end == mem.data() + next_aligned + num_bytes);
  CHECK(capacity == mem.size() - next_aligned - num_bytes);
}

TEST_CASE("pointer, aligned origin, not aligned to free list node, use full capacity")
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(free_list_node) == 8);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const next_aligned = round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 24;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + mem.size());
  CHECK(capacity == 0);
}

TEST_CASE("pointer, aligned origin, not aligned to free list node, no room for free list node")
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(free_list_node) == 8);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const next_aligned = round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 23;

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;
  auto p        = alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data() + next_aligned);
  CHECK(p.end == mem.data() + mem.size());
  CHECK(capacity == 0);
}

TEST_CASE("throwing bad_alloc")
{
  static_assert(alignof(int) == 4);
  static_assert(sizeof(int) == 4);
  static_assert(alignof(free_list_node) == 8);

  auto const offset = 4;
  static_assert(offset % alignof(int) == 0);

  auto const next_aligned = round_up_aligned(offset, alignof(int));
  static_assert(next_aligned % alignof(int) == 0);

  auto const num_bytes = sizeof(int) * 24;

  auto mem = std::array<std::byte, 20>{};

  auto* const origin = mem.data() + offset;

  auto capacity = mem.size() - offset;

  CHECK_THROWS_AS(alloc_from_buf(origin, num_bytes, alignof(int), capacity), std::bad_alloc);
}

TEST_CASE("char allocation unaligned to free_list_node, room for list node")
{
  static_assert(alignof(free_list_node) == 8);

  auto const num_bytes = 70;

  static_assert(num_bytes % alignof(free_list_node) > 0);

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data();

  auto capacity = mem.size();
  auto p        = alloc_from_buf(origin, num_bytes, alignof(int), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data());
  CHECK(p.end == mem.data() + round_up_aligned(num_bytes, alignof(free_list_node)));
  CHECK(capacity == mem.size() - num_bytes);
}

TEST_CASE("char allocation unaligned to free_list_node, no room for list node")
{
  static_assert(alignof(free_list_node) == 8);

  auto const num_bytes = 73;

  static_assert(num_bytes % alignof(free_list_node) > 0);

  auto mem = std::array<std::byte, 100>{};

  auto* const origin = mem.data();

  auto capacity = mem.size();
  auto p        = alloc_from_buf(origin, num_bytes, alignof(char), capacity);

  CHECK(p.origin == origin);
  CHECK(p.pos == mem.data());
  CHECK(p.end == mem.data() + mem.size());
  CHECK(capacity == 0);
}
