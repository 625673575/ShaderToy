#pragma once
#include "NodeInterpreter.h"
namespace ShaderNodeEditor {
    class ShaderNodeEditor
    {
    public:
        template<typename NODE_T, size_t _INPUT_COUNT, size_t _OUTPUT_COUNT>
        NodePtr addNode() {
            NodePtr op_ptr = std::make_shared<NODE_T>();
            op_ptr->Id.op = graph_.add_node(op_ptr);

            for (size_t i = 0; i < _INPUT_COUNT; ++i) {
                NodePtr value_ptr = std::make_shared<NumberNode>();
                value_ptr->type = NodeType::Node_Number;
                size_t node_id = graph_.add_node(value_ptr);

                op_ptr->Id.params.emplace_back(node_id, value_ptr);
                graph_.add_edge(op_ptr->Id.op, op_ptr->Id.params[i].id);
            }

            addNodeToGraph(NODE_T::StringMetaMap["Name"].get_default(), op_ptr);//放在最后，因为被 std::move 掉了
            return op_ptr;
        }
        template<typename NODE_T, size_t _COUNT>
        NodePtr addNode(bool addGraph = true) {
            NodePtr op_ptr = std::make_shared<NODE_T>();
            op_ptr->Id.op = graph_.add_node(op_ptr);

            for (size_t i = 0; i < _COUNT; ++i) {
                NodePtr value_ptr = std::make_shared<NumberNode>();
                value_ptr->type = NodeType::Node_Number;
                size_t node_id = graph_.add_node(value_ptr);

                op_ptr->Id.params.emplace_back(node_id, value_ptr);
                graph_.add_edge(op_ptr->Id.op, op_ptr->Id.params[i].id);
            }
            if (addGraph)
                addNodeToGraph(NODE_T::StringMetaMap["Name"].get_default(), op_ptr);//放在最后，因为被 std::move 掉了
            op_ptr->OnNodeCreate();
            return op_ptr;
        }
        template<typename NODE_T>
        NodePtr addNode() {
            NodePtr op_ptr = std::make_shared<NODE_T>();
            op_ptr->Id.op = graph_.add_node(op_ptr);

            addNodeToGraph(NODE_T::StringMetaMap["Name"].get_default(), op_ptr);//放在最后，因为被 std::move 掉了
            return op_ptr;
        }

    public:

        ShaderNodeEditor() = default;
        ~ShaderNodeEditor() = default;
        Graph graph_;
        StaticVector<NodePtr, 1> output_nodes_;
        std::unordered_map<std::string, NodeVec> nodes;
        std::vector<NodeCompositor> compositors;

        NodeVec intermediate_nodes() {
            NodeVec r;
            for (auto& v : nodes) {
                if (v.first == std::string("IntermediateVariable"))
                    for (auto& temp : v.second)
                        r.push_back(temp);
            }
            return r;
        }
        void Init();
        void Show();
    private:
        void addNodeToGraph(const std::string& node_name, NodePtr& node);
        bool removeNodeFromGraph(const std::string& node_name, size_t id);
        bool findRemoveNode(size_t id);
        //更新Input Node变化有些节点需要删除
        bool updateEdge(NodePtr& p);
        NodePtr findOpNodeByInput(size_t from_id);
    private:
        static size_t variable_counter;
        std::string autoVariableCount() { return std::to_string(variable_counter++); }
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
        void addPopupItem_Mask() {
            NodeCompositor comp;
            auto nodeI = addNode<IntermediateVariableNode, 1>();
            nodeI->variableName = "Intermediate_" + autoVariableCount();
            comp.node_compositor.push_back(nodeI->Id.op);
            auto nodeT = addNode<TempVariableNode>();
            nodeT->variableName = nodeI->variableName;
            comp.node_compositor.push_back(nodeT->Id.op);

            for (int i = 0; i < 4; ++i) {
                auto nodeFrom = addNode<MaskNode, 1>();
                std::static_pointer_cast<MaskNode>(nodeFrom)->mask_idx = i;
                auto from_id = nodeFrom->Id.params[0].id;
                comp.node_compositor.push_back(from_id);
                auto edge_id = graph_.add_edge(from_id, nodeT->Id.op);
                comp.edge_compositor.push_back(edge_id);
                graph_.node(from_id)->type = Node_NumberExpression;//必须要有否则无法Eval
            }
            comp.name = "MaskRGBA";
            compositors.push_back(std::move(comp));
        }
        void showCompositor_Mask();
    private:
        void addOutputDiffuse() {
            output_nodes_.push_back(addNode<OutputDiffuseNode, 1>(false));
        }
        void addPopupItem_IntermediateVariable() {
            auto node = addNode<IntermediateVariableNode, 1>();
            node->variableName = node->GetName() + autoVariableCount();
        }

        void addPopupItem_TempVariable() {
            auto node = addNode<TempVariableNode>();
            auto node_vec = intermediate_nodes();
            if (node_vec.empty())return;

            node->variableName = node_vec.front()->variableName;
        }
        void addPopupItem_VariableFloat() {
            auto node = addNode<FloatVariableNode>();
            node->variableName = node->GetName() + autoVariableCount();
        };
        void addPopupItem_ConstantFloat() {
            auto node = addNode<FloatConstantNode>();
            node->variableName = node->GetName() + autoVariableCount();
        };

        void addPopupItem_VariableVector3() {
            auto node = addNode<Vector3VariableNode>();
            node->variableName = node->GetName() + autoVariableCount();
        };
        void addPopupItem_ConstantVector3() {
            auto node = addNode<Vector3ConstantNode>();
            node->variableName = node->GetName() + autoVariableCount();
        };

        void addPopupItem_Time() { addNode<TimeNode>(); };
        void addPopupItem_Sine() { addNode<SinNode, 1>(); };
        void addPopupItem_Cos() { addNode<CosNode, 1>(); };
        void addPopupItem_Add() { addNode<AddNode, 2>(); };
        void addPopupItem_Substract() { addNode<SubNode, 2>(); };
        void addPopupItem_Multiply() { addNode<MultiplyNode, 2>(); };
        void addPopupItem_Divide() { addNode<DivideNode, 2>(); };
        void addPopupItem_MultiplyAdd() { addNode<MultiplyAddNode, 3>(); };

        void addPopupItem_AppendChannel() { addNode<AppendChannelNode, 4>(); };
        void addPopupItem_Mask1() { addNode<MaskNode, 1>(); };
    };
}
