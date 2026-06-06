#pragma once

#include <string>

namespace tools
{
    struct Task_Manager;
}

namespace ndbl
{
    // forward declarations
    class Graph;
    class Nodlang;

    class Nodable_Headless
    {
    public:
        Nodable_Headless() = default;
        virtual ~Nodable_Headless() = default;
        virtual void        init();
        virtual void        update();
        virtual void        shutdown();
        virtual void        clear();
        bool                should_stop() const;
        virtual std::string& serialize( std::string& out ) const;
        virtual Graph*      parse( const std::string& in );
        Nodlang*            get_language() const;
        Graph*              graph() const;
    protected:
        tools::Task_Manager* m_task_manager{};
        Nodlang*            m_language{};
        bool                m_should_stop{false};
        Graph*              m_graph{};
        std::string         m_source_code;
        bool                m_auto_completion{false};
    };
}

