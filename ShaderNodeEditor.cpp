#include "ShaderNodeEditor.h"
#include "imnodes.h"
#include "node_editor.h"
namespace ShaderNodeEditor {
    void ShaderNodeEditor::Init()
    {
#define ADD_POPUP_NODE_ITEM(NAME,FUNC) {#NAME, std::bind(&ShaderNodeEditor::FUNC, this)}
        click_pos = ImVec2(400.0f, 200.0f);
        addOutput();
        popupNodesFunctions.emplace("Constant", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Float, addPopupItem_ConstantFloat)
        });

        popupNodesFunctions.emplace("Variable", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Float, addPopupItem_VariableFloat)
        });
        popupNodesFunctions.emplace("Misc", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Time, addPopupItem_Time)
        });
        popupNodesFunctions.emplace("Math", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Sine, addPopupItem_Sine),
                ADD_POPUP_NODE_ITEM(Add, addPopupItem_Add),
                ADD_POPUP_NODE_ITEM(Substract, addPopupItem_Substract),
                ADD_POPUP_NODE_ITEM(Multiply, addPopupItem_Multiply),
                ADD_POPUP_NODE_ITEM(Divide, addPopupItem_Divide)
        });

    }
    void ShaderNodeEditor::addNodeToGraph(const std::string& node_name, NodePtr& node)
    {
        if (auto node_iter = nodes.find(node_name); node_iter == nodes.end()) {
            NodeVec a;
            a.push_back(node);
            nodes.emplace(node_name, std::move(a));
        }
        else {
            node_iter->second.push_back(node);
        }
    }

    bool ShaderNodeEditor::removeNodeFromGraph(const std::string& node_name, size_t id)
    {
        if (auto node_iter = nodes.find(node_name); node_iter != nodes.end()) {
            for (auto iter = node_iter->second.begin(); iter != node_iter->second.end(); ++iter)
            {
                auto& nodeId = iter->get()->Id;
                if (nodeId.op == id)
                {
                    graph_.erase_node(nodeId.op);
                    for (auto v : nodeId.params) {
                        graph_.erase_node(v);
                    }
                    node_iter->second.erase(iter);
                    return true;
                }
            }
            return false;
        }
    }

    bool ShaderNodeEditor::removeOutputNodeFromGraph(size_t id)
    {
        if (output_nodes_.size() > 0u && output_nodes_[0].out == id)
        {
            const auto& node = output_nodes_[0];
            graph_.erase_node(node.out);
            graph_.erase_node(node.in);
            output_nodes_.pop_back();
            return true;
        }
        return false;
    }

    void ShaderNodeEditor::Show()
    {
        ImGui::Begin("Visual Shader Editor", &is_open, ImVec2(1024, 768), 0.9f, ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("Eval")) {
            graph_.evaluate(output_nodes_[0u].out);
        }
        
        imnodes::BeginNodeEditor();
        showOutputNodes();
        showCommonNodes();
        showLinks();
        showPopupMenu();
        imnodes::EndNodeEditor();
        linkInput();
        ImGui::End();
    }

    bool ShaderNodeEditor::find_and_remove_node(size_t id)
    {
        for (const auto& v : nodes) {
            for (const auto& node : v.second) {
                auto iter = std::find_if(node->Id.params.begin(), node->Id.params.end(), [id](NodeParam x) {return x.id == size_t(id); });
                if (iter != node->Id.params.end())
                {
                    graph_.erase_node(*iter);
                    node->Id.params.erase(iter);
                    return true;
                }
            }
        }
        return false;
    }

    void ShaderNodeEditor::showOutputNodes()
    {
        for (const auto& node : output_nodes_)
        {
            const float node_width = 100.0f;
            imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(11, 109, 191, 255));
            imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(45, 126, 194, 255));
            imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(81, 148, 204, 255));
            imnodes::BeginNode(node.out);
            imnodes::Name("output");

            ImGui::Dummy(ImVec2(node_width, 0.f));
            {
                // TODO: the color style of the pin needs to be pushed here
                imnodes::BeginInputAttribute(int(node.in));
                const float label_width = ImGui::CalcTextSize("in").x;
                ImGui::Text("in");
                if (graph_.node(node.in)->type == Node_Number)
                {
                    ImGui::SameLine();
                    ImGui::PushItemWidth(node_width - label_width);
                    ImGui::DragFloat(
                        "##hidelabel",
                        &graph_.node(node.in)->number.fVal[0],
                        0.01f,
                        0.f,
                        1.0f);
                    ImGui::PopItemWidth();
                }
                imnodes::EndAttribute();
            }

            imnodes::EndNode();
            imnodes::PopColorStyle();
            imnodes::PopColorStyle();
            imnodes::PopColorStyle();
        }

    }

    void ShaderNodeEditor::showLinks()
    {
        for (auto iter = graph_.begin_edges(); iter != graph_.end_edges(); ++iter) {
            // don't render internal edges
            // internal edges always look like
            //
            // Node_Output | Node_Operation
            // ->
            // Node_Number | Node_NumberExpression
            const NodeType type = graph_.node(iter->second.to)->type;
            if (type == Node_Number || type == Node_NumberExpression)
                continue;
            imnodes::Link(iter->first, iter->second.from, iter->second.to);
        }
    }

    void ShaderNodeEditor::showCommonNodes()
    {
        for (auto& kv : nodes) {
            showCommonNode(kv.first, kv.second);
        }
    }

    void ShaderNodeEditor::showCommonNode(const std::string& name, NodeVec& nodes)
    {
        for (auto& v : nodes)
        {
            auto& node = v->Id;
            const float node_width = 100.0f;
            imnodes::BeginNode(node.op);
            imnodes::Name(name.c_str());

            if (graph_.node(node.op)->ForceShowNumber()) {
                imnodes::BeginInputAttribute(int(node.op));
                ImGui::PushItemWidth(64);
                ImGui::DragFloat(
                    "##hidelabel",
                    &graph_.node(node.op)->number.fVal[0],
                    0.01f,
                    0.f,
                    1.0f);
                ImGui::PopItemWidth();
                imnodes::EndAttribute();
            }
            size_t i = 0;
            for (NodeParam& input : node.params)
            {
                auto& metaInfo = v->GetInputMetaInfo(i);
                imnodes::BeginInputAttribute(int(input.id));
                const float label_width = ImGui::CalcTextSize(metaInfo.name).x;
                ImGui::Text(metaInfo.name);
                auto input_type = graph_.node(input.id)->type;
                if (input_type == Node_Number) {
                    ImGui::SameLine();
                    ImGui::PushItemWidth(node_width - label_width);
                    ImGui::DragFloat(
                        "##hidelabel",
                        &input.value.fVal[0],
                        0.01f,
                        0.f,
                        1.0f);
                    ImGui::PopItemWidth();
                }
                imnodes::EndAttribute();
                ++i;
            }

            ImGui::Spacing();

            {
                auto& metaInfo = v->GetOutputMetaInfo(0);
                imnodes::BeginOutputAttribute(int(node.op));
                const float label_width = ImGui::CalcTextSize(metaInfo.name).x;
                ImGui::Indent(node_width - label_width);
                ImGui::Text(metaInfo.name);
                imnodes::EndAttribute();
            }

            imnodes::EndNode();
        }
    }

    void ShaderNodeEditor::showPopupMenu()
    {
        const bool open_popup = ImGui::IsMouseClicked(1);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));

        if (!ImGui::IsAnyItemHovered() && ImGui::IsAnyWindowHovered() &&
            open_popup)
        {
            ImGui::OpenPopup("add node");
        }
        if (ImGui::BeginPopup("add node"))
        {
            click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

            for (auto& kv : popupNodesFunctions) {
                const ImVec4 cColor(0.8f, 0.65f, 0.38f, 1.0f);
                ImGui::TextColored(cColor, kv.first.c_str());
                for (auto& m : kv.second) {
                    if (ImGui::MenuItem(("\t" + m.first).c_str()))
                    {
                        m.second();
                    }
                }
            }

            ImGui::EndPopup();
        }

        ImGui::PopStyleVar();
    }

    void ShaderNodeEditor::linkInput()
    {
        Id link_selected;
        if (int selectedId = -1; imnodes::IsLinkSelected(&selectedId))
        {
            link_selected.id = selectedId;
            if (ImGui::IsKeyReleased(ImGui::GetIO().KeyMap[ImGuiKey_X]))
            {
                auto node = graph_.node(graph_.edge(link_selected).from);
                assert(node->type == Node_NumberExpression);
                node->type = Node_Number;
                graph_.erase_edge(size_t(link_selected));
            }
        }

        Id node_selected;
        if (int selectedId = -1; imnodes::IsNodeSelected(&selectedId))
        {
            node_selected.id = selectedId;
            if (ImGui::IsKeyReleased(ImGui::GetIO().KeyMap[ImGuiKey_X]))
            {
                find_and_remove_node(node_selected);
            }
        }

        Id link_start, link_end;
        int start, end;
        if (imnodes::IsLinkCreated(&start, &end))
        {
            link_start.id = start, link_end.id = end;
            // in the expression graph, we want the edge to always go from
            // the number to the operation, since the graph is directed!
            const size_t from_id = graph_.node(link_start)->type == Node_Number
                ? link_start
                : link_end;
            const size_t to_id = graph_.node(link_end)->type == Node_Operation
                ? link_end
                : link_start;

            bool invalid_node = false;
            for (size_t edge : graph_.edges_to_node(from_id))
            {
                if (graph_.edge(edge).from == to_id)
                {
                    invalid_node = true;
                    break;
                }
            }

            invalid_node = (graph_.node(from_id)->type != Node_Number ||
                graph_.node(to_id)->type != Node_Operation) ||
                invalid_node;

            if (!invalid_node)
            {
                graph_.add_edge(from_id, to_id);
                auto node_from = graph_.node(from_id);
                auto node_to = graph_.node(to_id);
                node_from->type = node_from->type == Node_Number
                    ? Node_NumberExpression
                    : node_from->type;
            }
        }
    }

    void ShaderNodeEditor::addOutput()
    {
        OutputNode node;

        NodePtr in = std::make_shared<FloatVariableNode>();
        in->type = NodeType::Node_Number;

        NodePtr op = std::make_shared<FloatVariableNode>();
        op->type = NodeType::Node_Output;

        node.out = graph_.add_node(op);
        node.in = graph_.add_node(in);
        output_nodes_.push_back(node);

        graph_.add_edge(node.out, node.in);

        imnodes::SetNodePos(node.out, click_pos);
    }



    static ShaderNodeEditor color_editor;
}
namespace example
{
    void NodeEditorInitialize()
    {
        auto& style = imnodes::GetStyle();
        style.colors[imnodes::ColorStyle_TitleBar] = IM_COL32(232, 27, 86, 255);
        style.colors[imnodes::ColorStyle_TitleBarHovered] = IM_COL32(235, 67, 115, 255);
        style.colors[imnodes::ColorStyle_TitleBarSelected] = IM_COL32(241, 108, 146, 255);
        style.colors[imnodes::ColorStyle_Link] = IM_COL32(255, 210, 0, 255);
        style.colors[imnodes::ColorStyle_LinkHovered] = IM_COL32(255, 229, 108, 255);
        style.colors[imnodes::ColorStyle_LinkSelected] = IM_COL32(255, 237, 154, 255);
        style.colors[imnodes::ColorStyle_Pin] = IM_COL32(255, 210, 0, 255);
        style.colors[imnodes::ColorStyle_PinHovered] = IM_COL32(255, 237, 154, 255);
        style.flags = imnodes::Flags_None;
        ShaderNodeEditor::color_editor.Init();
    }

    void NodeEditorShow() { ShaderNodeEditor::color_editor.Show(); }

    void NodeEditorShutdown() {}
}
