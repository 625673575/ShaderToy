#pragma once
#include "Document.h"

namespace ShaderStudio {
    struct Parser {
        virtual void Format() = 0;
    };
    struct HLSLParser :Parser {
        HLSLParser(TextDocument* doc) {}
        void Format()override{}
    };
    struct OpenGLParser :Parser {
        OpenGLParser(TextDocument* doc) {}
        void Format()override {}
    };
    
    class ShaderToyDocument :
        public Document
    {
    protected:
        Gui* pGui;
    private:
        std::map<std::string,TextDocument> mTextDocuments;
        const static std::vector<std::string> ConstBufferIdentifier;
        const size_t BUFFER_SIZE = ConstBufferIdentifier.size();
        bool hasCommon;
        std::vector<std::string> availablePages;
        void createNewImageBufferFile(const std::string&);
        void createNewCommonFile(const std::string&);
        void insertIfNotExist(const std::string& pageName, const std::string& file) {
            if (mTextDocuments.find(pageName) == mTextDocuments.end())
                mTextDocuments.emplace(pageName, TextDocument(pGui, file));
        }
    public :
        std::map<std::string, TextDocument>& GetShaderCodes() { return mTextDocuments; }
        static std::string& GetPSEntry() { static std::string ps = "mainImage"; return ps; }
        static std::string& GetCommonName() { static std::string ps = "Common"; return ps; }
        static std::string& GetMainImageName() { static std::string ps = "MainImage"; return ps; }
        std::string GetCommonFullName() { auto v = mTextDocuments.find(GetCommonName()); if (v != mTextDocuments.end()) { return v->second.Path; } return ""; }
        bool HasCommonFile() { return mTextDocuments.find("Common") != mTextDocuments.end(); }
    public :
        ShaderToyDocument(Falcor::Gui* pGui, const std::string& path);
        void DoOpen() override;
        void DoSave()override;
        void DisplayContents() override;
        void AddPage(const std::string& page);
    };
}
