#pragma once

#include <nodable/GraphTraversal.h>
#include <nodable/InstructionNode.h>

namespace Nodable
{
    // forward declarations
    class ScopedCodeBlockNode;
    struct SimpleInstrList;

    enum Register {
        LAST_EVAL = 0,
        LAST_CONDITION,
        //MEM1,
        //MEM2,
        COUNT
    };

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
                      inline Variant*         get_last_eval() { return &m_register[0]; }

    private:
        SimpleInstrList*      compile_program(const ScopedCodeBlockNode* _program);
        void                  compile_node_and_append_to_program(const Node* _node);
        bool                  is_program_valid(const ScopedCodeBlockNode* _program);
        bool                  _stepOver();
        GraphTraversal        m_traversal;
        ScopedCodeBlockNode*  m_program_tree;
        SimpleInstrList*      m_program_compiled;
        Node*                 m_current_node;
        bool                  m_is_program_running;
        bool                  m_is_debugging;
        Variant               m_register[Register::COUNT]; // variants to store temp values
    };


    enum SIType
    {
        Type_UND, // undefined
        Type_EVA, // evaluate
        Type_MOV, // store last eval in a register
        Type_JMP, // always jump
        Type_JNE, // jump only if last condition register is false
        Type_EXI, // stop the program
    };

    class SimpleInstr
    {
    public:

        SimpleInstr(SIType _type, long _line):m_type(_type), m_line(_line) {}
        SimpleInstr(const SimpleInstr& _other) = default;

        std::string to_string()
        {
            std::string result;
            std::string str = std::to_string(m_line);
            while( str.length() < 4 )
                str.append(" ");
            result.append( str );
            result.append( ": " );

            switch ( m_type )
            {
                case Type_EVA:
                {
                    Member* member = mpark::get<Member*>(m_left_h_arg );
                    result.append("EVA ");
                    result.append( "&" + std::to_string((size_t)member) );
                    result.append( " $" + std::to_string( Register::LAST_EVAL ) );
                    break;
                }

                case Type_MOV:
                {
                    result.append("MOV");
                    result.append( " $" + std::to_string( (int)mpark::get<Register>(m_left_h_arg ) ) );
                    result.append( " $" + std::to_string( (int)mpark::get<Register>(m_right_h_arg ) ) );
                    break;
                }

                case Type_JNE:
                {
                    result.append("JNE ");
                    result.append( std::to_string( mpark::get<long>(m_left_h_arg ) ) );
                    break;
                }

                case Type_JMP:
                {
                    result.append("JMP ");
                    result.append( std::to_string( mpark::get<long>(m_left_h_arg ) ) );
                    break;
                }

                case Type_UND:
                {
                    result.append("UND ");
                    break;
                }

                case Type_EXI:
                {
                    result.append("EXI ");
                    break;
                }

            }

            if ( !m_comment.empty() )
            {
                while( result.length() < 50 ) // align on 80th char
                    result.append(" ");
                result.append( "; " );
                result.append( m_comment );
            }
            return result;
        }

        SIType m_type;
        long   m_line;
        mpark::variant<void* , Node*, Member*, long, Register> m_left_h_arg;
        mpark::variant<void* , Node*, Member*, long, Register> m_right_h_arg;
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
         bool         is_over() { assert(get_curr()); return get_curr()->m_type == Type_EXI; }
     private:
         size_t m_cursor_position = 0;
         std::vector<SimpleInstr*> m_instructions;
     };
}


