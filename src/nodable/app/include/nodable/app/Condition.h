#pragma once

namespace ndbl
{
    typedef int Condition;
    enum Condition_ : Condition
    {
        Condition_DISABLE                          = 0,
        Condition_ENABLE_IF_HAS_SELECTION          = 1 << 0,
        Condition_ENABLE_IF_HAS_NO_SELECTION       = 1 << 1,
        Condition_ENABLE_IF_HAS_GRAPH              = 1 << 3,
        Condition_ENABLE                           = Condition_ENABLE_IF_HAS_SELECTION
                                                   | Condition_ENABLE_IF_HAS_NO_SELECTION
                                                   | Condition_ENABLE_IF_HAS_GRAPH,
        Condition_HIGHLIGHTED_IN_GRAPH_EDITOR      = 1 << 4,
        Condition_HIGHLIGHTED_IN_TEXT_EDITOR       = 1 << 5,
        Condition_HIGHLIGHTED                      = Condition_HIGHLIGHTED_IN_GRAPH_EDITOR
                                                   | Condition_HIGHLIGHTED_IN_TEXT_EDITOR,
    };
}// namespace nodable