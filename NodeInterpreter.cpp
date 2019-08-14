#include "NodeInterpreter.h"

namespace ShaderNodeEditor {
    size_t VariableCount = 1;

#define NODE_PROPERTY(CLASS, NAME,DESC,COLOR)\
    const char* CLASS::Name = #NAME;\
    const char* CLASS::Desc = #DESC;\
    const ImColor CLASS::Color = ImVec4 COLOR
    //
    NODE_PROPERTY(FloatNumberNode, Time, time since the game started, (.0f, .0f, .0f, 1.0f));
    const std::array<NodeInputPinMeta, 0> FloatNumberNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> FloatNumberNode::OutputPinMeta = { "Float" };

    NODE_PROPERTY(TimeNode, Time, time since the game started, (.0f, .0f, .0f, 1.0f));
    const std::array<NodeInputPinMeta, 0> TimeNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> TimeNode::OutputPinMeta = {"Time"};

    NODE_PROPERTY(SinNode, Sin, return sin(x), (.0f, .0f, 1.0f, 1.0f));
    const std::array<NodeInputPinMeta, 1> SinNode::InputPinMeta = { NodeInputPinMeta{"",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> SinNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(AddNode, Add, return x + y, (1.0f, 1.0f, 1.0f, 1.0f));
    const std::array<NodeInputPinMeta, 2> AddNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> AddNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(MultiplyNode, Multiply, return x * y, (1.0f, .0f, 0.5f, 1.0f));
    const std::array<NodeInputPinMeta, 2> MultiplyNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> MultiplyNode::OutputPinMeta = { "Result" };
}
