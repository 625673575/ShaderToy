#pragma once
#include "Node.h"
#include "Externals/dear_imgui/imgui.h"
namespace ShaderNodeEditor {
#define DECLAR_NODE_STATIC_VAR \
    static const char* Name;\
    static const char* Desc;\
    static const ImColor Color;

#define INTERPRET_BEGIN(INPUT_COUNT,OUTPUT_COUNT)DECLAR_NODE_STATIC_VAR\
    static const std::array<NodeInputPinMeta, INPUT_COUNT> InputPinMeta;\
    static const std::array<NodeOutputPinMeta, OUTPUT_COUNT> OutputPinMeta;\
    const NodeInputPinMeta& GetInputMetaInfo(size_t i)const override {return InputPinMeta[i];}\
    const NodeOutputPinMeta& GetOutputMetaInfo(size_t i)const override {return OutputPinMeta[i];}\
    std::string Interpret()override\
{ assert(Id.params.size() == INPUT_COUNT);

#define INTERPRET_END }

    extern size_t VariableCount;

    struct FloatNumberNode :public INodeInterpreter {
        INTERPRET_BEGIN(0, 1)
            return "%0";
        INTERPRET_END
    };

    struct TimeNode :public INodeInterpreter {
        INTERPRET_BEGIN(0,1)
            return "_Time";
        INTERPRET_END
    };

    struct SinNode :public INodeInterpreter {
        INTERPRET_BEGIN(1,1)
            return "sin({0})";
        INTERPRET_END
    };

    struct AddNode :public INodeInterpreter {
        INTERPRET_BEGIN(2,1)
            return "{0} + {1}";
        INTERPRET_END
    };

    struct MultiplyNode :public INodeInterpreter {
        INTERPRET_BEGIN(2,1)
            return "{0} * {1}";
        INTERPRET_END
    };
}
