#include "ASTUtils.h"

using namespace ndbl;

std::vector<ASTNode*> ASTUtils::get_adjacent_nodes(const ASTNode* _node, SlotFlags _flags)
{
    std::vector<ASTNode*> result;
    for ( Slot* slot : _node->filter_slots( _flags ) )
    {
        for( const Slot* adjacent : slot->adjacent() )
        {
            result.emplace_back(adjacent->node() );
        }
    }
    return result;
}

ASTNode* ASTUtils::adjacent_node_at(const ASTNode* _node, SlotFlags _flags, u8_t _pos)
{
    if ( Slot* adjacent_slot = _node->find_adjacent_at( _flags, _pos ) )
    {
        return adjacent_slot->node();
    }
    return {};
}