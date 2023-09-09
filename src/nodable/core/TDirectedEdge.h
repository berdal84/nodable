#pragma once

namespace ndbl
{
    template<typename VertexT, typename RelationT>
    class TDirectedEdge
    {
    public:
        using vertex_t   = VertexT;
        using relation_t = RelationT;

        static TDirectedEdge<VertexT, RelationT> null;

        VertexT   tail;
        VertexT   head;
        RelationT relation;

        TDirectedEdge() // is equivalent of NULL DirectedEdge
        {}

        TDirectedEdge(const TDirectedEdge & other)
        : tail(other.tail)
        , head(other.head)
        , relation(other.relation)
        {}

        TDirectedEdge(VertexT _tail, RelationT _relation, VertexT _head)
        : tail(_tail)
        , head(_head)
        , relation(_relation)
        {}

        TDirectedEdge<VertexT, RelationT>& operator=(const TDirectedEdge<VertexT, RelationT>& other)
        {
            tail     = other.tail;
            head     = other.head;
            relation = other.relation;
            return *this;
        }

        operator bool () const
        { return *this != null; }

        bool operator==(const TDirectedEdge<VertexT, RelationT>& other) const
        { return this->tail == other.tail && this->head == other.head && this->relation == other.relation; }

        bool operator!=(const TDirectedEdge<VertexT, RelationT>& other) const
        { return this->tail != other.tail || this->head == other.head || this->relation != other.relation;; }
    };

    template<typename VertexT, typename EdgeT>
    TDirectedEdge<VertexT, EdgeT> TDirectedEdge<VertexT, EdgeT>::null{};
}