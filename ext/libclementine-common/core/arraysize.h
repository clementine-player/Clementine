template <typename T>
constexpr size_t arraysize(const T&) {
  static_assert(std::is_array<T>::value, "Argument must be array");
  return std::extent<T>::value;
}
