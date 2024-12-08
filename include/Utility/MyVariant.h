#ifndef MYVARIANT_H
#define MYVARIANT_H
#include "../MyApp.h"
#include "rttr/registration.h"

RTTR_REGISTRATION { rttr::registration::class_<MyApp>("MyApp"); }
// class MyVariant {
//     enum Type{
//         VAR_INT32,
//         VAR_FLOAT,
//         VAR_STRING,
//         VAR_BOOL,
//         VAR_VECTOR2,
//         VAR_VECTOR3,
//         VAR_VECTOR4,
//         VAR_MAT3,
//         VAR_MAT4,
//         VAR_COUNT
//     };
//     MyVariant() = default;
//     MyVariant(const MyVariant& other) = default;
// private:
//     std::shared_ptr<void> m_data;
//     Type m_type;

// };

#endif// MYVARIANT_H