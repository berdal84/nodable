#include "DataAccess.h"

#include <iostream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "tools/core/assertions.h"
#include "tools/core/memory/memory.h"

#include "Node.h"

using namespace ndbl;
using namespace tools;

REFLECT_STATIC_INIT
{
    StaticInitializer<DataAccess>("DataAccess")
            .extends<NodeComponent>();
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

    ASSERT( m_owner != nullptr );
    writer.StartObject();
    {
    	// Write Properties
    	//--------------

    	writer.Key("properties");
    	writer.StartObject();
    	{
		    for(auto each_property : m_owner->props )
		    {
		    	writeProperty( *each_property );
		    }
		}
	    writer.EndObject();

	    // Write Components
    	//-----------------
    	
    	writer.Key("components");
    	writer.StartObject();
    	{
		    for(auto component : m_owner->get_components())
		    {
		    	writer.Key(component->get_class()->get_name());
		    	writer.StartObject();

		    	// TODO: implementation

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