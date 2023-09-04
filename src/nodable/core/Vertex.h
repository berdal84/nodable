#pragma once

namespace ndbl
{
    template<typename DataT>
    class Vertex
    {
    public:
        DataT data;

        Vertex(DataT _data)
        : data( std::move(_data) )
        {}

        DataT& operator->() const
        { return &data; }

        DataT& operator*() const
        { return data; }

        bool operator==(const Vertex& other) const
        {
            return this->data == other.data;
        }
    };

}// namespace fw

