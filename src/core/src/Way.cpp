#include <nodable/core/Way.h>

using namespace ndbl;

std::string ndbl::WayToString(Way _way)
{
    switch(_way)
    {
        case Way_In:    return "In";
        case Way_Out:   return "Out";
        case Way_InOut: return "InOut";
        default:        return "None";
    }
}