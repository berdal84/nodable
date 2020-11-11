#pragma once

#include <memory>
#include <string>
#include <utility>
#include <Node/Variable.h>
#include <Language/Operator.h>
#include <Language/Function.h>

namespace Nodable
{
    class Language;

    /**
     * The only role of this class is to create Node (or derived) instances.
     */
    class NodeFactory
    {
    public:

        enum class Mode {
            WITH_GUI,
            HEADLESS,
            DEFAULT = WITH_GUI
        };

        NodeFactory( std::shared_ptr<const Language>  _language, Mode _mode = Mode::DEFAULT):
            language(std::move(_language)),
            mode(_mode)
        {}
        ~NodeFactory() = default;

        std::shared_ptr<Variable>	newVariable(std::string = "");
        std::shared_ptr<Variable>   newNumber(double = 0);
        std::shared_ptr<Variable>	newNumber(const char*);
        std::shared_ptr<Variable>	newString(const char*);
        std::shared_ptr<Node>       newBinOp(std::shared_ptr<const Operator>);
        std::shared_ptr<Node>       newUnaryOp(std::shared_ptr<const Operator>);
        std::shared_ptr<Wire>       newWire();
        std::shared_ptr<Node>       newFunction(std::shared_ptr<const Function> _proto);

    private:
        std::shared_ptr<const Language> language;
        Mode mode;
    };
}