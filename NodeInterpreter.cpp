#include "NodeInterpreter.h"

namespace ShaderNodeEditor {
    size_t VariableCount = 1;

    const char* ShaderNodeEditor::xyzw[4] = { "x","y","z","w" };
    const char* ShaderNodeEditor::XYZW[4] = { "x","y","z","w" };
    const char* ShaderNodeEditor::rgba[4] = { "r","g","b","a" };
    const char* ShaderNodeEditor::RGBA[4] = { "R","G","B","A" };

    PinValueType INodeInterpreter::MergeOutputType(const std::vector<PinValueType>& p)
    {

        if (p.empty())return PinValueType::None;
        if (p.size() == 1)return p.front();
        std::vector<PinValueType> c;
        std::unique_copy(p.begin(), p.end(), std::back_inserter(c));
        if (c.size() == 1)return c.front();
        if (c.size() == 2) {
            if (c[0] == PinValueType::Float) {
                return c[1];
            }
            if (c[1] == PinValueType::Float) {
                return c[0];
            }
        }
        return PinValueType::None;
    }
    bool INodeInterpreter::drawVectorVariable(int size, float* data, float min_val , float max_val )
    {
        bool changed = false;
        for (int i = 0; i < size; i++) {
            ImGui::PushItemWidth(80);
            changed |= ImGui::DragFloat(
                xyzw[i],
                &data[i],
                0.01f,
                min_val,
                max_val);
            ImGui::PopItemWidth();
        }
        return changed;
    }
    bool INodeInterpreter::drawDragIntVariable(const char* label, int* value, int min_val, int max_val)
    {
        return ImGui::DragInt(label, value, 1.0, min_val, max_val);
    }
    bool INodeInterpreter::drawDragFloatVariable(const char* label, float* value, float min_val, float max_val)
    {
        return ImGui::DragFloat(label, value, 1.0, min_val, max_val);
    }
    bool INodeInterpreter::drawUpdownIntVariable(const char* label, int* value, int min_val, int max_val)
    {
        const int step = 1;
        bool r = ImGui::InputScalar(label, ImGuiDataType_S64, value, &step, NULL, "%d");
        if (r && *value > max_val) { *value = max_val; r = false; }
        if (r && *value < min_val) { *value = min_val; r = false; }
        return r;
    }
    int INodeInterpreter::drawEnumVariable(int size, const char* items[], int init_label)
    {
        const char* current_item = items[init_label];
        if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
        {
            for (int n = 0; n < size; n++)
            {
                bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
                if (ImGui::Selectable(items[n], is_selected)) {
                    current_item = items[n];
                    ImGui::EndCombo();
                    return n;
                }
            }
            ImGui::EndCombo();
        }
        return -1;
    }
#define NODE_PROPERTY(CLASS,NAME,CATEGORY,DESC,COLOR,SIZE)\
    std::map<std::string, VariableWithDefault<std::string>> CLASS::StringMetaMap = { {"Name",#NAME},{"Desc",#DESC},{"Category",#CATEGORY} };\
    std::map<std::string, VariableWithDefault<int>> CLASS::IntMetaMap = { {"NodeSize",SIZE} };\
    std::map<std::string, VariableWithDefault<ImVec4>> CLASS::ImVec4MetaMap{ {"Color", ImVec4 COLOR} };

    NODE_PROPERTY(NumberNode, Number, Number, number present, (.0f, .0f, .0f, 1.0f), 80);
    const std::array<NodeInputPinMeta, 0> NumberNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> NumberNode::OutputPinMeta = { "Output(Number)" };

    NODE_PROPERTY(IntermediateVariableNode, IntermediateVariable, Misc, intermediate variable, (.0f, .0f, .0f, 1.0f), 60);
    const std::array<NodeInputPinMeta, 1> IntermediateVariableNode::InputPinMeta = { NodeInputPinMeta{"Input",PinValueType::Any} };
    const std::array<NodeOutputPinMeta, 0> IntermediateVariableNode::OutputPinMeta = { };

    NODE_PROPERTY(OutputDiffuseNode, Diffuse, Output,no light diffuse , (.0f, .0f, .0f, 1.0f), 60);
    const std::array<NodeInputPinMeta, 1> OutputDiffuseNode::InputPinMeta = { NodeInputPinMeta{"Diffuse",PinValueType::Any} };
    const std::array<NodeOutputPinMeta, 0> OutputDiffuseNode::OutputPinMeta = {};

    NODE_PROPERTY(FloatConstantNode, Float_Constant, Constant, const float type, (.0f, .0f, .0f, 1.0f), 80);
    const std::array<NodeInputPinMeta, 0> FloatConstantNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> FloatConstantNode::OutputPinMeta = { "Output(Float)" };

    NODE_PROPERTY(FloatVariableNode, Float_Variable, Variable, variable float type, (.0f, .0f, .0f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 0> FloatVariableNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> FloatVariableNode::OutputPinMeta = { "Output(Float)" };

    NODE_PROPERTY(Vector3ConstantNode, Vector3_Constant, Constant, const float3 type, (.0f, .0f, .0f, 1.0f), 80);
    const std::array<NodeInputPinMeta, 0> Vector3ConstantNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> Vector3ConstantNode::OutputPinMeta = { "Output(Vector3)" };

    NODE_PROPERTY(Vector3VariableNode, Vector3_Variable, Variable, variable float3 type, (.0f, .0f, .0f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 0> Vector3VariableNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> Vector3VariableNode::OutputPinMeta = { "Output(Vector3)" };

    NODE_PROPERTY(TimeNode, Time, Misc, time since the game started, (.0f, .0f, .0f, 1.0f), 45);
    const std::array<NodeInputPinMeta, 0> TimeNode::InputPinMeta = {};
    const std::array<NodeOutputPinMeta, 1> TimeNode::OutputPinMeta = { "Output" };

    NODE_PROPERTY(SinNode, Sin, Math, return sin(x), (.0f, .0f, 1.0f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 1> SinNode::InputPinMeta = { NodeInputPinMeta{"Input",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> SinNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(CosNode, Cos, Math, return cos(x), (.0f, .0f, 1.0f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 1> CosNode::InputPinMeta = { NodeInputPinMeta{"Input",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> CosNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(AddNode, Add, Math, return x + y, (1.0f, 1.0f, 1.0f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 2> AddNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> AddNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(SubNode, Sub, Math, return x - y, (1.0f, 1.0f, 1.0f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 2> SubNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> SubNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(MultiplyNode, Multiply, Math, return x * y, (1.0f, .0f, 0.5f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 2> MultiplyNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> MultiplyNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(DivideNode, Divide, Math, return x / y, (1.0f, .0f, 0.5f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 2> DivideNode::InputPinMeta = { NodeInputPinMeta{"Left",PinValueType::Demical_Float},NodeInputPinMeta{"Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> DivideNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(MultiplyAddNode, MultiplyAdd, Math, return x * y + z, (1.0f, .0f, 0.5f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 3> MultiplyAddNode::InputPinMeta = { NodeInputPinMeta{"Mul Left",PinValueType::Demical_Float},NodeInputPinMeta{"Mul Right",PinValueType::Demical_Float},NodeInputPinMeta{"Add Right",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> MultiplyAddNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(AppendChannelNode, AppendChannel, Vector, append channel, (1.0f, .0f, 0.5f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 4> AppendChannelNode::InputPinMeta = { NodeInputPinMeta{"Input(Float)",PinValueType::Float},NodeInputPinMeta{"Input(Float)",PinValueType::Float},NodeInputPinMeta{"Input(Float)",PinValueType::Float},NodeInputPinMeta{"Input(Float)",PinValueType::Float} };
    const std::array<NodeOutputPinMeta, 1> AppendChannelNode::OutputPinMeta = { "Result" };

    NODE_PROPERTY(MaskNode, Mask, Vector, get specific channel, (1.0f, .0f, 0.5f, 1.0f), 100);
    const std::array<NodeInputPinMeta, 1> MaskNode::InputPinMeta = { NodeInputPinMeta{"Input(Vector)",PinValueType::Demical_Float} };
    const std::array<NodeOutputPinMeta, 1> MaskNode::OutputPinMeta = { "Result" };

}
