#include "DataAccess.h"

#include <iostream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "tools/core/assertions.h"
#include "tools/core/memory/Pool.h"

#include "Node.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<DataAccess>("DataAccess")
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
    	const type* type = value.get_type();

    	     if (type->is<std::string>() ) writer.String((const char*)value);
        else if (type->is<double>() )      writer.Double((double)value);
        else if (type->is<bool>() )        writer.Bool((bool)value);
    	else                               writer.Null();
	};

    Node* owner = m_owner.get();
    ASSERT( owner != nullptr );
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
		    for(PoolID<Component> component : owner->get_components())
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