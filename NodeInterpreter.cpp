#include "NodeInterpreter.h"

namespace ShaderNodeEditor {
    size_t VariableCount = 1;

#define NODE_PROPERTY(CLASS,NAME,CATEGORY,DESC,COLOR)\
    const char* CLASS::Name = #NAME;\
    const char* CLASS::Desc = #DESC;\
    const char* CLASS::Category = #CATEGORY;\
    const ImColor CLASS::Color = ImVec4 COLOR
    //

    NODE_PROPERTY(FloatConstantNode, Float_Constant, Constant , const float type, (.0f, .0f, .0f, 1.0f));
    const std::array<NodeInputPinMeta, 0> FloatConstantNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> FloatConstantNode::OutputPinMeta = { "Float" };

    NODE_PROPERTY(FloatVariableNode, Float_Variable, Variable , variable float type, (.0f, .0f, .0f, 1.0f));
    const std::array<NodeInputPinMeta, 0> FloatVariableNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> FloatVariableNode::OutputPinMeta = { "Float" };

    NODE_PROPERTY(TimeNode, Time, Misc , time since the game started, (.0f, .0f, .0f, 1.0f));
    const std::array<NodeInputPinMeta, 0> TimeNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> TimeNode::OutputPinMeta = {"Time"};

    NODE_PROPERTY(SinNode, Sin, Math , return sin(x), (.0f, .0f, 1.0f, 1.0f));
    const std::array<NodeInputPinMeta, 1> SinNode::InputPinMeta = { NodeInputPinMeta{"",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> SinNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(AddNode, Add, Math, return x + y, (1.0f, 1.0f, 1.0f, 1.0f));
    const std::array<NodeInputPinMeta, 2> AddNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> AddNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(SubNode, Sub, Math, return x - y, (1.0f, 1.0f, 1.0f, 1.0f));
    const std::array<NodeInputPinMeta, 2> SubNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> SubNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(MultiplyNode, Multiply, Math, return x * y, (1.0f, .0f, 0.5f, 1.0f));
    const std::array<NodeInputPinMeta, 2> MultiplyNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> MultiplyNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(DivideNode, Divide, Math, return x / y, (1.0f, .0f, 0.5f, 1.0f));
    const std::array<NodeInputPinMeta, 2> DivideNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> DivideNode::OutputPinMeta = { "Result" };

}
