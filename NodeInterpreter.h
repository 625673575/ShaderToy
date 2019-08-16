#pragma once
#include "Node.h"
#include "Externals/dear_imgui/imgui.h"
namespace ShaderNodeEditor {
#define DECLAR_NODE_STATIC_VAR \
    static const char* Name;\
    static const char* Desc;\
    static const char* Category;\
    static const ImColor Color;

#define INTERPRET_BEGIN(INPUT_COUNT,OUTPUT_COUNT)DECLAR_NODE_STATIC_VAR\
    static const std::array<NodeInputPinMeta, INPUT_COUNT> InputPinMeta;\
    static const std::array<NodeOutputPinMeta, OUTPUT_COUNT> OutputPinMeta;\
    const NodeInputPinMeta& GetInputMetaInfo(size_t i)const override {return InputPinMeta[i];}\
    const NodeOutputPinMeta& GetOutputMetaInfo(size_t i)const override {return OutputPinMeta[i];}\
    const char* GetCategory()const override{return Category;}\
    std::string Interpret()override\
{ assert(Id.params.size() == INPUT_COUNT);

#define INTERPRET_END }
    extern size_t VariableCount;

    struct FloatConstantNode :public IConstantInterpreter {
        INTERPRET_BEGIN(0, 1)
            return "%C";
        INTERPRET_END
    };

    struct FloatVariableNode :public IVariableInterpreter {
        INTERPRET_BEGIN(0, 1)
            return "%V";
        INTERPRET_END
    };

    struct TimeNode :public IOperationInterpreter {
        INTERPRET_BEGIN(0, 1)
            return "_Time";
        INTERPRET_END
    };

    struct SinNode :public IOperationInterpreter {
        INTERPRET_BEGIN(1, 1)
            return "sin({0})";
        INTERPRET_END
    };

    struct AddNode :public IOperationInterpreter {
        INTERPRET_BEGIN(2, 1)
            return "({0} + {1})";
        INTERPRET_END
    };

    struct SubNode :public IOperationInterpreter {
        INTERPRET_BEGIN(2, 1)
            return "({0} - {1})";
        INTERPRET_END
    };

    struct MultiplyNode :public IOperationInterpreter {
        INTERPRET_BEGIN(2, 1)
            return "({0} * {1})";
        INTERPRET_END
    };

    struct DivideNode :public IOperationInterpreter {
        INTERPRET_BEGIN(2, 1)
            return "({0} / {1})";
        INTERPRET_END
    };
}
