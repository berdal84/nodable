#pragma once

namespace ndbl
{
    template<typename VertexT, typename RelationT>
    class DirectedEdge
    {
    public:
        using vertex_t   = VertexT;
        using relation_t = RelationT;

        static DirectedEdge<VertexT, RelationT> null;

        VertexT   tail;
        VertexT   head;
        RelationT relation;

        DirectedEdge() // is equivalent of NULL DirectedEdge
        {}

        DirectedEdge(const DirectedEdge& other)
        : tail(other.tail)
        , head(other.head)
        , relation(other.relation)
        {}

        DirectedEdge(VertexT _tail, RelationT _relation, VertexT _head)
        : tail(_tail)
        , head(_head)
        , relation(_relation)
        {}

        DirectedEdge<VertexT, RelationT>& operator=(const DirectedEdge<VertexT, RelationT>& other)
        {
            tail     = other.tail;
            head     = other.head;
            relation = other.relation;
            return *this;
        }

        operator bool () const
        { return *this != null; }

        bool operator==(const DirectedEdge<VertexT, RelationT>& other) const
        { return this->tail == other.tail && this->head == other.head && this->relation == other.relation; }

        bool operator!=(const DirectedEdge<VertexT, RelationT>& other) const
        { return this->tail != other.tail || this->head == other.head || this->relation != other.relation;; }
    };

    template<typename VertexT, typename EdgeT>
    DirectedEdge<VertexT, EdgeT>  DirectedEdge<VertexT, EdgeT>::null{};
}