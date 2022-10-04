#include <nodable/core/DataAccess.h>

#include <iostream>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <nodable/core/Node.h>
#include <nodable/core/assertions.h>

using namespace ndbl;

REGISTER
{
    registration::push_class<DataAccess>("DataAccess")
            .extends<Component>();
}

bool DataAccess::update()
{

	rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Document doc;

	auto writeMember = [&writer](const Member* _value)
	{
		writer.Key(_value->get_name().c_str());

    	type t = _value->get_type();

    	     if ( t == type::get<std::string>() ) writer.String(((const char*)*_value));
        else if ( t == type::get<double>() )      writer.Double((double)*_value);
        else if ( t == type::get<bool>() )        writer.Double((bool)*_value);
    	else                                         writer.Null();
	};

    NODABLE_ASSERT(get_owner() != nullptr);

    Node* owner = get_owner();

    writer.StartObject();
    {
    	// Write Members
    	//--------------

    	writer.Key("members");
    	writer.StartObject();
    	{
		    for(auto& each : owner->props()->by_name())
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
		    for(auto& eachComponent : owner->get_components())
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

    std::string fileName("Entity_" + std::to_string((size_t) get_owner()) + ".json");

    std::ofstream outfile ("saves/" +fileName ,std::ofstream::binary);
    outfile.write (buffer.GetString(),buffer.GetSize());
    outfile.close();

    return true;
}