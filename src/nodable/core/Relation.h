#pragma once
#include "fw/core/reflection/reflection"

namespace ndbl
{
    enum Relation : i8_t
    {
        // Primary relations

        NONE = 0, // has no complementary
        CHILD_PARENT,
        NEXT_PREVIOUS,
        WRITE_READ,

        PRIMARY_COUNT,
        PRIMARY_BEGIN = CHILD_PARENT,

        // Secondary (complementary)

        PARENT_CHILD  = -CHILD_PARENT,
        PREVIOUS_NEXT = -NEXT_PREVIOUS,
        READ_WRITE    = -WRITE_READ,

    };

    R_ENUM(Relation)
    R_ENUM_VALUE(NONE)
    R_ENUM_VALUE(CHILD_PARENT)
    R_ENUM_VALUE(NEXT_PREVIOUS)
    R_ENUM_VALUE(WRITE_READ)
    R_ENUM_VALUE(PARENT_CHILD)
    R_ENUM_VALUE(PREVIOUS_NEXT)
    R_ENUM_VALUE(READ_WRITE)
    R_ENUM_END

    static bool is_primary(Relation relation)
    { return relation >= Relation::PRIMARY_BEGIN && relation < Relation::PRIMARY_COUNT; }

    static Relation complement(Relation relation)
    { return static_cast<Relation>(-relation); }
} // namespace ndbl