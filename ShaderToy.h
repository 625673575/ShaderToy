/***************************************************************************
# Copyright (c) 2015, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
#pragma once
#include "Falcor.h"
#include "ShaderToyDocument.h"
#include "ShaderToyPass.h"
using namespace Falcor;
using namespace ShaderStudio;

static const uint32_t CHANNEL_COUNT = 4;
class ShaderToy : public Renderer
{
    enum class BufferType {
        Image,
        RenderTarget,
        MainPass
    };
    struct ShaderToyBuffer {
        BufferType type;
        ShaderToyPass::UniquePtr pass;
        Fbo::SharedPtr fbo;
        Texture::SharedPtr image;
        uint32_t selectedChannel[CHANNEL_COUNT];
        const Texture::SharedPtr getTexture() {
            return type == BufferType::Image ? image : fbo->getColorTexture(0);
        }
    };
public:
    ~ShaderToy();

    void onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext) override;
    void onFrameRender(SampleCallbacks* pSample, RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo) override;
    void onShutdown(SampleCallbacks* pSample) override;
    void onResizeSwapChain(SampleCallbacks* pSample, uint32_t width, uint32_t height) override;
    bool onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent) override;
    bool onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent) override;
    void onGuiRender(SampleCallbacks* pSample, Gui* pGui) override;
private:
    Sampler::SharedPtr              mpLinearSampler;
    float                           mAspectRatio = 0;
    RasterizerState::SharedPtr      mpNoCullRastState;
    DepthStencilState::SharedPtr    mpNoDepthDS;
    BlendState::SharedPtr           mpOpaqueBS;
    GraphicsVars::SharedPtr         mpToyVars;
    ProgramReflection::BindLocation mToyCBBinding;
private:
    std::unique_ptr<ShaderToyDocument>      mShaderToyDocument;
    std::map<std::string, ShaderToyBuffer>  mBufferPass;
    Gui::DropdownList                       mChannelDropdownList;
    bool                                    hasCompileError = false;
    bool                                    isRenderBufferOnly = false;
    uint32_t                                selectedRenderBuffer;
    glm::vec4                               mouseCBData;
    void CompileShader(SampleCallbacks* pSample);
    void RefreshChannelGUI(SampleCallbacks* pSample);
    ShaderToyPass* GetMainPass() { return mBufferPass[ShaderToyDocument::GetMainImageName()].pass.get(); }
};
