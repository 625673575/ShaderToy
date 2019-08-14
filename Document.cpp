#include "Document.h"
void ShaderStudio::TextDocument::SetupEditor()
{
    //editor.SetPalette(TextEditor::GetLightPalette());
    
    // error markers
    TextEditor::ErrorMarkers markers;
    markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
    markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
    editor.SetErrorMarkers(markers);
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF("c:/windows/fonts/Arial.ttf", 16.0f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
    IM_ASSERT(font != NULL);
    ImGui::GetIO().FontDefault = font;
    // "breakpoint" markers
    //TextEditor::Breakpoints bpts;
    //bpts.insert(24);
    //bpts.insert(47);
    //editor.SetBreakpoints(bpts);

}
ShaderStudio::TextDocument::TextDocument(Gui* pgui,const std::string& path) :Document(getFilenameFromPath(path)),  pGui(pgui), Content(std::string(ImGui::GetVersion()) + "Paste code here ...")
{
    Path = path;
}

void ShaderStudio::TextDocument::DoOpen()
{
    Content = Falcor::readFile(Path);
    editor.SetText(Content);
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
    Document::DoOpen();
}

void ShaderStudio::TextDocument::DoSave()
{
    Content = editor.GetText();
    std::ofstream out(Path);
    if (out.is_open())
    {
        out << Content;
        out.close();
    }
    else {
        Falcor::logError("can't save the file");
    }
}

void ShaderStudio::TextDocument::DisplayContents()
{
    if (!IsOpen)return;

    auto cpos = editor.GetCursorPosition();
    ImGui::Begin(Name.c_str(), nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
    ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    ImGui::SetWindowPos(ImVec2(350, 0), ImGuiCond_FirstUseEver);
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
            {
                DoSave();
                /// save text....
            }
            if (ImGui::MenuItem("Close", "Alt-F4"))
            {
                ImGui::EndMenu();
                ImGui::EndMenuBar();
                ImGui::End();
                DoForceClose();
                return;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            bool ro = editor.IsReadOnly();
            if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
                editor.SetReadOnly(ro);
            ImGui::Separator();

            if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && editor.CanUndo()))
                editor.Undo();
            if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
                editor.Redo();

            ImGui::Separator();

            if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
                editor.Copy();
            if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && editor.HasSelection()))
                editor.Cut();
            if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
                editor.Delete();
            if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
                editor.Paste();

            ImGui::Separator();

            if (ImGui::MenuItem("Select all", nullptr, nullptr))
                editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Dark palette"))
                editor.SetPalette(TextEditor::GetDarkPalette());
            if (ImGui::MenuItem("Light palette"))
                editor.SetPalette(TextEditor::GetLightPalette());
            if (ImGui::MenuItem("Retro blue palette"))
                editor.SetPalette(TextEditor::GetRetroBluePalette());
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, editor.GetTotalLines(),
        editor.IsOverwrite() ? "Ovr" : "Ins",
        editor.CanUndo() ? "*" : " ",
        editor.GetLanguageDefinition().mName.c_str(), Path);

    editor.Render("TextEditor");
    if (editor.IsTextChanged())
        IsDirty = true;
    ImGui::End();
}
