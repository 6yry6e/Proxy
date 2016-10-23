#ifndef _PTR_CMP_HPP
#define _PTR_CMP_HPP

#include <type_traits>
#include <functional>
#include <memory>

template<class T>
struct ptr_comp {
  typedef std::true_type is_transparent;
  struct wraper {
    T* ptr;
    wraper() :ptr(nullptr) {}
    wraper(wraper const&) = default;
    wraper(T* p) :ptr(p) {}
    template<class U>
    wraper(std::shared_ptr<U> const& sp) : ptr(sp.get()) {}
    template<class U, class...Ts>
    wraper(std::unique_ptr<U, Ts...> const& up) : ptr(up.get()) {}
    bool operator<(const wraper& o) const {
      return std::less<T*>()(ptr, o.ptr);
    }
  };
  bool operator()(wraper const&& lhs, wraper const&& rhs) const {
    return lhs < rhs;
  }
};

#endif