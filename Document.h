#pragma once
#include <string>
#include <fstream>
#include "Falcor.h"
#include "TextEditor.h"
using namespace Falcor;
namespace ShaderStudio {
    // Simplified structure to mimic a Document model
    class Document
    {
    public:
        std::string Name;           // Document title
        std::string Path;           // Target File
        bool        IsOpen;           // Set when the document is open (in this demo, we keep an array of all available documents to simplify the demo)
        bool        IsOpenPrev;       // Copy of Open from last update.
        bool        IsDirty;          // Set when the document has been modified
        bool        IsWantClose;      // Set when the document

    public:
        Document(const std::string& name)
        {
            Name = name;
            IsOpen = IsOpenPrev = false;
            IsDirty = false;
            IsWantClose = false;
        }
        virtual void DoOpen() { IsOpen = true; }
        virtual void DoQueueClose() { IsWantClose = true; }
        virtual void DoForceClose() { IsOpen = false; IsDirty = false; }
        virtual void MarkAsDirty() { IsDirty = true; }
        virtual void DoSave() { IsDirty = false; }
        virtual void DisplayContents() = 0;
    };

    class TextDocument :public Document
    {
    protected:
        Gui* pGui;
    public:
        std::string Content;
        TextEditor editor;
        void SetupEditor();
        TextDocument(Gui* pGui,const std::string& path);
        void DoOpen() override;
        void DoSave()override;
        void DisplayContents() override;
    };
}
