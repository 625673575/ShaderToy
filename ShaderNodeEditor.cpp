#include "ShaderNodeEditor.h"
#include "imnodes.h"
#include "node_editor.h"
namespace ShaderNodeEditor {
    size_t ShaderNodeEditor::variable_counter = 0;

    void ShaderNodeEditor::Init()
    {
#define ADD_POPUP_NODE_ITEM(NAME,FUNC) {#NAME, std::bind(&ShaderNodeEditor::FUNC, this)}
        click_pos = ImVec2(400.0f, 200.0f);
        addOutputDiffuse();
        popupNodesFunctions.emplace("Constant", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Float, addPopupItem_ConstantFloat),
                ADD_POPUP_NODE_ITEM(Vector3, addPopupItem_ConstantVector3)
        });

        popupNodesFunctions.emplace("Variable", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Float, addPopupItem_VariableFloat),
                ADD_POPUP_NODE_ITEM(Vector3, addPopupItem_VariableVector3)
        });
        popupNodesFunctions.emplace("Misc", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Time, addPopupItem_Time)
        });
        popupNodesFunctions.emplace("Vector", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Mask, addPopupItem_Mask),
                ADD_POPUP_NODE_ITEM(AppendChannel, addPopupItem_AppendChannel)
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
        popupNodesFunctions.emplace("Custom", std::map<std::string, std::function<void()>>{
            ADD_POPUP_NODE_ITEM(Get Variable, addPopupItem_TempVariable),
                ADD_POPUP_NODE_ITEM(Set Variable, addPopupItem_IntermediateVariable)
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

    void ShaderNodeEditor::Show()
    {
        ImGui::Begin("Visual Shader Editor", &is_open, ImVec2(1024, 640), 0.9f, ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("Eval Diffuse")) {
            graph_.evaluate(output_nodes_[0u]->Id.op);
        }
        if (ImGui::Button("Eval Variable")) {
            for (auto& v : intermediate_nodes()) {
                graph_.evaluate(v->Id.op);
            }
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
                for (const auto& param : (*iter)->Id.params) {
                    int param_id = param.id;
                    auto edge_iter = std::find_if(graph_.begin_edges(), graph_.end_edges(), [param_id](auto& x) {return x.second.from == param_id; });
                    if (edge_iter != graph_.end_edges()) {
                        auto node = graph_.node(edge_iter->second.from);
                        if (node->type == Node_NumberExpression) {
                            node->type = Node_Number;
                            graph_.erase_edge(edge_iter->first);
                        }
                    }

                    edge_iter = std::find_if(graph_.begin_edges(), graph_.end_edges(), [param_id](auto& x) {return x.second.to == param_id; });
                    if (edge_iter != graph_.end_edges()) {
                        auto node = graph_.node(edge_iter->second.to);
                        if (node->type != Node_Output) {
                            node->type = Node_Operation;
                        }
                        graph_.erase_node(edge_iter->second.to);
                    }
                }
                auto edge_iter = std::find_if(graph_.begin_edges(), graph_.end_edges(), [id](auto& x) {return x.second.to == id; });
                if (edge_iter != graph_.end_edges()) {
                    auto node = graph_.node(edge_iter->second.from);
                    if (node->type == Node_NumberExpression)
                        node->type = Node_Number;
                }
                graph_.erase_node((*iter)->Id.op);
                time_nodes_.erase(iter);
                return true;
            }
        }
        return false;
    }

    bool ShaderNodeEditor::updateEdge(NodePtr& p)
    {
        bool isUpdate = false;
        if (p->needUpdateEdge) {
            //删除多于的节点连接
            OutputDebugStringA("need update edge");
            for (auto& param : p->Id.params) {
                if (!param.value->IsValid()) {
                    auto param_id = param.id;
                    auto edge_iter = std::find_if(graph_.begin_edges(), graph_.end_edges(), [param_id](auto& x) {return x.second.from == param_id; });
                    if (edge_iter != graph_.end_edges()) {
                        auto node = graph_.node(edge_iter->second.from);
                        if (node->type == Node_NumberExpression) {
                            node->type = Node_Number;
                            graph_.erase_edge(edge_iter->first);
                            isUpdate = true;
                        }
                    }
                }
            }

            p->needUpdateEdge = false;
        }
        return isUpdate;
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
            imnodes::BeginNode(node->Id.op);
            imnodes::Name("output");

            ImGui::Dummy(ImVec2(node_width, 0.f));
            {
                size_t i = 0;
                // TODO: the color style of the pin needs to be pushed here
                for (auto in : node->Id.params) {
                    imnodes::BeginInputAttribute(int(in.id));
                    auto& meta = node->GetInputMetaInfo(i++);
                    const float label_width = ImGui::CalcTextSize(meta.name).x;
                    ImGui::Text(meta.name);
                    auto& input = graph_.node(in.id);
                    if (input->type == Node_Number)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel",
                            static_cast<float*>(input->NumberData()),
                            0.01f,
                            0.f,
                            1.0f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndAttribute();
                }
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
            updateEdge(v);

            auto& node = v->Id;
            imnodes::BeginNode(node.op);
            imnodes::Name(v->GetName());

            auto& op_node = v;
            const int node_width = op_node->GetNodeSize();

            if (op_node->ShowVariableNameEditor()) {
                ImGui::PushItemWidth(node_width);
                ImGui::InputText("Name", op_node->variableName.data(), 255,
                    ImGuiInputTextFlags_CharsNoBlank| ImGuiInputTextFlags_AutoSelectAll);
                ImGui::PopItemWidth();
            }
            if (op_node->ForceShowNumber()) {
                if (op_node->runtimeValueType == PinValueType::Float) {
                    drawVariableNumber(op_node->GetCategory(), int(node.op), 1, op_node->number.fVal);
                }
                if (op_node->runtimeValueType == PinValueType::Vector2) {
                    drawVariableNumber(op_node->GetCategory(), int(node.op), 2, op_node->number.fVal);
                }
                if (op_node->runtimeValueType == PinValueType::Vector3) {
                    drawVariableNumber(op_node->GetCategory(), int(node.op), 3, op_node->number.fVal);
                }
                if (op_node->runtimeValueType == PinValueType::Vector4) {
                    drawVariableNumber(op_node->GetCategory(), int(node.op), 4, op_node->number.fVal);
                }
            }
            size_t i = 0;
            for (NodeParam& input : node.params)
            {
                if (!input.value->IsValid())continue;

                auto& metaInfo = v->GetInputMetaInfo(i);
                assert(metaInfo.name != NULL);//if this is NULL ,check the addNode template< size_t _COUNT> 
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
                        static_cast<float*>(input.value->NumberData()),
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
                if (metaInfo.name != nullptr) {
                    imnodes::BeginOutputAttribute(int(node.op));
                    const float label_width = ImGui::CalcTextSize(metaInfo.name).x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::Text(metaInfo.name);
                    imnodes::EndAttribute();
                }
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
        if (graph_.has_node(node_selected.id)) {
            drawNodeProperty(graph_.node(node_selected.id));
        }
        if (int selectedId = -1; imnodes::IsNodeSelected(&selectedId))
        {
            if (selectedId != node_selected.id) {
                if (graph_.has_node(node_selected.id)) {
                    graph_.node(node_selected.id)->OnNodeClicked();
                }
            }
            node_selected.id = selectedId;
            if (ImGui::IsKeyReleased(ImGui::GetIO().KeyMap[ImGuiKey_X]))
            {
                graph_.node(node_selected.id)->OnNodeDestroy();
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
                        runtimeTypes.push_back(graph_.node(v.id)->runtimeValueType);
                    }
                    runtimeTypes.push_back(node_to->runtimeValueType);//添加即将建立连接的Input类型
                    auto ret = std::remove_if(runtimeTypes.begin(), runtimeTypes.end(), [](PinValueType x) {return x == PinValueType::Any; });//忽略还没有建立连接的Input
                    runtimeTypes.resize(std::distance(runtimeTypes.begin(), ret));
                    ret = std::unique(runtimeTypes.begin(), runtimeTypes.end());
                    runtimeTypes.resize(std::distance(runtimeTypes.begin(), ret));
                    if (runtimeTypes.size() > 1) {
                        if (runtimeTypes.size() == 2 && (opNode->runtimeValueType == PinValueType::Any || opNode->runtimeValueType == PinValueType::Demical_Float)) {
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
                node_from->SetRuntimeType(node_to->runtimeValueType);

                if (opNode != nullptr) {
                    std::vector<PinValueType> pvs;
                    for (auto& p : opNode->Id.params) {
                        pvs.push_back(graph_.node(p.id)->runtimeValueType);
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

    void ShaderNodeEditor::showCompositor_Mask()
    {
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
