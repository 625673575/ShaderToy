#pragma once
#include "NodeInterpreter.h"
namespace ShaderNodeEditor {
    class ShaderNodeEditor
    {
    public:
        template<typename NODE_T, size_t _INPUT_COUNT,size_t _OUTPUT_COUNT>
        NodePtr addNode() {
        NodePtr op_ptr = std::make_shared<NODE_T>();
        op_ptr->Id.op = graph_.add_node(op_ptr);
        op_ptr->type = NodeType::Node_Operation;

        for (size_t i = 0; i < _INPUT_COUNT; ++i) {
            NodePtr value_ptr = std::make_shared<NumberNode>();
            value_ptr->type = NodeType::Node_Number;
            size_t node_id = graph_.add_node(value_ptr);

            op_ptr->Id.params.emplace_back(node_id,value_ptr);
            graph_.add_edge(op_ptr->Id.op, op_ptr->Id.params[i].id);
        }

        addNodeToGraph(NODE_T::Name, op_ptr);//放在最后，因为被 std::move 掉了
        return op_ptr;
    }
        template<typename NODE_T, size_t _COUNT>
        NodePtr addNode() {
            NodePtr op_ptr = std::make_shared<NODE_T>();
            op_ptr->Id.op = graph_.add_node(op_ptr);
            op_ptr->type = NodeType::Node_Operation;

            for (size_t i = 0; i < _COUNT; ++i) {
                NodePtr value_ptr = std::make_shared<NumberNode>();
                value_ptr->type = NodeType::Node_Number;
                size_t node_id = graph_.add_node(value_ptr);

                op_ptr->Id.params.emplace_back(node_id, value_ptr);
                graph_.add_edge(op_ptr->Id.op, op_ptr->Id.params[i].id);
            }

            addNodeToGraph(NODE_T::StringMetaMap["Name"].get_default(), op_ptr);//放在最后，因为被 std::move 掉了
            op_ptr->OnNodeCreate();
            return op_ptr;
        }
        template<typename NODE_T>
        NodePtr addNode() {
            NodePtr op_ptr = std::make_shared<NODE_T>();
            op_ptr->Id.op = graph_.add_node(op_ptr);
            op_ptr->type = NodeType::Node_Operation;

            addNodeToGraph(NODE_T::StringMetaMap["Name"].get_default(), op_ptr);//放在最后，因为被 std::move 掉了
            return op_ptr;
        }
    public:

        ShaderNodeEditor() = default;
        ~ShaderNodeEditor() = default;
        Graph graph_;
        StaticVector<OutputNode, 1> output_nodes_;
        std::unordered_map<std::string, NodeVec> nodes;
        void Init();
        void Show();
    private:
        void addNodeToGraph(const std::string& node_name, NodePtr& node);
        bool removeNodeFromGraph(const std::string& node_name, size_t id);
        bool removeOutputNodeFromGraph(size_t id);
        bool findRemoveNode(size_t id);
        NodePtr findOpNodeByInput(size_t from_id);
    private:
        static size_t variable_counter;
        ImVec2 click_pos;
        bool is_open = true;
        bool is_property_open = true;
        using NodeFuncMap = std::map<std::string, std::function<void()>>;
        std::map<std::string, NodeFuncMap> popupNodesFunctions;
        void drawVariableNumber(const char* category, int op, int size, float* data);
        void drawNodeProperty(NodePtr& ptr);
    private:
        void showOutputNodes();
        void showLinks();
        void showCommonNodes();
        void showCommonNode(const std::string& name, NodeVec& nodes);
        void showPopupMenu();
        void linkInput();
    private:
        void addOutput();
        void addPopupItem_VariableFloat() {
            auto node = addNode<FloatVariableNode>();
            node->variableName = node->GetCategory() + std::to_string(variable_counter++);
        };
        void addPopupItem_ConstantFloat() { addNode<FloatConstantNode>(); };

        void addPopupItem_VariableVector3() {
            auto node = addNode<Vector3VariableNode>();
            node->variableName = node->GetCategory() + std::to_string(variable_counter++);
        };
        void addPopupItem_ConstantVector3() { addNode<Vector3ConstantNode>(); };

        void addPopupItem_Time() { addNode<TimeNode>(); };
        void addPopupItem_Sine() { addNode<SinNode, 1>(); };
        void addPopupItem_Cos() { addNode<CosNode, 1>(); };
        void addPopupItem_Add() { addNode<AddNode, 2>(); };
        void addPopupItem_Substract() { addNode<SubNode, 2>(); };
        void addPopupItem_Multiply() { addNode<MultiplyNode, 2>(); };
        void addPopupItem_Divide() { addNode<DivideNode, 2>(); };
        void addPopupItem_MultiplyAdd() { addNode<MultiplyAddNode, 3>(); };

        void addPopupItem_AppendChannel() { addNode<AppendChannelNode, 4>(); };
        void addPopupItem_Mask() { addNode<MaskNode, 1>(); };
    };
}
