#include "DataAccess.h"
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include "Entity.h"
#include <iostream>
#include <fstream>

using namespace Nodable;

void DataAccess::update()
{

	rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Document doc;

	auto writeMember = [&writer](const Member* _value)
	{
		writer.Key(_value->getName().c_str());

    	switch( _value->getType())
    	{
    		case (Type_String) :
    		{
    			writer.String(_value->getValueAsString().c_str());
    			break;
    		}

    		case (Type_Number) :
    		{
    			writer.Double(_value->getValueAsNumber());
    			break;
    		}
    		
    		case (Type_Boolean) :
    		{
    			writer.Bool(_value->getValueAsBoolean());
    			break;
    		}
    		default:
    		{
    			writer.Null();
    			break;
    		}
    	}
	};

    NODABLE_ASSERT(getOwner() != nullptr);

    Entity* owner = getOwner();

    writer.StartObject();
    {
    	// Write Members
    	//--------------

    	writer.Key("members");
    	writer.StartObject();
    	{
		    for(auto& each : owner->getMembers())
		    {
		    	auto value = each.second;

		    	writeMember(value);
		    }
		}
	    writer.EndObject();

	    // Write Components
    	//-----------------
    	
    	writer.Key("components");
    	writer.StartObject();
    	{
		    for(auto& eachComponent : owner->getComponents())
		    {
		    	writer.Key   (eachComponent.first.c_str());
		    	writer.StartObject();

		    	for(auto& each : eachComponent.second->getMembers())
			    {
			    	auto value = each.second;
			    	writeMember(value);
			    }

			    writer.EndObject();
		    }
		}
	    writer.EndObject();

	}
    writer.EndObject();

    std::string fileName("Entity_" + std::to_string((size_t)getOwner()) + ".json");

    std::ofstream outfile ("saves/" +fileName ,std::ofstream::binary);
    outfile.write (buffer.GetString(),buffer.GetSize());
    outfile.close();
}