#pragma once

#include "tools/core/Signals.h"

#define BASE_COMPONENT_CLASS( OWNER_CLASS_NAME ) \
    BASE_COMPONENT_FOR_EX( OWNER_CLASS_NAME, OWNER_CLASS_NAME##Component)

#define BASE_COMPONENT_CLASS_EX( OWNER_CLASS_NAME, COMPONENT_CLASS_NAME )\
    class COMPONENT_CLASS_NAME \
    {\
    public:\
        using owner_t = OWNER_CLASS_NAME;\
    public:\
        SIGNAL(on_reset_owner); \
        virtual ~COMPONENT_CLASS_NAME(){}\
        bool         has_owner() const         { return m_owner != nullptr; }\
        owner_t*     owner() const             { return m_owner; }\
        void         reset_owner(owner_t* owner = nullptr)\
        {\
            m_owner = owner;\
            on_reset_owner.emit();\
        }\
    protected:\
        owner_t* m_owner{};                                                  \
    private:                                                                 \
        __BASE_COMPONENT_CLASS_CUSTOM_IMPL


#define __BASE_COMPONENT_CLASS_CUSTOM_IMPL( ... ) \
    /** custom implementation */ \
    __VA_ARGS__                                   \
    /** custom implementation (end) */ \
    }
