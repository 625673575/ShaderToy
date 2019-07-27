#pragma once
#include "Falcor.h"

namespace Falcor
{
    class ShaderToyPass
    {
    public:
        using UniquePtr = std::unique_ptr<ShaderToyPass>;
        using UniqueConstPtr = std::unique_ptr<const ShaderToyPass>;

        ~ShaderToyPass()
        {
            assert(gFullScreenData.objectCount > 0);

            gFullScreenData.objectCount--;
            if (gFullScreenData.objectCount == 0)
            {
                gFullScreenData.pVao = nullptr;
                gFullScreenData.pVertexBuffer = nullptr;
            }
        }
        static UniquePtr create(const std::string& psCode, const Program::DefineList& programDefines = Program::DefineList(), bool disableDepth = true, bool disableStencil = true, uint32_t viewportMask = 0, bool enableSPS = false, Shader::CompilerFlags compilerFlags = Shader::CompilerFlags::None, const std::string & shaderModel = "");

        /** Execute the pass.
            \param[in] pRenderContext The render context.
            \param[in] pDsState Optional. Use it to make the pass use a different DS state then the one created during initialization
        */
        void execute(RenderContext* pRenderContext, DepthStencilState::SharedPtr pDsState = nullptr, BlendState::SharedPtr pBlendState = nullptr) const;

        /** Get the program.
        */
        const Program::SharedConstPtr getProgram() const { return mpProgram; }
        Program::SharedPtr getProgram() { return mpProgram; }

    protected:
        ShaderToyPass() { gFullScreenData.objectCount++; }
        void init(const std::string& psCode, const Program::DefineList& programDefines, bool disableDepth, bool disableStencil, uint32_t viewportMask, bool enableSPS, Shader::CompilerFlags compilerFlags, const std::string& shaderModel);

    private:
        GraphicsProgram::SharedPtr mpProgram;
        GraphicsState::SharedPtr mpPipelineState;
        DepthStencilState::SharedPtr mpDepthStencilState;
    };
}
