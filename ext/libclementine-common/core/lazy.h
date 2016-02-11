#include <functional>
#include <memory>

template <typename T>
class Lazy {
 public:
  explicit Lazy(std::function<T*()> init) : init_(init) {}

  T* get() {
    if (!ptr_) {
      ptr_.reset(init_());
    }
    return ptr_.get();
  }

  typename std::add_lvalue_reference<T>::type operator*() const {
    return *ptr_;
  }

  T* operator->() const { return ptr_.get(); }

 private:
  std::function<T*()> init_;
  std::unique_ptr<T> ptr_;
};
