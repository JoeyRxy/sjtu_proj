#include <memory>
#include <utility>
#include <typeinfo>

namespace rxy {

template <typename T>
struct is_in_place_type : std::false_type {};

template <typename T>
struct is_in_place_type<std::in_place_type_t<T>> : std::true_type {};

class any {
  template <typename ValueType>
  friend const ValueType *any_cast(const any *) noexcept;

  template <typename ValueType>
  friend ValueType *any_cast(any *) noexcept;

public:
  // constructors

  constexpr any() noexcept = default;

  any(const any &other) {
    if (other.instance) {
      instance = other.instance->clone();
    }
  }

  any(any &&other) noexcept
    : instance(std::move(other.instance)) {}

  template <typename ValueType, typename = std::enable_if_t<
    !std::is_same_v<std::decay_t<ValueType>, any> &&
    !is_in_place_type<std::decay_t<ValueType>>::value &&
    std::is_copy_constructible_v<std::decay_t<ValueType>>
  >>
  any(ValueType &&value) {
    static_assert(std::is_copy_constructible_v<std::decay_t<ValueType>>, "program is ill-formed");
    emplace<std::decay_t<ValueType>>(std::forward<ValueType>(value));
  }

  template <typename ValueType, typename... Args, typename = std::enable_if_t<
    std::is_constructible_v<std::decay_t<ValueType>, Args...> &&
    std::is_copy_constructible_v<std::decay_t<ValueType>>
  >>
  explicit any(std::in_place_type_t<ValueType>, Args &&... args) {
    emplace<std::decay_t<ValueType>>(std::forward<Args>(args)...);
  }

  template <typename ValueType, typename List, typename... Args, typename = std::enable_if_t<
    std::is_constructible_v<std::decay_t<ValueType>, std::initializer_list<List> &, Args...> &&
    std::is_copy_constructible_v<std::decay_t<ValueType>>
  >>
  explicit any(std::in_place_type_t<ValueType>, std::initializer_list<List> list, Args &&... args) {
    emplace<std::decay_t<ValueType>>(list, std::forward<Args>(args)...);
  }

  // assignment operators

  any &operator=(const any &rhs) {
    any(rhs).swap(*this);
    return *this;
  }

  any &operator=(any &&rhs) noexcept {
    any(std::move(rhs)).swap(*this);
    return *this;
  }

  template <typename ValueType>
  std::enable_if_t<
    !std::is_same_v<std::decay_t<ValueType>, any> &&
    std::is_copy_constructible_v<std::decay_t<ValueType>>,
    any &
  >
  operator=(ValueType &&rhs) {
    any(std::forward<ValueType>(rhs)).swap(*this);
    return *this;
  }

  // modifiers

  template <typename ValueType, typename... Args>
  std::enable_if_t<
    std::is_constructible_v<std::decay_t<ValueType>, Args...> &&
    std::is_copy_constructible_v<std::decay_t<ValueType>>,
    std::decay_t<ValueType> &
  >
  emplace(Args &&... args) {
    auto new_inst = std::make_unique<storage_impl<std::decay_t<ValueType>>>(std::forward<Args>(args)...);
    std::decay_t<ValueType> &value = new_inst->value;
    instance = std::move(new_inst);
    return value;
  }

  template <typename ValueType, typename List, typename... Args>
  std::enable_if_t<
    std::is_constructible_v<std::decay_t<ValueType>, std::initializer_list<List> &, Args...> &&
    std::is_copy_constructible_v<std::decay_t<ValueType>>,
    std::decay_t<ValueType> &
  >
  emplace(std::initializer_list<List> list, Args &&... args) {
    auto new_inst = std::make_unique<storage_impl<std::decay_t<ValueType>>>(list, std::forward<Args>(args)...);
    std::decay_t<ValueType> &value = new_inst->value;
    instance = std::move(new_inst);
    return value;
  }

  void reset() noexcept {
    instance.reset();
  }

  void swap(any &other) noexcept {
    std::swap(instance, other.instance);
  }

  // observers

  bool has_value() const noexcept {
    return static_cast<bool>(instance);
  }

  const std::type_info &type() const noexcept {
    return instance ? instance->type() : typeid(void);
  }

private:
  struct storage_base;

  std::unique_ptr<storage_base> instance;

  struct storage_base {
    virtual ~storage_base() = default;

    virtual const std::type_info &type() const noexcept = 0;
    virtual std::unique_ptr<storage_base> clone() const = 0;
  };

  template <typename ValueType>
  struct storage_impl final : public storage_base {
    template <typename... Args>
    storage_impl(Args &&... args)
      : value(std::forward<Args>(args)...) {}

    const std::type_info &type() const noexcept override {
      return typeid(ValueType);
    }

    std::unique_ptr<storage_base> clone() const override {
      return std::make_unique<storage_impl<ValueType>>(value);
    }

    ValueType value;
  };
};

} // rxy

template <>
inline void std::swap(rxy::any &lhs, rxy::any &rhs) noexcept {
  lhs.swap(rhs);
}

namespace rxy {

class bad_any_cast : public std::exception {
public:
  const char *what() const noexcept {
    return "bad any cast";
  }
};
}