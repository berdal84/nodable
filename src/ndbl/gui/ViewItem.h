#pragma once
#include <utility>
#include <variant>
#include <cstddef>

namespace ndbl
{
    // forward declarations
    class SlotView;
    class NodeView;

    // possible items
    using  EdgeViewItem = std::pair<SlotView*, SlotView*>;
    using  SlotViewItem = SlotView*;
    using  NodeViewItem = NodeView*;

    // ViewItem is able to hold different ViewItems
    class ViewItem
    {
    public:
        template<typename T>
        bool is() const
        { return std::holds_alternative<T>(data); }

        template<typename T>
        T get() const
        { return std::get<T>(data); }

        bool empty() const
        { return std::holds_alternative<Empty>(data); }

        template<typename T>
        ViewItem& operator=(T value)
        { data = value; return *this; }

        size_t index() const
        { return data.index(); }
    private:
        struct Empty {};
        std::variant<Empty, SlotViewItem, EdgeViewItem, NodeViewItem> data = Empty{};
    };
}