#pragma once
#include "NodeInterpreter.h"
namespace ShaderNodeEditor {
    class ShaderNodeEditor
    {
        template<typename NODE_T, size_t _COUNT>
        void addNode() {
            NodePtr op_ptr = std::make_shared<NODE_T>();
            op_ptr->Id.op = graph_.add_node(op_ptr);
            op_ptr->type = NodeType::Node_Operation;

            for (size_t i = 0; i < _COUNT; ++i) {
                NodePtr value_ptr = std::make_shared<FloatVariableNode>();
                value_ptr->type = NodeType::Node_Number;
                size_t node_id = graph_.add_node(value_ptr);
                
                op_ptr->Id.params.push_back(node_id);
                graph_.add_edge(op_ptr->Id.op, op_ptr->Id.params[i].id);
            }

            imnodes::SetNodePos(op_ptr->Id.op, click_pos);
            addNodeToGraph(NODE_T::Name, op_ptr);//放在最后，因为被 std::move 掉了
        }
        template<typename NODE_T>
        void addNode() {
            NodePtr op_ptr = std::make_shared<NODE_T>();
            op_ptr->Id.op = graph_.add_node(op_ptr);
            op_ptr->type = NodeType::Node_Operation;

            imnodes::SetNodePos(op_ptr->Id.op, click_pos);
            addNodeToGraph(NODE_T::Name, op_ptr);//放在最后，因为被 std::move 掉了
        }
    public:
        using NodePtr = std::shared_ptr<INodeInterpreter>;
        using NodeVec = std::vector<NodePtr>;

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
        bool find_and_remove_node(size_t id);
    private:
        ImVec2 click_pos;
        bool is_open = true;
        using NodeFuncMap = std::map<std::string, std::function<void()>>;
        std::map<std::string, NodeFuncMap> popupNodesFunctions;
    private:
        void showOutputNodes();
        void showLinks();
        void showCommonNodes();
        void showCommonNode(const std::string& name, NodeVec& nodes);
        void showPopupMenu();
        void linkInput();
    private:
        void addOutput();
        void addPopupItem_VariableFloat() { addNode<FloatVariableNode>(); };
        void addPopupItem_ConstantFloat() { addNode<FloatConstantNode>(); };
        void addPopupItem_Time() { addNode<TimeNode>(); };
        void addPopupItem_Sine() { addNode<SinNode, 1>(); };
        void addPopupItem_Add() { addNode<AddNode, 2>(); };
        void addPopupItem_Substract() { addNode<SubNode, 2>(); };
        void addPopupItem_Multiply() { addNode<MultiplyNode, 2>(); };
        void addPopupItem_Divide() { addNode<DivideNode, 2>(); };
        void addPopupItem_MultiplyAdd() { addNode<MultiplyAddNode, 3>(); };
    };
}
