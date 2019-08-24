#include "ShaderNodeEditor.h"
#include "imnodes.h"
#include "node_editor.h"
namespace ShaderNodeEditor {
    size_t ShaderNodeEditor::variable_counter = 0;

    void ShaderNodeEditor::Init()
    {
#define ADD_POPUP_NODE_ITEM(NAME,FUNC) {#NAME, std::bind(&ShaderNodeEditor::FUNC, this)}
        click_pos = ImVec2(400.0f, 200.0f);
        addOutput();
        popupNodesFunctions.emplace("Constant", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Float, addPopupItem_ConstantFloat),
                ADD_POPUP_NODE_ITEM(Vector3, addPopupItem_ConstantVector3)
        });

        popupNodesFunctions.emplace("Variable", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Float, addPopupItem_VariableFloat),
                ADD_POPUP_NODE_ITEM(Vector3, addPopupItem_VariableVector3)
        });
        popupNodesFunctions.emplace("Misc", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Time, addPopupItem_Time),
                ADD_POPUP_NODE_ITEM(Mask, addPopupItem_Mask)
        });
        popupNodesFunctions.emplace("Math", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Sine, addPopupItem_Sine),
                ADD_POPUP_NODE_ITEM(Cos, addPopupItem_Cos),
                ADD_POPUP_NODE_ITEM(Add, addPopupItem_Add),
                ADD_POPUP_NODE_ITEM(Substract, addPopupItem_Substract),
                ADD_POPUP_NODE_ITEM(Multiply, addPopupItem_Multiply),
                ADD_POPUP_NODE_ITEM(Divide, addPopupItem_Divide),
                ADD_POPUP_NODE_ITEM(MultiplyAdd, addPopupItem_MultiplyAdd)
        });

    }
    void ShaderNodeEditor::addNodeToGraph(const std::string& node_name, NodePtr& node)
    {
        imnodes::SetNodePos(node->Id.op, click_pos);
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
        ImGui::Begin("Visual Shader Editor", &is_open, ImVec2(1024, 640), 0.9f, ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("Eval")) {
            graph_.evaluate(output_nodes_[0u].out);
        }

        imnodes::BeginNodeEditor();
        showOutputNodes();
        showCommonNodes();
        showLinks();
        imnodes::EndNodeEditor();
        linkInput();
        ImGui::End();
    }

    bool ShaderNodeEditor::findRemoveNode(size_t id)
    {
        for (auto& v : nodes) {
            NodeVec& time_nodes_ = v.second;
            auto iter = std::find_if(time_nodes_.begin(), time_nodes_.end(), [id](NodePtr& x) ->bool {return id == x->Id.op; });
            if (iter != time_nodes_.end())
            {
                for (auto& param : (*iter)->Id.params) {
                    auto edge_iter = std::find_if(graph_.begin_edges(), graph_.end_edges(), [param](auto& x) {return x.second.from == param.id; });
                    if (edge_iter != graph_.end_edges()) {
                        auto node = graph_.node(edge_iter->second.from);
                        if (node->type == Node_NumberExpression) {
                            node->type = Node_Number;
                            graph_.erase_edge(edge_iter->first);
                        }
                    }

                    edge_iter = std::find_if(graph_.begin_edges(), graph_.end_edges(), [param](auto& x) {return x.second.to == param.id; });
                    if (edge_iter != graph_.end_edges()) {
                        auto node = graph_.node(edge_iter->second.to);
                        node->type = Node_Operation;
                        graph_.erase_node(edge_iter->second.to);
                    }
                }
                auto edge_iter = std::find_if(graph_.begin_edges(), graph_.end_edges(), [id](auto& x) {return x.second.to == id; });
                auto node = graph_.node(edge_iter->second.from);
                if (node->type == Node_NumberExpression)
                    node->type = Node_Number;
                graph_.erase_node((*iter)->Id.op);
                time_nodes_.erase(iter);
                return true;
            }
        }
        return false;
    }

    NodePtr ShaderNodeEditor::findOpNodeByInput(size_t from_id)
    {
        for (auto& op : nodes) {
            for (auto& n : op.second) {
                std::vector<PinValueType> pvs;
                if (std::find(n->Id.params.begin(), n->Id.params.end(), from_id) != n->Id.params.end()) {
                    return n;
                }
            }
        }
        return nullptr;
    }

    void ShaderNodeEditor::drawVariableNumber(const char* category, int op, int size, float* data)
    {
        ImGui::Text(category);
        for (int i = 0; i < size; i++) {
            ImGui::PushItemWidth(80);
            imnodes::BeginInputAttribute(op);
            ImGui::DragFloat(
                xyzw[i],
                &data[i],
                0.01f,
                0.f,
                1.0f);
            imnodes::EndAttribute();
            ImGui::PopItemWidth();
        }
    }

    void ShaderNodeEditor::drawNodeProperty(NodePtr& ptr)
    {
        const ImGuiID PROPERTY_CHILD_FRAMD_ID = 123;
        if (ptr == nullptr)return;
        ImGui::Begin("Node Property", &is_open, ImVec2(200, 360));

        ImGui::PushStyleColor(ImGuiCol_TitleBg, IM_COL32(11, 109, 191, 255));
        ImGui::Text(ptr->GetName());
        ImGui::PopStyleColor();

        ptr->OnInspectGUI();

        ImGui::BeginChildFrame(PROPERTY_CHILD_FRAMD_ID, ImVec2(200, 100));
        ImGui::Text(ptr->GetDesc());
        ImGui::EndChildFrame();

        ImGui::End();
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
            imnodes::BeginNode(node.op);
            imnodes::Name(v->GetName());

            auto op_node = graph_.node(node.op);
            const int node_width = op_node->GetNodeSize();
            const char* cate = op_node->GetCategory();

            if (cate == std::string("Variable")) {
                imnodes::BeginInputAttribute(int(node.op));
                ImGui::PushItemWidth(node_width);
                ImGui::InputText("Name", op_node->variableName.data(), 255);
                ImGui::PopItemWidth();
                imnodes::EndAttribute();
            }
            if (op_node->ForceShowNumber()) {
                if (op_node->RuntimeValueType == PinValueType::Float) {
                    drawVariableNumber(op_node->GetCategory(), int(node.op), 1, op_node->number.fVal);
                }
                if (op_node->RuntimeValueType == PinValueType::Vector2) {
                    drawVariableNumber(op_node->GetCategory(), int(node.op), 2, op_node->number.fVal);
                }
                if (op_node->RuntimeValueType == PinValueType::Vector3) {
                    drawVariableNumber(op_node->GetCategory(), int(node.op), 3, op_node->number.fVal);
                }
                if (op_node->RuntimeValueType == PinValueType::Vector4) {
                    drawVariableNumber(op_node->GetCategory(), int(node.op), 4, op_node->number.fVal);
                }
            }
            size_t i = 0;
            for (NodeParam& input : node.params)
            {
                auto& metaInfo = v->GetInputMetaInfo(i);
                imnodes::BeginInputAttribute(int(input.id));
                const float label_width = ImGui::CalcTextSize(metaInfo.name).x;
                ImGui::Text(metaInfo.name);
                auto input_type = graph_.node(input.id)->type;
                auto hideParam = op_node->ForceHideParamNumber();
                if (input_type == Node_Number && hideParam == false) {
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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));

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
                return;
            }
        }

        static Id node_selected;
        drawNodeProperty(graph_.node(node_selected.id));
        if (int selectedId = -1; imnodes::IsNodeSelected(&selectedId))
        {
            if (selectedId != node_selected.id) {
                graph_.node(node_selected.id)->OnNodeClicked();
            }
            node_selected.id = selectedId;
            if (ImGui::IsKeyReleased(ImGui::GetIO().KeyMap[ImGuiKey_X]))
            {
                findRemoveNode(node_selected);
                return;
            }
        }

        Id link_start, link_end;
        int start, end;
        static bool hasStartLink = false;
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
                auto node_from = graph_.node(from_id);//接受Operation输出的Node
                auto node_to = graph_.node(to_id);//输出的Node
                auto opNode = findOpNodeByInput(from_id);
                if (opNode != nullptr) {
                    //判断所有的param是否兼容，如果不兼容则return
                    std::vector<PinValueType> runtimeTypes;
                    for (auto v : opNode->Id.params) {
                        runtimeTypes.push_back(graph_.node(v.id)->RuntimeValueType);
                    }
                    runtimeTypes.push_back(node_to->RuntimeValueType);//添加即将建立连接的Input类型
                    auto ret = std::remove_if(runtimeTypes.begin(), runtimeTypes.end(), [](PinValueType x) {return x == PinValueType::Any; });//忽略还没有建立连接的Input
                    runtimeTypes.resize(std::distance(runtimeTypes.begin(), ret));
                    ret = std::unique(runtimeTypes.begin(), runtimeTypes.end());
                    runtimeTypes.resize(std::distance(runtimeTypes.begin(), ret));
                    if (runtimeTypes.size() > 1) {
                        if (runtimeTypes.size() == 2 && (opNode->RuntimeValueType == PinValueType::Any || opNode->RuntimeValueType == PinValueType::Demical_Float)) {
                            if (runtimeTypes[0] == PinValueType::Float || runtimeTypes[1] == PinValueType::Float) {
                            }
                            else {
                                return;
                            }
                        }
                        else {
                            return;
                        }
                    }
                }
                //判定类型是否吻合
                graph_.add_edge(from_id, to_id);
                node_from->type = node_from->type == Node_Number
                    ? Node_NumberExpression
                    : node_from->type;
                node_from->SetRuntimeType(node_to->RuntimeValueType);

                if (opNode != nullptr) {
                    std::vector<PinValueType> pvs;
                    for (auto& p : opNode->Id.params) {
                        pvs.push_back(graph_.node(p.id)->RuntimeValueType);
                    }
                    auto merge_type = opNode->MergeOutputType(pvs);
                    if (merge_type != PinValueType::None) {
                        opNode->SetRuntimeType(merge_type);
                    }
                }
            }
            hasStartLink = false;
            return;
        }
        if (!hasStartLink && imnodes::IsLinkStarted(&start))
        {
            hasStartLink = true;
            return;
        }
        const bool rightclick_popup = ImGui::IsMouseClicked(1);
        if ((!ImGui::IsAnyItemHovered() && ImGui::IsAnyWindowHovered() && rightclick_popup)
            || (hasStartLink && imnodes::IsLinkDropped())) {
            hasStartLink = false;
            ImGui::OpenPopup("add node");
        }

        showPopupMenu();
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
