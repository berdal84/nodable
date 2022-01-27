#pragma once

#include <nodable/GraphTraversal.h>
#include <nodable/InstructionNode.h>

namespace Nodable
{
    // forward declarations
    class ScopedCodeBlockNode;
    struct SimpleInstrList;

    /**
     * Class to execute a Nodable program.
     */
    class Runner {
    public:
        Runner();
        [[nodiscard]] bool load_program(ScopedCodeBlockNode*);
        void unload_program();
        void run_program();
        bool is_program_over();
        void stop_program();
        void debug_program();
        [[nodiscard]] inline bool             is_program_running() const{ return m_is_program_running; }
        [[nodiscard]] inline bool             is_debugging() const{ return m_is_debugging; }
        [[nodiscard]] inline bool             is_program_stopped() const{ return !m_is_debugging && !m_is_program_running; }
                             bool             step_over();
        [[nodiscard]] inline const Node*      get_current_node() const {return m_current_node; }
                      inline Member*          get_last_eval() { return m_last_eval; }

    private:
        SimpleInstrList*      compile_program(const ScopedCodeBlockNode* _program);
        void                  compile_node_and_append_to_program(const Node* _node);
        bool                  is_program_valid(const ScopedCodeBlockNode* _program);
        bool                  _stepOver();
        GraphTraversal        m_traversal;
        ScopedCodeBlockNode*  m_program;
        SimpleInstrList*      m_compiled_program;
        Node*                 m_current_node;
        Member*               m_last_eval;
        bool                  m_is_program_running;
        bool                  m_is_debugging;
        bool                  m_registers[1]; // TODO: organise a set of flags in a char.
    };


    enum SIType
    {
        Type_UNDEF,
        Type_EVAL,
        Type_STORE,
        Type_JUMP,
        Type_JUMP_IF_FALSE,
        Type_EXIT,
    };

    class SimpleInstr
    {
    public:

        SimpleInstr(SIType _type, long _line):m_type(_type), m_line(_line) {}
        SimpleInstr(const SimpleInstr& _other) = default;

        std::string to_string()
        {
            std::string result;

            result.append( std::to_string(m_line) );
            result.append( ": " );

            switch ( m_type )
            {
                case Type_EVAL:
                {
                    Member* member = mpark::get<Member*>( m_data );
                    result.append("EVAL");
                    result.append( " " );
                    result.append( member->getOwner()->get_class()->get_name() );
                    result.append( "->" );
                    result.append( member->getName() );
                    break;
                }

                case Type_STORE:
                {
                    result.append("STOR");
                    break;
                }

                case Type_JUMP_IF_FALSE:
                {
                    result.append("CJMP");
                    result.append( " " );
                    result.append( std::to_string( mpark::get<long>( m_data ) ) );
                    break;
                }

                case Type_JUMP:
                {
                    result.append("JUMP");
                    result.append( " " );
                    result.append( std::to_string( mpark::get<long>( m_data ) ) );
                    break;
                }

                case Type_UNDEF:
                {
                    result.append("UNDEF");
                    break;
                }

                case Type_EXIT:
                {
                    result.append("EXIT");
                    break;
                }

            }
            return result;
        }

        SIType m_type;
        long   m_line;
        mpark::variant<void* , Node*, Member*, long> m_data;
        std::string m_comment;
     };

    /**
     * Class to store a simple instruction list and navigate forward through it
     */
     class SimpleInstrList
     {
     public:
         SimpleInstrList(){}
         ~SimpleInstrList()
         {
             for( auto each : m_instructions )
                 delete each;
             m_instructions.clear();
         }
         void         advance(long _amount = 1) { m_cursor_position += _amount; }
         SimpleInstr* push_instr(SIType _type)
         {
             SimpleInstr* instr = new SimpleInstr(_type, m_instructions.size());
             m_instructions.emplace_back(instr);
             return instr;
         };
         SimpleInstr* get_curr(){ return m_cursor_position < m_instructions.size() ? m_instructions[m_cursor_position] : nullptr; };
         void         reset_cursor(){ m_cursor_position = 0; };
         long         get_next_line_nb(){ return m_instructions.size(); }
         bool         is_over() { assert(get_curr()); return get_curr()->m_type == Type_EXIT; }
     private:
         size_t m_cursor_position = 0;
         std::vector<SimpleInstr*> m_instructions;
     };
}


