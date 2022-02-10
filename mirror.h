#ifndef MIRROR_H_
#define MIRROR_H_

#include <type_traits>
#include <iostream>
#include <tuple>

// Some utils
#define MIRROR_CAT_I(x, y) x ## y
#define MIRROR_CAT(x, y) MIRROR_CAT_I(x, y)
#define MIRROR_GET_FIRST(N, ...) N
#define MIRROR_GET_SECOND(N, M, ...) M
#define MIRROR_CHECK(...) MIRROR_GET_SECOND(__VA_ARGS__, 0)
#define MIRROR_PROBE(x) x, 1

#define MIRROR_DEFER_EMPTY()
#define MIRROR_DEFER(id) id MIRROR_DEFER_EMPTY()

#define MIRROR_EMPTY_LIST(...) MIRROR_CHECK(MIRROR_CAT(MIRROR_EMPTY_LIST_,\
    MIRROR_CAT(MIRROR_GET_FIRST(__VA_ARGS__, _EMPTY_), \
      MIRROR_GET_SECOND(__VA_ARGS__, _EMPTY_))))
#define MIRROR_EMPTY_LIST__EMPTY_ MIRROR_PROBE()

#define MIRROR_EXPAND1(...) __VA_ARGS__
#define MIRROR_EXPAND2(...) MIRROR_EXPAND1(MIRROR_EXPAND1(MIRROR_EXPAND1(__VA_ARGS__)))
#define MIRROR_EXPAND3(...) MIRROR_EXPAND2(MIRROR_EXPAND2(MIRROR_EXPAND2(__VA_ARGS__)))
#define MIRROR_EXPAND4(...) MIRROR_EXPAND3(MIRROR_EXPAND3(MIRROR_EXPAND3(__VA_ARGS__)))
#define MIRROR_EXPAND5(...) MIRROR_EXPAND4(MIRROR_EXPAND4(MIRROR_EXPAND4(__VA_ARGS__)))
#define MIRROR_EXPAND6(...) MIRROR_EXPAND5(MIRROR_EXPAND5(MIRROR_EXPAND5(__VA_ARGS__)))
#define MIRROR_EXPAND(...) MIRROR_EXPAND6(__VA_ARGS__)

#define MIRROR_FOR_EACH_IMPL(METHOD, x, ...) METHOD(x) \
  MIRROR_CAT(MIRROR_FOR_EACH_, MIRROR_EMPTY_LIST(__VA_ARGS__))(METHOD, __VA_ARGS__)
#define MIRROR_FOR_EACH_1(...)
#define MIRROR_FOR_EACH_0(...) MIRROR_DEFER(MIRROR_FOR_EACH_I)()(__VA_ARGS__)
#define MIRROR_FOR_EACH_I() MIRROR_FOR_EACH_IMPL
#define MIRROR_FOR_EACH(METHOD, ...) MIRROR_EXPAND(MIRROR_FOR_EACH_IMPL(METHOD, __VA_ARGS__))

namespace mirror {

struct ReflectionDetailsGetter {};
struct ReflectionDetailsError {
  template<typename T>
  ReflectionDetailsError(const T&){}
};
void operator<<(ReflectionDetailsGetter, ReflectionDetailsError);

template<typename T>
struct Enable {
  using Details = decltype(std::declval<ReflectionDetailsGetter>() << std::declval<T>());
  static constexpr bool value = !(std::is_same<Details, void>::value);
};

template<typename T>
struct ReflectionDetailsImpl {
  using Details = decltype(std::declval<ReflectionDetailsGetter>() << std::declval<T>());
  static_assert(!std::is_same<Details, void>::value,
      "You should use MIRROR_REFLECTION_DEFINE(STRUCT, FIELD1, FIELD2...)"
      " to define a relection.");
};

template<typename T>
struct ReflectionDetails : public ReflectionDetailsImpl<T>::Details { };

template<typename T, typename... FieldList>
struct ReflectionTuple {
  using FieldStdTuple = std::tuple<FieldList...>;

  template<int id>
  using Field = typename std::tuple_element<id, FieldStdTuple>::type;

  template<int id>
  using FieldType = typename Field<id>::type;

  template<int id>
  static constexpr const char* field_name() { return Field<id>::name(); }

  template<int id>
  static FieldType<id>& get(T& val) {
    return Field<id>::get(val);
  }

  template<int id>
  static const FieldType<id>& get(const T& val) {
    return Field<id>::get(val);
  }

  template <typename Functor>
  static void ForeachField(Functor functor, T& val) {
    ForeachFieldHelper<Functor, FieldList...>(functor, val);
  }

  template <typename Functor>
  static void ForeachFieldHelper(Functor functor, T& val) {
  }

  template <typename Functor, typename FirstField, typename... LastField>
  static void ForeachFieldHelper(Functor functor, T& val) {
    functor.template run<typename FirstField::type>(FirstField::name(), FirstField::get(val));
    ForeachFieldHelper<Functor, LastField...>(functor, val);
  }

  template <typename Functor>
  static void ForeachField(Functor functor, const T& val) {
    ForeachFieldHelper<Functor, FieldList...>(functor, val);
  }

  template <typename Functor>
  static void ForeachFieldHelper(Functor functor, const T& val) {
  }

  template <typename Functor, typename FirstField, typename... LastField>
  static void ForeachFieldHelper(Functor functor, const T& val) {
    functor.template run<typename FirstField::type>(FirstField::name(), FirstField::get(val));
    ForeachFieldHelper<Functor, LastField...>(functor, val);
  }
};

template <typename T>
using Reflection = typename ReflectionDetails<T>::Tuple;

}

#define MIRROR_REFLECTION_DEFINE_FIELD(FIELDNAME) \
  struct Field_##FIELDNAME { \
    using type = std::remove_reference<decltype(std::declval<T>().FIELDNAME)>::type; \
    static constexpr const char* name() { return #FIELDNAME; }\
    static constexpr const type& get(const T& val) { return val.FIELDNAME; }\
    static constexpr type& get(T& val) { return val.FIELDNAME; }\
  };

#define MIRROR_REFLECTION_TUPLE_HELPER(FIELDNAME) , Field_##FIELDNAME

#define MIRROR_REFLECTION_DEFINE(STRUCT, ...) \
template <typename T> struct MirrorDetails; \
template<> struct MirrorDetails<STRUCT> { \
  using T = STRUCT; \
  MIRROR_FOR_EACH(MIRROR_REFLECTION_DEFINE_FIELD, __VA_ARGS__); \
  using Tuple = ::mirror::ReflectionTuple<T MIRROR_FOR_EACH(MIRROR_REFLECTION_TUPLE_HELPER, __VA_ARGS__)>; \
}; \
MirrorDetails<STRUCT> operator<<(::mirror::ReflectionDetailsGetter, const STRUCT&);

#endif
