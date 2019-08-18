#include "ShaderToyDocument.h"

const std::vector<std::string> ShaderStudio::ShaderToyDocument::ConstBufferIdentifier = std::vector<std::string>{ "A","B","C","D" };
ShaderStudio::ShaderToyDocument::ShaderToyDocument(Falcor::Gui* pgui, const std::string& path) : Document("ShaderToy"), pGui(pgui), hasCommon(false)
{
}

void ShaderStudio::ShaderToyDocument::DoOpen()
{
    if (!mTextDocuments.empty()) {
        DoQueueClose();
    }

    std::string mainImageFile = Path + "/MainImage.hlsl";
    if (doesFileExist(mainImageFile)) {
        insertIfNotExist(GetMainImageName(), mainImageFile);
    }
    else {
        msgBox("Not find MainImage.hlsl under " + Path);
        return;
    }
    std::string commonFile = Path + "/Common.hlsl";
    hasCommon = doesFileExist(commonFile);
    if (hasCommon) {
        insertIfNotExist(GetCommonName(), commonFile);
    }
    else {
        availablePages.push_back(GetCommonName());
    }
    std::vector<std::string> bufferFiles;

    for (auto& v : ConstBufferIdentifier) {
        std::string bufName = "Buffer" + v;
        std::string bufFileName = Path + "/" + bufName + ".hlsl";
        if (doesFileExist(bufFileName)) {
            insertIfNotExist(bufName, bufFileName);
        }
        else {
            availablePages.push_back(bufName);
        }
    }

    for (auto& v : mTextDocuments) {
        v.second.DoOpen();
    }
    Document::DoOpen();
}

void ShaderStudio::ShaderToyDocument::DoSave()
{
    for (auto& v : mTextDocuments) {
        v.second.DoSave();
    }
}

void ShaderStudio::ShaderToyDocument::DisplayContents()
{

    const FileDialogFilterVec hlslFileExtensionFilters = { { "hlsl", "HLSL Files"} };
    if (pGui->addButton("Create New Toy")) {
        std::string fileName = "";
        if (saveFileDialog(hlslFileExtensionFilters, fileName)) {
            Path = getDirectoryFromFile(fileName);
            AddPage(GetMainImageName());
            DoOpen();
        }
    }
    if (pGui->addButton("Open a Toy")) {
        std::string fileName = "";
        if (openFileDialog(hlslFileExtensionFilters, fileName)) {
            Path = getDirectoryFromFile(fileName);
            DoOpen();
        }
    }
    if (mTextDocuments.empty()) { return; }

    static const char* popupName = "select_shader_page";
    if (pGui->addButton(" + ")) {
        ImGui::OpenPopup(popupName);
    }
    //ImGui::SameLine();
    if (ImGui::BeginPopup(popupName))
    {
        ImGui::Text("Code Page");
        ImGui::Separator();
        for (size_t i = 0; i < availablePages.size(); i++)
            if (ImGui::Selectable(availablePages[i].c_str()))
                AddPage(availablePages[i]);
        ImGui::EndPopup();
    }
    for (auto& v : mTextDocuments) {
        if (pGui->addButton(v.first.c_str())) {
            v.second.DoOpen();
        }
        v.second.DisplayContents();
    }
}

void ShaderStudio::ShaderToyDocument::DoQueueClose()
{
    for (auto& v : mTextDocuments) {
        if (v.second.IsDirty) {
            MsgBoxButton btn = Falcor::msgBox("Should save the change of " + v.first , MsgBoxType::OkCancel);
            if (btn == MsgBoxButton::Ok) {
                v.second.DoSave();
            }
        }
    }
    mTextDocuments.clear();
}

void ShaderStudio::ShaderToyDocument::AddPage(const std::string& page)
{
    std::string file = Path + "/" + page + ".hlsl";
    if (page == GetCommonName()) {
        createNewCommonFile(file);
    }
    else {
        createNewImageBufferFile(file);
    }
    DoOpen();
}

void ShaderStudio::ShaderToyDocument::createNewImageBufferFile(const std::string& file)
{
    std::ofstream stream(file);
    stream << "vec4 mainImage(in vec2 fragCoord)" << std::endl;
    stream << "{" << std::endl;
    stream << "\t// Normalized pixel coordinates (from 0 to 1)" << std::endl;
    stream << "\tvec2 uv = fragCoord / iResolution.xy;" << std::endl;

    stream << "\t// Time varying pixel color" << std::endl;
    stream << "\tvec3 col = 0.5 + 0.5 * cos(iTime + uv.xyx + vec3(0, 2, 4));" << std::endl;

    stream << "\treturn vec4(col, 1.0);" << std::endl;
    stream << "} " << std::endl;
    stream.close();
}

void ShaderStudio::ShaderToyDocument::createNewCommonFile(const std::string& file)
{
    std::ofstream stream(file);
    stream << "vec4 someFunction(vec4 a, float b)" << std::endl;
    stream << "{" << std::endl;
    stream << "\treturn a + b;" << std::endl;
    stream << "}" << std::endl;
    stream.close();
}
