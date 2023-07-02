#include <ndbl/core/DataAccess.h>

#include <iostream>
#include <fstream>

#include "fw/core/assertions.h"
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <ndbl/core/Node.h>


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

    	fw::type t = _value->get_type();

    	     if ( t == fw::type::get<std::string>() ) writer.String(((const char*)*_value));
        else if ( t == fw::type::get<double>() )      writer.Double((double)*_value);
        else if ( t == fw::type::get<bool>() )        writer.Double((bool)*_value);
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
		    for(auto& each : m_owner->props()->by_name())
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
		    for(const auto& [hash, component] : m_owner->components())
		    {
		    	writer.Key(component->get_type().get_name());
		    	writer.StartObject();

		    	// TODO: Save component: how to?

			    writer.EndObject();
		    }
		}
	    writer.EndObject();

	}
    writer.EndObject();

    std::string fileName("node_");
    fileName += m_owner->get_name();
    fileName += ".json";

    std::ofstream outfile ("saves/" +fileName ,std::ofstream::binary);
    outfile.write (buffer.GetString(), (long)buffer.GetSize());
    outfile.close();

    return true;
}