#include <nodable/DataAccess.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <nodable/Node.h>
#include <iostream>
#include <fstream>

using namespace Nodable;

bool DataAccess::update()
{

	rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Document doc;

	auto writeMember = [&writer](const Member* _value)
	{
		writer.Key(_value->getName().c_str());

    	switch( _value->getType())
    	{
        case Type_String :
    		writer.String( ((std::string)*_value).c_str());
    		break;

        case Type_Double :
    		writer.Double((double)*_value);
    		break;
    		
        case Type_Boolean:
    		writer.Bool((bool)*_value);
    		break;
    	default:
    		writer.Null();
    		break;
    	}
	};

    NODABLE_ASSERT(getOwner() != nullptr);

    Node* owner = getOwner();

    writer.StartObject();
    {
    	// Write Members
    	//--------------

    	writer.Key("members");
    	writer.StartObject();
    	{
		    for(auto& each : owner->getProps()->getMembers())
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

		    	// TODO: use mirror to serialize members

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

    return true;
}