#include "DataAccess.h"

#include <iostream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "fw/core/assertions.h"
#include "fw/core/Pool.h"
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

	auto writeProperty = [&writer](const Property& property)
	{
		writer.Key(property.get_name().c_str());

        auto& value = *property.value();
    	const fw::type* type = value.get_type();

    	     if (type->is<std::string>() ) writer.String(value);
        else if (type->is<double>() )      writer.Double(value);
        else if (type->is<bool>() )        writer.Bool(value);
    	else                               writer.Null();
	};

    Node* owner = m_owner.get();
    FW_ASSERT( owner != nullptr );
    writer.StartObject();
    {
    	// Write Properties
    	//--------------

    	writer.Key("properties");
    	writer.StartObject();
    	{
		    for(auto& each_property : owner->props )
		    {
		    	writeProperty( each_property );
		    }
		}
	    writer.EndObject();

	    // Write Components
    	//-----------------
    	
    	writer.Key("components");
    	writer.StartObject();
    	{
		    for(ID<Component> component : owner->get_components())
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
    fileName += owner->name;
    fileName += ".json";

    std::ofstream outfile ("saves/" +fileName ,std::ofstream::binary);
    outfile.write (buffer.GetString(), (long)buffer.GetSize());
    outfile.close();

    return true;
}