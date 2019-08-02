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
#include "ShaderToy.h"
#include "Externals/dear_imgui/imgui.h"
ShaderToy::~ShaderToy()
{
}

void ShaderToy::onLoad(SampleCallbacks* pSample, RenderContext* pRenderContext)
{
    // create rasterizer state
    RasterizerState::Desc rsDesc;
    mpNoCullRastState = RasterizerState::create(rsDesc);

    // Depth test
    DepthStencilState::Desc dsDesc;
    dsDesc.setDepthTest(false);
    mpNoDepthDS = DepthStencilState::create(dsDesc);

    // Blend state
    BlendState::Desc blendDesc;
    mpOpaqueBS = BlendState::create(blendDesc);

    // Texture sampler
    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(Sampler::Filter::Linear, Sampler::Filter::Linear, Sampler::Filter::Linear).setMaxAnisotropy(8);
    mpLinearSampler = Sampler::create(samplerDesc);

    ShaderToyBuffer mainBuffer{ BufferType::MainPass,ShaderToyPass::create(Falcor::readFile("Data/toyContainer.hlsl")),nullptr,nullptr };

    mBufferPass.emplace(ShaderToyDocument::GetMainImageName(), std::move(mainBuffer));
    auto mpMainPass = GetMainPass();
    // Create Constant buffer
    mpToyVars = GraphicsVars::create(mpMainPass->getProgram()->getReflector());

    // Get buffer finding
    mToyCBBinding = mpMainPass->getProgram()->getReflector()->getDefaultParameterBlock()->getResourceBinding("ToyCB");

    //
    std::string docPath("F:\\Cpp\\Falcor\\Samples\\Core\\ShaderToy\\Data\\toyContainer.hlsl");
    mShaderToyDocument = std::make_unique<ShaderToyDocument>(pSample->getGui(), docPath);
    //mShaderToyDocument->DoOpen();
}

void ShaderToy::onFrameRender(SampleCallbacks* pSample, RenderContext* pRenderContext, const Fbo::SharedPtr& pTargetFbo)
{
    if (hasCompileError) {
        return;
    }
    // iResolution
    float width = (float)pTargetFbo->getWidth();
    float height = (float)pTargetFbo->getHeight();
    ParameterBlock* pDefaultBlock = mpToyVars->getDefaultBlock().get();
    auto& cb0 = pDefaultBlock->getConstantBuffer(mToyCBBinding, 0);
    cb0["iResolution"] = glm::vec2(width, height);
    float iGlobalTime = pSample->getCurrentTime();
    cb0["iTime"] = iGlobalTime;
    float iTimeDelta = pSample->getLastFrameTime();
    cb0["iTimeDelta"] = iTimeDelta;
    auto iFrame = pSample->getFrameID();
    cb0["iFrame"] = uint32_t(iFrame);
    cb0["iMouse"] = mouseCBData;
    pRenderContext->setGraphicsVars(mpToyVars);

    // run final pass

    if (isRenderBufferOnly) {
        mBufferPass[mChannelDropdownList[selectedRenderBuffer].label].pass->execute(pRenderContext);
        return;
    }

    static const glm::vec4 CLEAR_COLOR(0.f);
    GraphicsState* pState = pRenderContext->getGraphicsState().get();
    for (auto& v : mBufferPass) {
        if (v.second.type != BufferType::Image) {
            if (v.second.type == BufferType::MainPass) {
                pState->setFbo(pTargetFbo);
                pRenderContext->clearFbo(pTargetFbo.get(), CLEAR_COLOR, 1.0f, 0);
            }
            else {
                pState->pushFbo(v.second.fbo);
                pRenderContext->clearFbo(v.second.fbo.get(), CLEAR_COLOR, 1.0f, 0);
            }
            char channelParamName[] = "iChannel0";
            for (int i = 0; i < CHANNEL_COUNT; i++) {
                channelParamName[8] = i + '0';
                auto curSelect = v.second.selectedChannel[i];
                if (curSelect < mChannelDropdownList.size()) {
                    auto passKey = mChannelDropdownList[curSelect].label;
                    if (auto it = mBufferPass.find(passKey); it != mBufferPass.end()) {
                        mpToyVars->setTexture(channelParamName, it->second.getTexture());
                    }
                }
            }
            v.second.pass->execute(pRenderContext);
            if (v.second.type != BufferType::MainPass) {
                pState->popFbo();
            }
        }
    }
}

void ShaderToy::onShutdown(SampleCallbacks* pSample)
{
}

bool ShaderToy::onKeyEvent(SampleCallbacks* pSample, const KeyboardEvent& keyEvent)
{
    bool bHandled = false;
    {
        if (keyEvent.type == KeyboardEvent::Type::KeyPressed)
        {
            switch (keyEvent.key)
            {
            case KeyboardEvent::Key::Space:
                bHandled = true;

                CompileShader(pSample);
                break;
            }
        }
    }
    return bHandled;
}

bool ShaderToy::onMouseEvent(SampleCallbacks* pSample, const MouseEvent& mouseEvent)
{
    bool bHandled = false;
    if (mouseEvent.type == MouseEvent::Type::LeftButtonUp) {
        mouseCBData = glm::vec4(mouseEvent.pos.x * pSample->getWindow()->getClientAreaWidth(), mouseEvent.pos.y * pSample->getWindow()->getClientAreaHeight(), mouseEvent.wheelDelta.x, mouseEvent.wheelDelta.y);
        bHandled = true;
    }
    return bHandled;
}

