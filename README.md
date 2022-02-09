# mirror
A simple C++ reflection lib. (Single header only!)

## Define A Reflection
```
#include "mirror.h"

struct X { int a; int b; };
// mirror::Reflection<X> is defined by following sentence.
MIRROR_REFLECTION_DEFINE(X, a, b);

void test() {
  X x{1, 2};

  // X&
  mirror::Reflection<X>::ForeachField(Printer(), x);  // a = 1, b = 2, 

  std::cout << std::endl;
  mirror::Reflection<X>::ForeachField(Printer(), X(x));  // a = 1, b = 2, 
  std::cout << std::endl;
  mirror::Reflection<X>::Field<1>::get(x) = 42;
  mirror::Reflection<X>::ForeachField(Printer(), x);  // a = 1, b = 42, 
  std::cout << std::endl;
}
```

## Foreach API

You should define a functor for every field processor, such as:

```
// A Simple foreach functor
struct Printer {
  template <typename T> void run(const char* name, const T& x) {
    std::cout << name << " = " << x << ", ";
  }
};

// A Simple foreach functor, modify fields.
struct Setter {
  Setter(int val) : val_(val) {}
  template <typename T> void run(const char* name, T& x) {
    x = val_;
  }
  int val_;
};
```

Use functor like this:

```
X x{1, 2};

using Reflection = mirror::Reflection<X>;

// ForeachField

// Run ForeachField(Functor, X&)
Reflection::ForeachField(Printer(), x);  // a = 1, b = 2,
std::cout << std::endl;

// Run ForeachField(Functor, const X&)
Reflection::ForeachField(Printer(), X(x));  // a = 1, b = 2,
std::cout << std::endl;

// Modify field in ForeachField
Reflection::ForeachField(Setter(886), x);  // x.a = 886, x.b = 886
Reflection::ForeachField(Printer(), x);  // a = 886, b = 886,
std::cout << std::endl;
```

## Field API

If you want to get field by field id, you can use field api.

```
// Some field api.

// Reflection::field_name<field_id>()
std::cout << Reflection::field_name<0>() << std::endl;  // a

// Reflection::FieldType<>
std::cout << std::is_same<short, Reflection::FieldType<0>>::value << std::endl;  // true

// Reflection::get<field_id>(X&)
Reflection::get<0>(x) = 42;
Reflection::ForeachField(Printer(), x);  // a = 42, b = 886,
std::cout << std::endl;

// Reflection::get<field_id>(const X&)
std::cout << Reflection::get<0>(X(x)) << std::endl;  // 42
```

## Class-Style Field API

If you may want to make your owned template class:

```
// You can use Reflection::Field<field_id>, if you want.

// Reflection::Field<field_id>::name()
std::cout << Reflection::Field<1>::name() << std::endl;  // b

// Reflection::Field<field_id>::type
std::cout << std::is_same<int, Reflection::Field<1>::type>::value << std::endl;  // true

// Reflection::Field<field_id>::get(X&)
Reflection::Field<1>::get(x) = 84;
Reflection::ForeachField(Printer(), x);  // a = 42, b = 84,
std::cout << std::endl;

// Reflection::Field<field_id>::get(const X&)
std::cout << Reflection::Field<1>::get(X(x)) << std::endl;  // 84

// Reflection::FieldStdTuple == std::tuple<Reflection::Field<0>, Reflection::Field<1>...>
std::cout << std::is_same<Reflection::FieldStdTuple,
  std::tuple<Reflection::Field<0>, Reflection::Field<1>>>::value << std::endl;  // true
```
