#pragma once

#include <algorithm>
#include <cassert>
#include <limits>
#include <numeric>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace thh
{
  // default tag to circumvent type safety
  struct default_tag_t
  {
  };

  // weak handle to underlying data stored in the container
  // note: fields should not be modified
  template<typename Tag, typename Index = int32_t, typename Gen = int32_t>
  struct typed_handle_t
  {
    static_assert(
      std::is_integral<Index>::value, "Index must be an integral type.");
    static_assert(std::is_signed<Index>::value, "Index must be a signed type.");
    static_assert(
      std::is_integral<Gen>::value, "Index must be an integral type.");
    static_assert(std::is_signed<Gen>::value, "Index must be a signed type.");

    Index id_ = -1; // value to uniquely identify the handle
    Gen gen_ = -1; // current handle generation

    typed_handle_t() = default;
    typed_handle_t(const Index id, const Gen gen) : id_(id), gen_(gen) {}
  };

  // handle equality operators
  template<typename Tag, typename Index, typename Gen>
  bool operator==(
    const typed_handle_t<Tag, Index, Gen>& lhs,
    const typed_handle_t<Tag, Index, Gen>& rhs);
  template<typename Tag, typename Index, typename Gen>
  bool operator!=(
    const typed_handle_t<Tag, Index, Gen>& lhs,
    const typed_handle_t<Tag, Index, Gen>& rhs);

  using handle_t = typed_handle_t<default_tag_t, int32_t, int32_t>;

  // storage for type T that is created in-place
  // may be accessed by resolving the returned typed_handle_t from add()
  // note: provide a custom tag to create a type-safe container-handle pair
  template<
    typename T, typename Tag = default_tag_t, typename Index = int32_t,
    typename Gen = int32_t>
  class handle_vector_t
  {
    // internal mapping from external handle to internal element
    // maintains a reference to the next free handle
    struct internal_handle_t
    {
      typed_handle_t<Tag, Index, Gen> handle_; // handle to be looked up
      Index lookup_ = -1; // mapping to element
      Index next_ = -1; // index of next available handle
    };

    // backing container for elements (vector remains tightly packed)
    std::vector<T> elements_;
    // parallel vector of ids that map from elements back to the corresponding
    // handle
    std::vector<Index> element_ids_;
    // sparse vector of handles to elements
    std::vector<internal_handle_t> handles_;

    // index of the next handle to be allocated
    Index next_ = 0;
    // number of handles that are depleted (generation is at its limit)
    Index depleted_handles_ = 0;

    // increases the number of available handles when the underlying container
    // of elements (T) grows (the capacity increases)
    void try_allocate_handles();
    // after sorting or partitioning the container, ensures handles refer to the
    // same value as before
    // begin - inclusive, end - exclusive
    void fixup_handles(Index begin, Index end);
    // returns a mutable pointer to the underlying element T referenced by the
    // handle
    [[nodiscard]] T* resolve(typed_handle_t<Tag, Index, Gen> handle);
    // returns a constant pointer to the underlying element T referenced by the
    // handle
    [[nodiscard]] const T* resolve(
      typed_handle_t<Tag, Index, Gen> handle) const;

  public:
    using iterator = typename decltype(elements_)::iterator;
    using const_iterator = typename decltype(elements_)::const_iterator;
    using reverse_iterator = typename decltype(elements_)::reverse_iterator;
    using const_reverse_iterator =
      typename decltype(elements_)::const_reverse_iterator;
    using value_type = typename decltype(elements_)::value_type;
    using reference = typename decltype(elements_)::reference;
    using const_reference = typename decltype(elements_)::const_reference;

    // creates an element T in-place and returns a handle to it
    // note: args allow arguments to be passed directly to the type constructor
    // useful if the type does not support a default constructor
    template<typename... Args>
    typed_handle_t<Tag, Index, Gen> add(Args&&... args);
    // invokes a callable object (usually a lambda) on a particular element in
    // the container
    template<typename Fn>
    void call(typed_handle_t<Tag, Index, Gen> handle, Fn&& fn);
    // invokes a callable object (usually a lambda) on a particular element in
    // the container (const overload)
    template<typename Fn>
    void call(typed_handle_t<Tag, Index, Gen> handle, Fn&& fn) const;
    // invokes a callable object (usually a lambda) on a particular element in
    // the container and returns a std::optional containing either the result
    // or an empty optional (as the handle may not have been successfully
    // resolved)
    template<typename Fn>
    decltype(auto) call_return(typed_handle_t<Tag, Index, Gen> handle, Fn&& fn);
    // invokes a callable object (usually a lambda) on a particular element in
    // the container and returns a std::optional containing either the result
    // or an empty optional (as the handle may not have been successfully
    // resolved) (const overload)
    template<typename Fn>
    decltype(auto) call_return(
      typed_handle_t<Tag, Index, Gen> handle, Fn&& fn) const;
    // removes the element referenced by the handle
    // returns true if the element was removed, false otherwise (the handle was
    // invalid or could not be found in the container)
    bool remove(typed_handle_t<Tag, Index, Gen> handle);
    // returns if the container still has the element referenced by the handle
    [[nodiscard]] bool has(typed_handle_t<Tag, Index, Gen> handle) const;
    // returns the number of elements currently stored in the container
    [[nodiscard]] Index size() const;
    // returns the number of available handles (includes element storage that is
    // reserved but not yet in use)
    [[nodiscard]] Index capacity() const;
    // reserves underlying memory for the number of elements specified
    void reserve(Index capacity);
    // removes all elements and invalidates all handles
    // note: capacity remains unchanged, internal handles are not cleared
    void clear();
    // returns the handle for a value at the given index
    // note: will return an invalid handle if the index is out of range
    typed_handle_t<Tag, Index, Gen> handle_from_index(Index index) const;
    // returns the index (position) of a value for a given handle
    // note: will return an empty optional if the handle is invalid
    std::optional<Index> index_from_handle(
      typed_handle_t<Tag, Index, Gen> handle) const;
    // returns the index from the id part of handle
    // note: will return an empty optional if handle id is invalid
    std::optional<Index> index_from_id(Index id) const;
    // returns if the container has any elements or not
    [[nodiscard]] bool empty() const;
    // returns mutable reference to element at position
    // note: position must be in range (0 <= position < size)
    T& operator[](Index position);
    // returns constant reference to element at position
    // note: position must be in range (0 <= position < size)
    const T& operator[](Index position) const;
    // returns a pointer to the underlying element storage
    T* data();
    // returns a const pointer to the underlying element storage
    const T* data() const;
    // returns an iterator to the beginning of the elements
    auto begin() -> iterator;
    // returns a const iterator to the beginning of the elements
    auto begin() const -> const_iterator;
    // returns a const iterator to the beginning of the elements
    auto cbegin() const -> const_iterator;
    // returns a reverse iterator to the last element
    auto rbegin() -> reverse_iterator;
    // returns a const reverse iterator to the last element
    auto rbegin() const -> const_reverse_iterator;
    // returns a const reverse iterator to the last element
    auto crbegin() const -> const_reverse_iterator;
    // returns an iterator to the end of the elements
    auto end() -> iterator;
    // returns a const iterator to the end of the elements
    auto end() const -> const_iterator;
    // returns a const iterator to the end of the elements
    auto cend() const -> const_iterator;
    // returns a reverse iterator to one before the first element
    auto rend() -> reverse_iterator;
    // returns a const reverse iterator to one before the first element
    auto rend() const -> const_reverse_iterator;
    // returns a const reverse iterator to one before the first element
    auto crend() const -> const_reverse_iterator;
    // returns an ascii representation of the currently allocated handles
    // note: useful for debugging purposes
    [[nodiscard]] std::string debug_handles() const;
    // sorts elements in the container according to the provided comparison
    template<typename Compare>
    void sort(Compare&& compare);
    // sorts elements in the container in the specified range according to the
    // provided comparison
    // begin - inclusive, end - exclusive
    template<typename Compare>
    void sort(Index begin, Index end, Compare&& compare);
    // partitions elements in the container according to the provided predicate
    // returns index of the first element for the second group
    template<typename Predicate>
    Index partition(Predicate&& predicate);
  };
} // namespace thh

#include "handle-vector.inl"