void ShaderToy::onGuiRender(SampleCallbacks* pSample, Gui* pGui)
{
    pGui->setCurrentWindowSize(350, 600);
    mShaderToyDocument->DisplayContents();
    if (pGui->addButton("Recompile")) {
        mShaderToyDocument->DoSave();
        CompileShader(pSample);
    }
    if (pGui->addButton("Load Image")) {
        std::string filename;
        FileDialogFilterVec filters = { {"bmp"}, {"jpg"}, {"dds"}, {"png"}, {"tiff"}, {"tif"}, {"tga"} };
        if (openFileDialog(filters, filename))
        {
            auto  pImage = createTextureFromFile(filename, false, true);
            pImage->setName(getFilenameFromPath(filename));
            if (mBufferPass.find(pImage->getName()) == mBufferPass.end()) {
                mBufferPass.emplace(pImage->getName(), ShaderToyBuffer{ BufferType::Image, nullptr,nullptr,pImage });
                RefreshChannelGUI(pSample);
            }
        }
    }

    if (!mChannelDropdownList.empty()) {
        pGui->addCheckBox("Render Buffer(Only accept texture file as Channel Input)", isRenderBufferOnly);
    }
    if (isRenderBufferOnly) {
        pGui->addDropdown("Select Buffer", mChannelDropdownList, selectedRenderBuffer);
    }
    if (!mChannelDropdownList.empty()) {
        for (auto& pass : mBufferPass) {
            if (pass.second.type != BufferType::Image && pGui->beginGroup(pass.first + "Channel", true)) {
                std::string name = pass.first + "Channeli";
                for (int i = 0; i < CHANNEL_COUNT; i++) {
                    name[name.length()-2] = '0'+i;
                    auto& refIndex = pass.second.selectedChannel[i];
                    if (pGui->addDropdown(name.c_str(), mChannelDropdownList, refIndex)) {
                        std::string selectTexture = mChannelDropdownList[refIndex].label;
                    }
                }
                pGui->endGroup();
            }
        }
    }
}

void ShaderToy::CompileShader(SampleCallbacks* pSample)
{
    mBufferPass.clear();
    auto& shaderCodeMap = mShaderToyDocument->GetShaderCodes();
    std::string containerCode = Falcor::readFile("Data/ShaderToyContainer.hlsl");
    hasCompileError = false;
    for (auto& v : shaderCodeMap) {
        auto is_ps = v.first != mShaderToyDocument->GetCommonName();
        if (is_ps) {// find the mainImage Entry
            //insert '#include "???.h"' into the ShaderToyContainer.hlsl
            //so we read the ShaderToyContainer.hlsl,then find the "float main(" add include string before main entry.
            auto entryStartPos = containerCode.find("float4 main(");
            std::string copyCode(containerCode);
            if (mShaderToyDocument->HasCommonFile()) {
                std::string includeCommonText = Falcor::readFile(mShaderToyDocument->GetCommonFullName());
                copyCode.insert(entryStartPos, includeCommonText);
                entryStartPos += includeCommonText.size();
            }
            std::string includeImageText = Falcor::readFile(v.second.Path);
            copyCode.insert(entryStartPos, includeImageText);
            if (v.first == mShaderToyDocument->GetMainImageName()) {
                mBufferPass[ShaderToyDocument::GetMainImageName()].pass = ShaderToyPass::create(copyCode);
                mBufferPass[ShaderToyDocument::GetMainImageName()].type = BufferType::MainPass;
                auto mpMainPass = GetMainPass();
                auto version = mpMainPass->getProgram()->getActiveVersion();
                hasCompileError |= version == nullptr;
                if (!hasCompileError) {
                    mpToyVars = GraphicsVars::create(mpMainPass->getProgram()->getReflector());
                    mToyCBBinding = mpMainPass->getProgram()->getReflector()->getDefaultParameterBlock()->getResourceBinding("ToyCB");
                }
            }
            else {//Buffer
                Fbo::Desc fboDesc;
                auto width = pSample->getWindow()->getClientAreaWidth();
                auto height = pSample->getWindow()->getClientAreaHeight();
                auto mpTmpFbo = FboHelper::create2D(width, height, fboDesc);
                auto pTex = Texture::create2D(width, height, ResourceFormat::RGBA8Unorm, 1, 1, nullptr, Resource::BindFlags::RenderTarget | Resource::BindFlags::ShaderResource);
                mpTmpFbo->attachColorTarget(pTex, 0);
                ShaderToyBuffer buf{ BufferType::RenderTarget, ShaderToyPass::create(copyCode),mpTmpFbo,nullptr };
                auto version = buf.pass->getProgram()->getActiveVersion();
                hasCompileError |= version == nullptr;
                mBufferPass.emplace(v.first, std::move(buf));
            }
        }
    }

    RefreshChannelGUI(pSample);
}

void ShaderToy::RefreshChannelGUI(SampleCallbacks* pSample)
{
    mChannelDropdownList.clear();
    uint32_t i = 0;
    for (auto& v : mBufferPass) {
        if (v.first.empty() || v.second.type == BufferType::MainPass)continue;
        mChannelDropdownList.push_back(Gui::DropdownValue{ i++,v.first });
    }

}

void ShaderToy::onResizeSwapChain(SampleCallbacks* pSample, uint32_t width, uint32_t height)
{
    mAspectRatio = (float(width) / float(height));
}

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#else
int main(int argc, char** argv)
#endif
{
    ShaderToy::UniquePtr pRenderer = std::make_unique<ShaderToy>();
    SampleConfig config;
    config.windowDesc.width = 1280;
    config.windowDesc.height = 720;
    config.deviceDesc.enableVsync = true;
    config.windowDesc.resizableWindow = true;
    config.windowDesc.title = "Falcor Shader Toy";
#ifdef _WIN32
    Sample::run(config, pRenderer);
#else
    config.argc = (uint32_t)argc;
    config.argv = argv;
    Sample::run(config, pRenderer);
#endif
    return 0;
}
