#include "DataAccess.h"

#include <iostream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "fw/core/assertions.h"
#include "core/Node.h"

using namespace ndbl;

REGISTER
{
    fw::registration::push_class<DataAccess>("DataAccess")
            .extends<Component>();
}

bool DataAccess::update()
{

	rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    rapidjson::Document doc;

	auto writeProperty = [&writer](const Property * _value)
	{
		writer.Key(_value->get_name().c_str());

    	const fw::type* property_type = _value->get_type();

    	     if (property_type->is<std::string>() ) writer.String(((const char*)*_value));
        else if (property_type->is<double>() )      writer.Double((double)*_value);
        else if (property_type->is<bool>() )        writer.Double((bool)*_value);
    	else                                         writer.Null();
	};

    FW_ASSERT(m_owner != nullptr);

    writer.StartObject();
    {
    	// Write Properties
    	//--------------

    	writer.Key("properties");
    	writer.StartObject();
    	{
		    for(auto& each : m_owner->props.by_name())
		    {
		    	auto value = each.second;

		    	writeProperty(value);
		    }
		}
	    writer.EndObject();

	    // Write Components
    	//-----------------
    	
    	writer.Key("components");
    	writer.StartObject();
    	{
		    for(const auto& [hash, component] : m_owner->components)
		    {
		    	writer.Key(component->get_type()->get_name());
		    	writer.StartObject();

		    	// TODO: Save component: how to?

			    writer.EndObject();
		    }
		}
	    writer.EndObject();

	}
    writer.EndObject();

    std::string fileName("node_");
    fileName += m_owner->name;
    fileName += ".json";

    std::ofstream outfile ("saves/" +fileName ,std::ofstream::binary);
    outfile.write (buffer.GetString(), (long)buffer.GetSize());
    outfile.close();

    return true;
}