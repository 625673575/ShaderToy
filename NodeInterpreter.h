#pragma once
#include "Node.h"
#include "Externals/dear_imgui/imgui.h"
namespace ShaderNodeEditor {

    extern const char* xyzw[4];
    extern const char* XYZW[4];
    extern const char* rgba[4];
    extern const char* RGBA[4];

    struct NodeInputPinMeta {
        const char* name;
        PinValueType pin;
    };
    struct NodeOutputPinMeta {
        const char* name;
    };
    struct INodeInterpreter {
        INodeInterpreter() {
            std::memset(number.fVal, 0, sizeof(number.fVal));
            variableName.reserve(255);
        };
        virtual ~INodeInterpreter() = default;
        INodeInterpreter& operator=(const INodeInterpreter&) = default;
        INodeInterpreter(const INodeInterpreter&) = default;
        virtual std::string Interpret() = 0;
        virtual std::string Intermediate() { return ""; }
        virtual const NodeInputPinMeta& GetInputMetaInfo(size_t i) const = 0;
        virtual const NodeOutputPinMeta& GetOutputMetaInfo(size_t i) const = 0;
        virtual const char* GetCategory() const = 0;
        virtual const char* GetName()const = 0;
        virtual const char* GetDesc()const = 0;
        virtual int GetNodeSize()const = 0;
        //event
        virtual void OnInspectGUI() {}
        virtual void OnNodeClicked() {}
        virtual void OnNodeCreate() {}
        virtual void OnNodeDestroy() {}
        virtual void OnPropertyChanged() {}

        virtual bool ForceShowNumber() { return false; }
        virtual bool ForceHideParamNumber() { return false; }
        virtual bool IsValid() { return true; }
        virtual std::string GetOutput(size_t i) { assert(OutputPinMeta.size() > 1); return ""; }
        void SetRuntimeType(PinValueType type) { runtimeValueType = type; };
        void* NumberData() { return &number.fVal[0]; }
        virtual PinValueType MergeOutputType(const std::vector<PinValueType>& p);
        bool drawVectorVariable(int size) { return drawVectorVariable(size, number.fVal); }
        bool drawVectorVariable(int size, float* data);
        bool drawDragIntVariable(const char* label, int* value, int min_val, int max_val);
        bool drawUpdownIntVariable(const char* label, int* value, int min_val, int max_val);
        int drawEnumVariable(int size, const char* labels[], int init_label = 0);
        //如果有变量名,就单独创建一个变量
        std::string variableName;
        NodeType type;
        PinValue number;//constant variable output用
        PinValueType runtimeValueType;
        NodeId Id;
    };

    using NodePtr = std::shared_ptr<INodeInterpreter>;
    using NodeVec = std::vector<NodePtr>;

#define FORCE_SHOW_NUMBER bool ForceShowNumber()override { return true; }
    struct IVariableInterpreter :public INodeInterpreter {
        IVariableInterpreter() = default;
        virtual ~IVariableInterpreter() = default;
        FORCE_SHOW_NUMBER
    };
    struct IConstantInterpreter :public INodeInterpreter {
        IConstantInterpreter() = default;
        virtual ~IConstantInterpreter() = default;
        FORCE_SHOW_NUMBER
    };
    struct IOperationInterpreter :public INodeInterpreter {
        bool isValid;
        bool IsValid()override { return isValid; }
        IOperationInterpreter() :isValid(true) {}
        virtual ~IOperationInterpreter() = default;
    };

    using OpNodePtr = std::shared_ptr<IOperationInterpreter>;
    using OpNodeVec = std::vector<OpNodePtr>;

