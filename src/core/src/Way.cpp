#include <nodable/Way.h>

using namespace Nodable::core;

std::string Nodable::core::WayToString(Way _way)
{
    switch(_way)
    {
        case Way_In:    return "In";
        case Way_Out:   return "Out";
        case Way_InOut: return "InOut";
        default:        return "None";
    }
}