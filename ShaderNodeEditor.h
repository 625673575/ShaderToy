#pragma once
#include "NodeInterpreter.h"
namespace ShaderNodeEditor {
    class ShaderNodeEditor
    {
    public:
        ShaderNodeEditor() = default;
        ~ShaderNodeEditor() = default;
        Graph graph_;
        StaticVector<OutputNode, 1> output_nodes_;
        using NodePtr = std::unique_ptr<INodeInterpreter>;
        using NodeVec = std::vector<NodePtr>;
        std::unordered_map<std::string, NodeVec> nodes;
        ImVec2 click_pos;
        void Init();
        void AddNode(const std::string& node_name, NodePtr& node);
        bool RemoveNode(const std::string& node_name, size_t id);
        bool RemoveOutputNode(size_t id);
        void Show();
    private:
        bool find_and_remove_node(size_t id);
    private:
        void showOutputNodes();
        void showLinks();
        void showCommonNodes();
        void showCommonNode(const std::string& name, NodeVec& nodes);
        void showPopupMenu();
    private:
        void addOutput();
        void addTime();
        void addSin();
    };
}