    struct IMultiOperationInterpreter :public INodeInterpreter {
        IMultiOperationInterpreter() {}
        OpNodeVec opOutputs;
        void addOutput(const OpNodePtr& p) {
            opOutputs.push_back(p);
        }
    };

#define DECLAR_NODE_STATIC_VAR \
    static std::map<std::string, VariableWithDefault<std::string>>StringMetaMap;\
    static std::map<std::string, VariableWithDefault<int>>IntMetaMap;\
    static std::map<std::string, VariableWithDefault<ImVec4>>ImVec4MetaMap;

#define INTERPRET_BEGIN(INPUT_COUNT,OUTPUT_COUNT)DECLAR_NODE_STATIC_VAR\
    static const std::array<NodeInputPinMeta, INPUT_COUNT> InputPinMeta;\
    static const std::array<NodeOutputPinMeta, OUTPUT_COUNT> OutputPinMeta;\
    const NodeInputPinMeta& GetInputMetaInfo(size_t i)const override {return InputPinMeta[i];}\
    const NodeOutputPinMeta& GetOutputMetaInfo(size_t i)const override {return OutputPinMeta[i];}\
    const char* GetCategory()const override{return StringMetaMap["Category"].get_value();}\
    const char* GetName()const override{return StringMetaMap["Name"].get_value();}\
    const char* GetDesc()const override{return StringMetaMap["Desc"].get_value();}\
    int GetNodeSize()const override{return IntMetaMap["NodeSize"].get_value();}\
    std::string Interpret()override\
{ assert(Id.params.size() == INPUT_COUNT);

#define INTERPRET_END }
#define FORCE_HIDE_PARAM bool ForceHideParamNumber()override { return true; }
    extern size_t VariableCount;
    struct NumberNode :public IOperationInterpreter {
        NumberNode() {
            SetRuntimeType(PinValueType::Any);
        }
        INTERPRET_BEGIN(0, 1)
            return "Number(Any Type)";
        INTERPRET_END
    };
    struct FloatConstantNode :public IConstantInterpreter {
        FloatConstantNode() {
            SetRuntimeType(PinValueType::Float);
        }
        void OnInspectGUI()override { drawVectorVariable(1, number.fVal); }
        INTERPRET_BEGIN(0, 1)
            if (variableName.empty())return std::to_string(number.fVal[0]);
            else return variableName;
        INTERPRET_END
    };

    struct FloatVariableNode :public IVariableInterpreter {
        FloatVariableNode() {
            SetRuntimeType(PinValueType::Float);
        }
        void OnInspectGUI()override { drawVectorVariable(1, number.fVal); }
        INTERPRET_BEGIN(0, 1)
            if (variableName.empty())return std::to_string(number.fVal[0]);
            else return variableName;
        INTERPRET_END
    };

    struct Vector3ConstantNode :public IConstantInterpreter {
        Vector3ConstantNode() {
            SetRuntimeType(PinValueType::Vector3);
        }
        void OnInspectGUI()override { drawVectorVariable(3, number.fVal); }
        INTERPRET_BEGIN(0, 1)
            if (variableName.empty())return "float3(" + std::to_string(number.fVal[0]) +
                "," + std::to_string(number.fVal[1]) +
                "," + std::to_string(number.fVal[2]) + ")";
            else return variableName;
        INTERPRET_END
    };

    struct Vector3VariableNode :public IConstantInterpreter {
        Vector3VariableNode() {
            SetRuntimeType(PinValueType::Vector3);
        }
        void OnInspectGUI()override { drawVectorVariable(3, number.fVal); }
        INTERPRET_BEGIN(0, 1)
            if (variableName.empty())return "float3(" + std::to_string(number.fVal[0]) +
                "," + std::to_string(number.fVal[1]) +
                "," + std::to_string(number.fVal[2]) + ")";
            else return variableName;
        INTERPRET_END
    };

    struct TimeNode :public IOperationInterpreter {
        TimeNode() {
            SetRuntimeType(PinValueType::Float);
        }
        INTERPRET_BEGIN(0, 1)
            return "_Time";
        INTERPRET_END
    };

    struct SinNode :public IOperationInterpreter {
        SinNode() {
            SetRuntimeType(PinValueType::Demical_Float);
        }
        INTERPRET_BEGIN(1, 1)
            return "sin({0})";
        INTERPRET_END
    };

    struct CosNode :public IOperationInterpreter {
        CosNode() {
            SetRuntimeType(PinValueType::Demical_Float);
        }
        INTERPRET_BEGIN(1, 1)
            return "cos({0})";
        INTERPRET_END
    };

    struct AddNode :public IOperationInterpreter {
        AddNode() {
            SetRuntimeType(PinValueType::Demical_Float);
        }
        INTERPRET_BEGIN(2, 1)
            return "({0} + {1})";
        INTERPRET_END
    };

    struct SubNode :public IOperationInterpreter {
        SubNode() {
            SetRuntimeType(PinValueType::Demical_Float);
        }
        INTERPRET_BEGIN(2, 1)
            return "({0} - {1})";
        INTERPRET_END
    };

    struct MultiplyNode :public IOperationInterpreter {
        MultiplyNode() {
            SetRuntimeType(PinValueType::Demical_Float);
        }
        INTERPRET_BEGIN(2, 1)
            return "({0} * {1})";
        INTERPRET_END
    };

    struct DivideNode :public IOperationInterpreter {
        DivideNode() {
            SetRuntimeType(PinValueType::Demical_Float);
        }
        INTERPRET_BEGIN(2, 1)
            return "({0} / {1})";
        INTERPRET_END
    };

    struct MultiplyAddNode :public IOperationInterpreter {
        MultiplyAddNode() {
            SetRuntimeType(PinValueType::Demical_Float);
        }
        INTERPRET_BEGIN(3, 1)
            return "(({0} * {1}) + {2})";
        INTERPRET_END
    };

    struct AppendChannelNode :public IOperationInterpreter {
        AppendChannelNode() :channel_count(4) {
            SetRuntimeType(PinValueType::Demical_Float);
        }
        int channel_count;
        void OnInspectGUI()override {
            if (drawUpdownIntVariable("Channel Count", &channel_count, 2, 4)) {
                OnPropertyChanged();
            }
        }
        void OnPropertyChanged() {
            changeName();
            changeParamNodeValid();
        }
        void changeName() {
            std::string ChannelName = "AppendChannel Vector" + std::to_string(channel_count);
            StringMetaMap["Name"].set_value(ChannelName);
        }
        void changeParamNodeValid() {
            for (int i = 0; i < 4; ++i) {
                if (auto& v = Id.params[i].value; v != nullptr) {
                    std::static_pointer_cast<IOperationInterpreter>(v)->isValid = i < channel_count;
                }
            }
        }
        INTERPRET_BEGIN(4, 1)
            switch (channel_count)
            {
            case 2:
                return "float2({0},{1})";
            case 3:
                return "float3({0},{1},{2})";
            case 4:
                return "float4({0},{1},{2},{3})";
            default:
                break;
            }
        return "";
        INTERPRET_END
    };

    //根据RuntimeType决定返回类型
    struct IntermediateVariableNode : public IOperationInterpreter {
        std::string GetTypeName() {
            switch (runtimeValueType)
            {
            case PinValueType::Float:
                return "float";
            case PinValueType::Vector2:
                return "float2";
            case PinValueType::Vector3:
                return "float3";
            case PinValueType::Vector4:
                return "float4";
            case PinValueType::INT:
                return "int";
            case PinValueType::UINT:
                return "uint";
            case PinValueType::BOOL:
                return "bool";
            default:
                break;
            }
            return "";
        }
        INTERPRET_BEGIN(1, 1)
            return GetTypeName() + " " + variableName + " = {0};";
        INTERPRET_END
    };


    struct MaskNode :public IOperationInterpreter {
        int mask_idx;
        MaskNode() :mask_idx(0) { changeName(); }
        std::string GetOutput(size_t i)override {
            char x[] = ".x";
            if (mask_idx) {
                x[1] = xyzw[i][0];
                return variableName + x;
            }
        }
        void OnPropertyChanged() {
            changeName();
        }
        void changeName() {
            std::string MaskName = "MaskR";
            MaskName[4] = RGBA[mask_idx][0];
            StringMetaMap["Name"].set_value(MaskName);
        }
        void OnInspectGUI()override {
            if (auto x = drawEnumVariable(4, RGBA, mask_idx); x > -1)
            {
                if (mask_idx != x) {
                    mask_idx = x;
                    OnPropertyChanged();
                }
            }
        }
        void OnNodeClicked()override {
            OutputDebugStringA(" Node Clicked ");
        }
        void OnNodeCreate()override {
            OutputDebugStringA(" Node Create ");
        }
        void OnNodeDestroy()override {
            OutputDebugStringA(" Node Destroy ");
        }
        FORCE_HIDE_PARAM
            INTERPRET_BEGIN(1, 1)
            std::string x(&xyzw[mask_idx][0]);
        return "({0}." + x + ")";
        INTERPRET_END

    };
}
