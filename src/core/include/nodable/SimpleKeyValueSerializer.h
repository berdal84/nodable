#pragma once
#include <string>
#include <nodable/Nodable.h>

namespace mirror
{
    /**
     * Draft of a simple class able to serialize to this kind of format (using mirror library)
     *
     * key1=value1
     * key2=value2
     * etc.
     *
     * Works only with: int32, float, bool and std::string, ex:
     * m_myMember=50.0000
     * m_myOtherMember="Hello, World."
     * m_myBool=false
     */
    class SimpleKeyValueSerializer
    {
    public:
        SimpleKeyValueSerializer(){}
        ~SimpleKeyValueSerializer(){}

        template<class T>
        void serialize(T* _object, std::string& _out)
        {
            std::vector<mirror::ClassMember*> members;
            mirror::Class* clss = _object->getClass();
            clss->getMembers(members, false);
            for( auto& each : members )
            {
                void* mPointer = each->getInstanceMemberPointer(_object);
                serialize( mPointer, each->getName(), each->getType(), _out);
            }
        }

        void serialize(void* _object, const char* _name, TypeDesc* _typeDesc, std::string& _out)
        {
            _out.append(_name);
            _out.append("=");

            switch ( _typeDesc->getType() )
            {
                case mirror::Type_float:
                    _out.append( std::to_string( *reinterpret_cast<float*>(_object) ) );
                    break;
                case mirror::Type_int32:
                    _out.append( std::to_string( *reinterpret_cast<int*>(_object) ) );
                    break;
                case mirror::Type_Pointer:
                {
                    const PointerTypeDesc *pointerTypeDesc = static_cast<const PointerTypeDesc *>(_typeDesc);
                    const TypeDesc *subType = pointerTypeDesc->getSubType();
                    if (subType->hasFactory())
                    {
                        void **pointerPtr = reinterpret_cast<void **>(_object);
                        bool isValidPointer;

                        isValidPointer = *pointerPtr != nullptr;
                        if (isValidPointer)
                        {
                            if (subType->getType() == Type_char)
                            {
                                _out.append("\"");
                                _out.append(reinterpret_cast<const char *>(*pointerPtr));
                                _out.append("\"");
                            }
                            else
                            {
                                LOG_ERROR("SimpleKeyValueSerializer", "Pointers non char* can't be serialized. Not implemented yet.");
                                NODABLE_ASSERT(false);
                            }
                        }
                    }
                }
                    break;
                case mirror::Type_bool:
                    _out.append( *reinterpret_cast<bool*>(_object) ? "true" : "false" );
                    break;
                default:
                    NODABLE_ASSERT(false); // Not yet implemented
            }
            _out.append("\n");
        }
    };
}