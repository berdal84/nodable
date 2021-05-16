#include "Way.h"

using namespace Nodable::core;

std::string WayToString(Way _way)
{
    switch(_way)
    {
        case Way_In:    return "In";
        case Way_Out:   return "Out";
        case Way_InOut: return "InOut";
        default:        return "None";
    }
}