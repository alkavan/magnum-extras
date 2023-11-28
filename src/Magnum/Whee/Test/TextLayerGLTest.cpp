/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023 Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include <sstream> /** @todo remove once Debug is stream-free */
#include <Corrade/Containers/StridedArrayView.h>
#include <Corrade/Containers/StringIterable.h>
#include <Corrade/PluginManager/Manager.h>
#include <Corrade/PluginManager/PluginMetadata.h>
#include <Corrade/TestSuite/Compare/Numeric.h>
#include <Corrade/Utility/ConfigurationGroup.h>
#include <Corrade/Utility/DebugStl.h> /** @todo remove once Debug is stream-free */
#include <Corrade/Utility/Path.h>
#include <Magnum/Image.h>
#include <Magnum/ImageView.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/DebugTools/CompareImage.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/OpenGLTester.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Text/AbstractFont.h>
#include <Magnum/Text/AbstractShaper.h>
#include <Magnum/Text/Alignment.h>
#include <Magnum/Text/GlyphCache.h>
#include <Magnum/Trade/AbstractImporter.h>

#include "Magnum/Whee/AbstractUserInterface.h"
#include "Magnum/Whee/Event.h"
#include "Magnum/Whee/Handle.h"
#include "Magnum/Whee/TextLayerGL.h"
#include "Magnum/Whee/TextProperties.h"

#include "configure.h"

namespace Magnum { namespace Whee { namespace Test { namespace {

struct TextLayerGLTest: GL::OpenGLTester {
    explicit TextLayerGLTest();

    void sharedConstruct();
    void sharedConstructSameStyleUniformCount();
    /* NoCreate tested in TextLayerGL_Test to verify it works without a GL
       context */
    void sharedConstructCopy();
    void sharedConstructMove();

    void sharedSetGlyphCache();
    void sharedSetGlyphCacheTakeOwnership();

    void construct();
    void constructCopy();
    void constructMove();

    void drawNoSizeSet();
    void drawNoStyleSet();

    void renderSetup();
    void renderTeardown();
    void render();
    void renderAlignmentPadding();
    void renderCustomColor();
    void renderChangeText();
    void renderChangeStyle();

    void drawSetup();
    void drawTeardown();
    void drawOrder();
    void drawClipping();

    void eventStyleTransition();

    private:
        PluginManager::Manager<Text::AbstractFont> _fontManager;
        PluginManager::Manager<Trade::AbstractImporter> _importerManager;
        GL::Texture2D _color{NoCreate};
        GL::Framebuffer _framebuffer{NoCreate};

        /* stb_truetype's rasterization is extremely slow, so the cache filling
           is done just once for all tests that need it; thus also the font has
           to be shared among all */
        Containers::Pointer<Text::AbstractFont> _font;
        Text::GlyphCache _fontGlyphCache{{64, 64}};
};

using namespace Math::Literals;

const struct {
    const char* name;
    const char* filename;
    TextLayerStyleUniform styleUniform;
} RenderData[]{
    {"default", "default.png",
        TextLayerStyleUniform{}},
    {"colored", "colored.png",
        TextLayerStyleUniform{}
            .setColor(0x3bd267_rgbf)},
    /** @todo test at least toggling kerning once StbTrueTypeFont supports
        that */
};

constexpr Vector2i RenderSize{128, 64};

/* Bounding box reported by Text::renderLineGlyphPositionsInto(). May change
   when StbTrueTypeFont gets kerning implemented, a different font or a
   different text is used. */
constexpr Range2D RenderAlignmentBoundingBox{{0.0f, -9.26651f}, {84.6205f, 33.4002f}};

const struct {
    const char* name;
    Text::Alignment alignment;
    bool partialUpdate;
    Vector2 nodeOffset, nodeSize;
    Vector4 paddingFromStyle, paddingFromData;
} RenderAlignmentPaddingData[]{
    /* Same as the "default" in RenderData */
    {"middle center, no padding", Text::Alignment::MiddleCenter, false,
        {8.0f, 8.0f}, {112.0f, 48.0f}, {}, {}},
    /* Deliberately having one excessively shifted to left/top and the other to
       bottom/right. It shouldn't cause any strange artifacts. */
    {"middle center, padding from style", Text::Alignment::MiddleCenter, false,
        {-64.0f, -128.0f}, {192.0f, 192.0f},
        {72.0f, 136.0f, 8.0f, 8.0f}, {}},
    {"middle center, padding from data", Text::Alignment::MiddleCenter, false,
        {0.0f, 0.0f}, {192.0f, 192.0f},
        {}, {8.0f, 8.0f, 72.0f, 136.0f}},
    {"middle center, padding from both", Text::Alignment::MiddleCenter, false,
        {0.0f, 0.0f}, {128.0f, 64.0f},
        {4.0f, 8.0f, 0.0f, 4.0f}, {4.0f, 0.0f, 8.0f, 4.0f}},
    {"middle center, padding from both, partial update", Text::Alignment::MiddleCenter, true,
        {0.0f, 0.0f}, {128.0f, 64.0f},
        {4.0f, 8.0f, 0.0f, 4.0f}, {4.0f, 0.0f, 8.0f, 4.0f}},
    /* The size isn't used for anything in this case so can be excessive */
    {"top left, no padding", Text::Alignment::TopLeft, false,
        (Vector2{RenderSize} - RenderAlignmentBoundingBox.size())/2.0f, {256.0f, 128.0f},
        {}, {}},
    {"top left, padding from data", Text::Alignment::TopLeft, false,
        {0.0f, 0.0f}, {256.0f, 128.0f},
        {}, {(RenderSize.x() - RenderAlignmentBoundingBox.size().x())/2.0f,
             (RenderSize.y() - RenderAlignmentBoundingBox.size().y())/2.0f,
             0.0f, 0.0f}},
    /* The min offset isn't used for anything in this case so can be
       excessive */
    {"bottom right, no padding", Text::Alignment::BottomRight, false,
        {-128.0f, -256.0f}, Vector2{128.0f, 256.0f} + (Vector2{RenderSize} + RenderAlignmentBoundingBox.size())/2.0f,
        {}, {}},
    {"bottom right, padding from style", Text::Alignment::BottomRight, false,
        {-128.0f, -256.0f}, Vector2{256.0f, 512.0f} + (Vector2{RenderSize} + RenderAlignmentBoundingBox.size())/2.0f,
        {0.0f, 0.0f, 128.0f, 256.0f}, {}},
    {"line right, no padding", Text::Alignment::LineRight, false,
        {0.0f, RenderSize.y()/2.0f + RenderAlignmentBoundingBox.max().y() - RenderAlignmentBoundingBox.sizeY()},
        {(RenderSize.x() + RenderAlignmentBoundingBox.sizeX())/2.0f, RenderAlignmentBoundingBox.sizeY()},
        {}, {}},
    {"line right, padding from both", Text::Alignment::LineRight, false,
        {0.0f, -RenderAlignmentBoundingBox.sizeY()},
        {(RenderSize.x() + RenderAlignmentBoundingBox.sizeX())/2.0f, RenderAlignmentBoundingBox.sizeY() + RenderSize.y()/2.0f + RenderAlignmentBoundingBox.max().y()},
        {0.0f, RenderSize.y()/2.0f, 0.0f, 0.0f},
        {0.0f, RenderAlignmentBoundingBox.max().y(), 0.0f, 0.0f}},
};

const struct {
    const char* name;
    bool setLater;
    bool partialUpdate;
} RenderCustomColorData[]{
    {"", false, false},
    {"set later", true, false},
    {"set later, partial update", true, true},
};

const struct {
    const char* name;
    bool partialUpdate;
} RenderChangeStyleTextData[]{
    {"", false},
    {"partial update", true},
};

const struct {
    const char* name;
    bool dataInNodeOrder;
} DrawOrderData[]{
    {"data created in node order", true},
    {"data created randomly", false}
};

const struct {
    const char* name;
    const char* filename;
    bool clip;
    bool singleTopLevel;
    bool flipOrder;
} DrawClippingData[]{
    {"clipping disabled", "clipping-disabled.png",
        false, false, false},
    {"clipping top-level nodes", "clipping-enabled.png",
        true, false, false},
    {"clipping top-level nodes, different node order", "clipping-enabled.png",
        true, false, true},
    {"single top-level node with clipping subnodes", "clipping-enabled.png",
        true, true, false},
};

TextLayerGLTest::TextLayerGLTest() {
    addTests({&TextLayerGLTest::sharedConstruct,
              &TextLayerGLTest::sharedConstructSameStyleUniformCount,
              &TextLayerGLTest::sharedConstructCopy,
              &TextLayerGLTest::sharedConstructMove,

              &TextLayerGLTest::sharedSetGlyphCache,
              &TextLayerGLTest::sharedSetGlyphCacheTakeOwnership,

              &TextLayerGLTest::construct,
              &TextLayerGLTest::constructCopy,
              &TextLayerGLTest::constructMove,

              &TextLayerGLTest::drawNoSizeSet,
              &TextLayerGLTest::drawNoStyleSet});

    addInstancedTests({&TextLayerGLTest::render},
        Containers::arraySize(RenderData),
        &TextLayerGLTest::renderSetup,
        &TextLayerGLTest::renderTeardown);

    addInstancedTests({&TextLayerGLTest::renderAlignmentPadding},
        Containers::arraySize(RenderAlignmentPaddingData),
        &TextLayerGLTest::renderSetup,
        &TextLayerGLTest::renderTeardown);

    addInstancedTests({&TextLayerGLTest::renderCustomColor},
        Containers::arraySize(RenderCustomColorData),
        &TextLayerGLTest::renderSetup,
        &TextLayerGLTest::renderTeardown);

    addInstancedTests({&TextLayerGLTest::renderChangeStyle,
                       &TextLayerGLTest::renderChangeText},
        Containers::arraySize(RenderChangeStyleTextData),
        &TextLayerGLTest::renderSetup,
        &TextLayerGLTest::renderTeardown);

    addInstancedTests({&TextLayerGLTest::drawOrder},
        Containers::arraySize(DrawOrderData),
        &TextLayerGLTest::drawSetup,
        &TextLayerGLTest::drawTeardown);

    addInstancedTests({&TextLayerGLTest::drawClipping},
        Containers::arraySize(DrawClippingData),
        &TextLayerGLTest::drawSetup,
        &TextLayerGLTest::drawTeardown);

    addTests({&TextLayerGLTest::eventStyleTransition},
        &TextLayerGLTest::renderSetup,
        &TextLayerGLTest::renderTeardown);

    /* Prefer the StbImageImporter so we can keep files small but always import
       them as four-channel */
    if(PluginManager::PluginMetadata* metadata = _importerManager.metadata("StbImageImporter")) {
        metadata->configuration().setValue("forceChannelCount", 4);
            _importerManager.setPreferredPlugins("PngImporter", {"StbImageImporter"});
    }

    /* Open the font and pre-fill the glyph cache so each test iteration
       doesn't have to suffer stb_truetype's extreme rasterization slowness
       again and again. They only check that the font was opened afterwards. */
    if((_font = _fontManager.loadAndInstantiate("StbTrueTypeFont")) &&
       _font->openFile(Utility::Path::join(UI_DIR, "SourceSansPro-Regular.ttf"), 32.0f))
        _font->fillGlyphCache(_fontGlyphCache, "Magi");
}

void TextLayerGLTest::sharedConstruct() {
    TextLayerGL::Shared shared{3, 5};
    CORRADE_COMPARE(shared.styleUniformCount(), 3);
    CORRADE_COMPARE(shared.styleCount(), 5);
}

void TextLayerGLTest::sharedConstructSameStyleUniformCount() {
    TextLayerGL::Shared shared{3};
    CORRADE_COMPARE(shared.styleUniformCount(), 3);
    CORRADE_COMPARE(shared.styleCount(), 3);
}

void TextLayerGLTest::sharedConstructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<TextLayerGL::Shared>{});
    CORRADE_VERIFY(!std::is_copy_assignable<TextLayerGL::Shared>{});
}

void TextLayerGLTest::sharedConstructMove() {
    TextLayerGL::Shared a{3};

    TextLayerGL::Shared b{Utility::move(a)};
    CORRADE_COMPARE(b.styleCount(), 3);

    TextLayerGL::Shared c{5};
    c = Utility::move(b);
    CORRADE_COMPARE(c.styleCount(), 3);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<TextLayerGL::Shared>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<TextLayerGL::Shared>::value);
}

void TextLayerGLTest::sharedSetGlyphCache() {
    Text::GlyphCache cache{{32, 32}};
    CORRADE_VERIFY(cache.texture().id());

    {
        TextLayerGL::Shared shared{3};
        shared.setGlyphCache(cache);
        CORRADE_COMPARE(&shared.glyphCache(), &cache);
    }

    /* It shouldn't get accidentally moved in and deleted */
    CORRADE_VERIFY(cache.texture().id());
}

void TextLayerGLTest::sharedSetGlyphCacheTakeOwnership() {
    Text::GlyphCache cache{{32, 32}};
    CORRADE_VERIFY(cache.texture().id());

    {
        TextLayerGL::Shared shared{3};
        shared.setGlyphCache(Utility::move(cache));

        /* It should get moved in */
        CORRADE_VERIFY(!cache.texture().id());
        CORRADE_VERIFY(&shared.glyphCache() != &cache);
        CORRADE_COMPARE(shared.glyphCache().size(), (Vector3i{32, 32, 1}));
    }

    /** @todo any way to check that a deletion happened? */
}

void TextLayerGLTest::construct() {
    TextLayerGL::Shared shared{3};

    TextLayerGL layer{layerHandle(137, 0xfe), shared};
    CORRADE_COMPARE(layer.handle(), layerHandle(137, 0xfe));
    CORRADE_COMPARE(&layer.shared(), &shared);
    /* Const overload */
    CORRADE_COMPARE(&static_cast<const TextLayerGL&>(layer).shared(), &shared);
}

void TextLayerGLTest::constructCopy() {
    CORRADE_VERIFY(!std::is_copy_constructible<TextLayerGL>{});
    CORRADE_VERIFY(!std::is_copy_assignable<TextLayerGL>{});
}

void TextLayerGLTest::constructMove() {
    TextLayerGL::Shared shared{3};
    TextLayerGL::Shared shared2{5};

    TextLayerGL a{layerHandle(137, 0xfe), shared};

    TextLayerGL b{Utility::move(a)};
    CORRADE_COMPARE(b.handle(), layerHandle(137, 0xfe));
    CORRADE_COMPARE(&b.shared(), &shared);

    TextLayerGL c{layerHandle(0, 2), shared2};
    c = Utility::move(b);
    CORRADE_COMPARE(c.handle(), layerHandle(137, 0xfe));
    CORRADE_COMPARE(&c.shared(), &shared);

    CORRADE_VERIFY(std::is_nothrow_move_constructible<TextLayerGL>::value);
    CORRADE_VERIFY(std::is_nothrow_move_assignable<TextLayerGL>::value);
}

void TextLayerGLTest::drawNoSizeSet() {
    CORRADE_SKIP_IF_NO_ASSERT();

    TextLayerGL::Shared shared{3};
    TextLayerGL layer{layerHandle(0, 1), shared};

    std::ostringstream out;
    Error redirectError{&out};
    layer.draw({}, 0, 0, {}, {}, 0, 0, {}, {}, {}, {});
    CORRADE_COMPARE(out.str(), "Whee::TextLayerGL::draw(): user interface size wasn't set\n");
}

void TextLayerGLTest::drawNoStyleSet() {
    CORRADE_SKIP_IF_NO_ASSERT();

    TextLayerGL::Shared shared{3};
    TextLayerGL layer{layerHandle(0, 1), shared};

    layer.setSize({10, 10}, {10, 10});

    std::ostringstream out;
    Error redirectError{&out};
    layer.draw({}, 0, 0, {}, {}, 0, 0, {}, {}, {}, {});
    CORRADE_COMPARE(out.str(), "Whee::TextLayerGL::draw(): no style data was set\n");
}

void TextLayerGLTest::renderSetup() {
    _color = GL::Texture2D{};
    _color.setStorage(1, GL::TextureFormat::RGBA8, RenderSize);
    _framebuffer = GL::Framebuffer{{{}, RenderSize}};
    _framebuffer
        .attachTexture(GL::Framebuffer::ColorAttachment{0}, _color, 0)
        .clear(GL::FramebufferClear::Color)
        .bind();

    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
}

void TextLayerGLTest::renderTeardown() {
    _framebuffer = GL::Framebuffer{NoCreate};
    _color = GL::Texture2D{NoCreate};

    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);
}

void TextLayerGLTest::render() {
    auto&& data = RenderData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    if(!(_fontManager.load("StbTrueTypeFont") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("StbTrueTypeFont plugin not found.");

    AbstractUserInterface ui{RenderSize};

    /* Opened in the constructor together with cache filling to circumvent
       stb_truetype's extreme rasterization slowness */
    CORRADE_VERIFY(_font && _font->isOpened());

    /* Testing the ArrayView overload, others cases use the initializer list */
    TextLayerStyleUniform styleUniforms[]{
        /* To verify it's not always picking the first uniform */
        TextLayerStyleUniform{},
        TextLayerStyleUniform{},
        data.styleUniform
    };
    UnsignedInt styleToUniform[]{
        /* To verify it's not using the style ID as uniform ID */
        1, 2, 0, 1, 0
    };
    TextLayerGL::Shared layerShared{UnsignedInt(Containers::arraySize(styleUniforms)), UnsignedInt(Containers::arraySize(styleToUniform))};
    layerShared.setGlyphCache(_fontGlyphCache);
    FontHandle fontHandle[]{
        layerShared.addFont(*_font, 32.0f)
    };
    /* The (lack of any) effect of padding on rendered output is tested
       thoroughly in renderAlignmentPadding() */
    layerShared.setStyle(TextLayerCommonStyleUniform{},
        styleUniforms,
        styleToUniform,
        Containers::stridedArrayView(fontHandle).broadcasted<0>(5),
        {});

    LayerHandle layer = ui.createLayer();
    ui.setLayerInstance(Containers::pointer<TextLayerGL>(layer, layerShared));

    NodeHandle node = ui.createNode({8.0f, 8.0f}, {112.0f, 48.0f});
    /* Using a text that has glyphs both above and below line and doesn't need
       too many glyphs */
    ui.layer<TextLayerGL>(layer).create(1, "Maggi", {}, node);

    ui.draw();

    MAGNUM_VERIFY_NO_GL_ERROR();

    if(!(_importerManager.load("AnyImageImporter") & PluginManager::LoadState::Loaded) ||
       !(_importerManager.load("StbImageImporter") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("AnyImageImporter / StbImageImporter plugins not found.");

    #if defined(MAGNUM_TARGET_GLES) && !defined(MAGNUM_TARGET_WEBGL)
    /* Same problem is with all builtin shaders, so this doesn't seem to be a
       bug in the text layer shader code */
    if(GL::Context::current().detectedDriver() & GL::Context::DetectedDriver::SwiftShader)
        CORRADE_SKIP("UBOs with dynamically indexed arrays don't seem to work on SwiftShader, can't test.");
    #endif
    CORRADE_COMPARE_WITH(_framebuffer.read({{}, RenderSize}, {PixelFormat::RGBA8Unorm}),
        Utility::Path::join({WHEE_TEST_DIR, "TextLayerTestFiles", data.filename}),
        DebugTools::CompareImageToFile{_importerManager});
}

void TextLayerGLTest::renderAlignmentPadding() {
    auto&& data = RenderAlignmentPaddingData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    if(!(_fontManager.load("StbTrueTypeFont") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("StbTrueTypeFont plugin not found.");

    AbstractUserInterface ui{RenderSize};

    /* Opened in the constructor together with cache filling to circumvent
       stb_truetype's extreme rasterization slowness */
    CORRADE_VERIFY(_font && _font->isOpened());

    TextLayerGL::Shared layerShared{1};
    layerShared.setGlyphCache(_fontGlyphCache);

    FontHandle fontHandle = layerShared.addFont(*_font, 32.0f);
    layerShared.setStyle(TextLayerCommonStyleUniform{},
        {TextLayerStyleUniform{}},
        {fontHandle},
        {data.paddingFromStyle});

    LayerHandle layer = ui.createLayer();
    ui.setLayerInstance(Containers::pointer<TextLayerGL>(layer, layerShared));

    NodeHandle node = ui.createNode(data.nodeOffset, data.nodeSize);
    DataHandle nodeData = ui.layer<TextLayerGL>(layer).create(0, "Maggi",
        TextProperties{}.setAlignment(data.alignment),
        node);

    if(data.partialUpdate) {
        ui.update();
        CORRADE_COMPARE(ui.state(), UserInterfaceStates{});
    }

    if(!data.paddingFromData.isZero()) {
        ui.layer<TextLayerGL>(layer).setPadding(nodeData, data.paddingFromData);
        CORRADE_COMPARE_AS(ui.state(),
            UserInterfaceState::NeedsDataUpdate,
            TestSuite::Compare::GreaterOrEqual);
    }

    ui.draw();

    MAGNUM_VERIFY_NO_GL_ERROR();

    if(!(_importerManager.load("AnyImageImporter") & PluginManager::LoadState::Loaded) ||
       !(_importerManager.load("StbImageImporter") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("AnyImageImporter / StbImageImporter plugins not found.");

    #if defined(MAGNUM_TARGET_GLES) && !defined(MAGNUM_TARGET_WEBGL)
    /* Same problem is with all builtin shaders, so this doesn't seem to be a
       bug in the text layer shader code */
    if(GL::Context::current().detectedDriver() & GL::Context::DetectedDriver::SwiftShader)
        CORRADE_SKIP("UBOs with dynamically indexed arrays don't seem to work on SwiftShader, can't test.");
    #endif
    CORRADE_COMPARE_WITH(_framebuffer.read({{}, RenderSize}, {PixelFormat::RGBA8Unorm}),
        Utility::Path::join(WHEE_TEST_DIR, "TextLayerTestFiles/default.png"),
        DebugTools::CompareImageToFile{_importerManager});
}

void TextLayerGLTest::renderCustomColor() {
    auto&& data = RenderCustomColorData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Basically the same as the "colored" case in render(), except that the
       color is additionally taken from the per-vertex data as well */

    if(!(_fontManager.load("StbTrueTypeFont") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("StbTrueTypeFont plugin not found.");

    AbstractUserInterface ui{RenderSize};

    /* Opened in the constructor together with cache filling to circumvent
       stb_truetype's extreme rasterization slowness */
    CORRADE_VERIFY(_font && _font->isOpened());

    TextLayerGL::Shared layerShared{1};
    layerShared.setGlyphCache(_fontGlyphCache);

    FontHandle fontHandle = layerShared.addFont(*_font, 32.0f);
    layerShared.setStyle(TextLayerCommonStyleUniform{},
        {TextLayerStyleUniform{}
            .setColor(0x3bd267_rgbf/0x336699_rgbf)},
        {fontHandle},
        {});

    LayerHandle layer = ui.createLayer();
    ui.setLayerInstance(Containers::pointer<TextLayerGL>(layer, layerShared));

    NodeHandle node = ui.createNode({8.0f, 8.0f}, {112.0f, 48.0f});
    DataHandle nodeData = data.setLater ?
        ui.layer<TextLayerGL>(layer).create(0, "Maggi", {}, node) :
        ui.layer<TextLayerGL>(layer).create(0, "Maggi", {}, 0x336699_rgbf, node);

    if(data.partialUpdate) {
        ui.update();
        CORRADE_COMPARE(ui.state(), UserInterfaceStates{});
    }

    if(data.setLater) {
        ui.layer<TextLayerGL>(layer).setColor(nodeData, 0x336699_rgbf);
        CORRADE_COMPARE_AS(ui.state(),
            UserInterfaceState::NeedsDataUpdate,
            TestSuite::Compare::GreaterOrEqual);
    }

    ui.draw();

    MAGNUM_VERIFY_NO_GL_ERROR();

    if(!(_importerManager.load("AnyImageImporter") & PluginManager::LoadState::Loaded) ||
       !(_importerManager.load("StbImageImporter") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("AnyImageImporter / StbImageImporter plugins not found.");

    #if defined(MAGNUM_TARGET_GLES) && !defined(MAGNUM_TARGET_WEBGL)
    /* Same problem is with all builtin shaders, so this doesn't seem to be a
       bug in the text layer shader code */
    if(GL::Context::current().detectedDriver() & GL::Context::DetectedDriver::SwiftShader)
        CORRADE_SKIP("UBOs with dynamically indexed arrays don't seem to work on SwiftShader, can't test.");
    #endif
    CORRADE_COMPARE_WITH(_framebuffer.read({{}, RenderSize}, {PixelFormat::RGBA8Unorm}),
        Utility::Path::join(WHEE_TEST_DIR, "TextLayerTestFiles/colored.png"),
        DebugTools::CompareImageToFile{_importerManager});
}

void TextLayerGLTest::renderChangeStyle() {
    auto&& data = RenderChangeStyleTextData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Basically the same as the "colored" case in render(), except that the
       style ID is changed to it only later. */

    if(!(_fontManager.load("StbTrueTypeFont") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("StbTrueTypeFont plugin not found.");

    AbstractUserInterface ui{RenderSize};

    /* Opened in the constructor together with cache filling to circumvent
       stb_truetype's extreme rasterization slowness */
    CORRADE_VERIFY(_font && _font->isOpened());

    TextLayerGL::Shared layerShared{2};
    layerShared
        .setGlyphCache(_fontGlyphCache);

    FontHandle fontHandle = layerShared.addFont(*_font, 32.0f);
    layerShared.setStyle(TextLayerCommonStyleUniform{},
        {TextLayerStyleUniform{},
         TextLayerStyleUniform{}
            .setColor(0x3bd267_rgbf)},
        {fontHandle, fontHandle},
        {});

    LayerHandle layer = ui.createLayer();
    ui.setLayerInstance(Containers::pointer<TextLayerGL>(layer, layerShared));

    NodeHandle node = ui.createNode({8.0f, 8.0f}, {112.0f, 48.0f});
    DataHandle nodeData = ui.layer<TextLayerGL>(layer).create(0, "Maggi", {}, node);

    if(data.partialUpdate) {
        ui.update();
        CORRADE_COMPARE(ui.state(), UserInterfaceStates{});
    }

    ui.layer<TextLayerGL>(layer).setStyle(nodeData, 1);
    CORRADE_COMPARE_AS(ui.state(),
        UserInterfaceState::NeedsDataUpdate,
        TestSuite::Compare::GreaterOrEqual);

    ui.draw();

    MAGNUM_VERIFY_NO_GL_ERROR();

    if(!(_importerManager.load("AnyImageImporter") & PluginManager::LoadState::Loaded) ||
       !(_importerManager.load("StbImageImporter") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("AnyImageImporter / StbImageImporter plugins not found.");

    #if defined(MAGNUM_TARGET_GLES) && !defined(MAGNUM_TARGET_WEBGL)
    /* Same problem is with all builtin shaders, so this doesn't seem to be a
       bug in the text layer shader code */
    if(GL::Context::current().detectedDriver() & GL::Context::DetectedDriver::SwiftShader)
        CORRADE_SKIP("UBOs with dynamically indexed arrays don't seem to work on SwiftShader, can't test.");
    #endif
    CORRADE_COMPARE_WITH(_framebuffer.read({{}, RenderSize}, {PixelFormat::RGBA8Unorm}),
        Utility::Path::join(WHEE_TEST_DIR, "TextLayerTestFiles/colored.png"),
        DebugTools::CompareImageToFile{_importerManager});
}

void TextLayerGLTest::renderChangeText() {
    auto&& data = RenderChangeStyleTextData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Basically the same as the "default" case in render(), except that the
       text is changed only subsequently. */

    if(!(_fontManager.load("StbTrueTypeFont") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("StbTrueTypeFont plugin not found.");

    AbstractUserInterface ui{RenderSize};

    /* Opened in the constructor together with cache filling to circumvent
       stb_truetype's extreme rasterization slowness */
    CORRADE_VERIFY(_font && _font->isOpened());

    TextLayerGL::Shared layerShared{1};
    layerShared.setGlyphCache(_fontGlyphCache);

    FontHandle fontHandle = layerShared.addFont(*_font, 32.0f);
    layerShared.setStyle(TextLayerCommonStyleUniform{},
        {TextLayerStyleUniform{}},
        {fontHandle},
        {});

    LayerHandle layer = ui.createLayer();
    ui.setLayerInstance(Containers::pointer<TextLayerGL>(layer, layerShared));

    NodeHandle node = ui.createNode({8.0f, 8.0f}, {112.0f, 48.0f});
    DataHandle nodeData = ui.layer<TextLayerGL>(layer).create(0, "gM!", {}, node);

    if(data.partialUpdate) {
        ui.update();
        CORRADE_COMPARE(ui.state(), UserInterfaceStates{});
    }

    ui.layer<TextLayerGL>(layer).setText(nodeData, "Maggi", {});
    CORRADE_COMPARE_AS(ui.state(),
        UserInterfaceState::NeedsDataUpdate,
        TestSuite::Compare::GreaterOrEqual);

    ui.draw();

    MAGNUM_VERIFY_NO_GL_ERROR();

    if(!(_importerManager.load("AnyImageImporter") & PluginManager::LoadState::Loaded) ||
       !(_importerManager.load("StbImageImporter") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("AnyImageImporter / StbImageImporter plugins not found.");

    #if defined(MAGNUM_TARGET_GLES) && !defined(MAGNUM_TARGET_WEBGL)
    /* Same problem is with all builtin shaders, so this doesn't seem to be a
       bug in the text layer shader code */
    if(GL::Context::current().detectedDriver() & GL::Context::DetectedDriver::SwiftShader)
        CORRADE_SKIP("UBOs with dynamically indexed arrays don't seem to work on SwiftShader, can't test.");
    #endif
    CORRADE_COMPARE_WITH(_framebuffer.read({{}, RenderSize}, {PixelFormat::RGBA8Unorm}),
        Utility::Path::join(WHEE_TEST_DIR, "TextLayerTestFiles/default.png"),
        DebugTools::CompareImageToFile{_importerManager});
}

constexpr Vector2i DrawSize{64, 64};

void TextLayerGLTest::drawSetup() {
    _color = GL::Texture2D{};
    _color.setStorage(1, GL::TextureFormat::RGBA8, DrawSize);
    _framebuffer = GL::Framebuffer{{{}, DrawSize}};
    _framebuffer
        .attachTexture(GL::Framebuffer::ColorAttachment{0}, _color, 0)
        .clear(GL::FramebufferClear::Color)
        .bind();

    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::enable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::enable(GL::Renderer::Feature::Blending);
    GL::Renderer::setBlendFunction(GL::Renderer::BlendFunction::One, GL::Renderer::BlendFunction::OneMinusSourceAlpha);
}

void TextLayerGLTest::drawTeardown() {
    _framebuffer = GL::Framebuffer{NoCreate};
    _color = GL::Texture2D{NoCreate};

    GL::Renderer::disable(GL::Renderer::Feature::FaceCulling);
    GL::Renderer::disable(GL::Renderer::Feature::ScissorTest);
    GL::Renderer::disable(GL::Renderer::Feature::Blending);
}

void TextLayerGLTest::drawOrder() {
    auto&& data = DrawOrderData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Based on BaseLayerGLTest::drawOrder(), with additional variability due
       to each text having a different size */

    AbstractUserInterface ui{DrawSize};

    /* A font that just produces glyph ID 0 spaced 16 units apart */
    struct: Text::AbstractFont {
        Text::FontFeatures doFeatures() const override { return {}; }
        bool doIsOpened() const override { return _opened; }
        Properties doOpenFile(Containers::StringView, Float) override {
            _opened = true;
            return {8.0f, 8.0f, -8.0f, 16.0f, 1};
        }
        void doClose() override { _opened = false; }

        void doGlyphIdsInto(const Containers::StridedArrayView1D<const char32_t>&, const Containers::StridedArrayView1D<UnsignedInt>&) override {}
        Vector2 doGlyphSize(UnsignedInt) override { return {}; }
        Vector2 doGlyphAdvance(UnsignedInt) override { return {}; }
        Containers::Pointer<Text::AbstractShaper> doCreateShaper() override {
            struct Shaper: Text::AbstractShaper {
                using Text::AbstractShaper::AbstractShaper;

                UnsignedInt doShape(Containers::StringView text, UnsignedInt, UnsignedInt, Containers::ArrayView<const Text::FeatureRange>) override {
                    return text.size();
                }
                void doGlyphIdsInto(const Containers::StridedArrayView1D<UnsignedInt>& ids) const override {
                    for(std::size_t i = 0; i != ids.size(); ++i)
                        ids[i] = 0;
                }
                void doGlyphOffsetsAdvancesInto(const Containers::StridedArrayView1D<Vector2>& offsets, const Containers::StridedArrayView1D<Vector2>& advances) const override {
                    for(std::size_t i = 0; i != offsets.size(); ++i) {
                        offsets[i] = {0.0f, -8.0f};
                        advances[i] = {8.0f, 0.0f};
                    }
                }
                void doGlyphClustersInto(const Containers::StridedArrayView1D<UnsignedInt>&) const override {
                    /** @todo implement when it actually does get called for
                        cursor / selection */
                    CORRADE_FAIL("This shouldn't be called.");
                }
            };
            return Containers::pointer<Shaper>(*this);
        }

        bool _opened = false;
    } font;
    font.openFile({}, 8.0f);

    /* A full-white glyph cache, containing just one 7x16 glyph. Default
       padding is 1, resetting to 0 to make this work. */
    Text::GlyphCache cache{{8, 16}, {}};
    for(auto row: cache.image().pixels<UnsignedByte>()[0])
        for(UnsignedByte& pixel: row)
            pixel = 255;
    cache.flushImage({{}, {8, 16}});
    cache.addGlyph(cache.addFont(1, &font), 0, {}, {{}, {7, 16}});

    TextLayerGL::Shared layerShared{3, 4};
    layerShared.setGlyphCache(cache);

    FontHandle fontHandleLarge = layerShared.addFont(font, 16.0f);
    FontHandle fontHandleSmall = layerShared.addFont(font, 8.0f);
    /* Testing the styleToUniform initializer list overload, others cases use
       implicit mapping initializer list overloads */
    layerShared.setStyle(TextLayerCommonStyleUniform{}, {
        TextLayerStyleUniform{}
            .setColor(0xff0000_rgbf),
        TextLayerStyleUniform{}
            .setColor(0x00ff00_rgbf),
        TextLayerStyleUniform{}
            .setColor(0x0000ff_rgbf)
    }, {
        0, /* 0, red large */
        0, /* 1, red small */
        1, /* 2, green large */
        2, /* 3, blue small */
    }, {
        fontHandleLarge,
        fontHandleSmall,
        fontHandleLarge,
        fontHandleSmall
    }, {});

    LayerHandle layer = ui.createLayer();
    ui.setLayerInstance(Containers::pointer<TextLayerGL>(layer, layerShared));

    NodeHandle topLevelOnTopGreen = ui.createNode({12.0f, 8.0f}, {32.0f, 32.0f});

    NodeHandle topLevelBelowRed = ui.createNode({28.0f, 24.0f}, {32.0f, 32.0f});
    ui.setNodeOrder(topLevelBelowRed, topLevelOnTopGreen);

    NodeHandle topLevelHiddenBlue = ui.createNode({24.0f, 8.0f}, {32.0f, 32.0f}, NodeFlag::Hidden);

    NodeHandle childBelowBlue = ui.createNode(topLevelOnTopGreen, {13.0f, 4.0f}, {16.0f, 16.0f});
    NodeHandle childAboveRed = ui.createNode(childBelowBlue, {-7.0f, 8.0f}, {16.0f, 16.0f});

    if(data.dataInNodeOrder) {
        ui.layer<TextLayerGL>(layer).create(0, "ab", {}, topLevelBelowRed);
        ui.layer<TextLayerGL>(layer).create(2, "abc", {}, topLevelOnTopGreen);
        ui.layer<TextLayerGL>(layer).create(3, "abcdef", {}, topLevelHiddenBlue);
        ui.layer<TextLayerGL>(layer).create(3, "abcd", {}, childBelowBlue);
        ui.layer<TextLayerGL>(layer).create(1, "abcde", {}, childAboveRed);
    } else {
        ui.layer<TextLayerGL>(layer).create(2, "abc", {}, topLevelOnTopGreen);
        ui.layer<TextLayerGL>(layer).create(3, "abcdef", {}, topLevelHiddenBlue);
        ui.layer<TextLayerGL>(layer).create(0, "ab", {}, topLevelBelowRed);
        ui.layer<TextLayerGL>(layer).create(1, "abcde", {}, childAboveRed);
        ui.layer<TextLayerGL>(layer).create(3, "abcd", {}, childBelowBlue);
    }

    ui.draw();

    MAGNUM_VERIFY_NO_GL_ERROR();

    if(!(_importerManager.load("AnyImageImporter") & PluginManager::LoadState::Loaded) ||
       !(_importerManager.load("StbImageImporter") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("AnyImageImporter / StbImageImporter plugins not found.");

    #if defined(MAGNUM_TARGET_GLES) && !defined(MAGNUM_TARGET_WEBGL)
    /* Same problem is with all builtin shaders, so this doesn't seem to be a
       bug in the base layer shader code */
    if(GL::Context::current().detectedDriver() & GL::Context::DetectedDriver::SwiftShader)
        CORRADE_SKIP("UBOs with dynamically indexed arrays don't seem to work on SwiftShader, can't test.");
    #endif
    CORRADE_COMPARE_WITH(_framebuffer.read({{}, DrawSize}, {PixelFormat::RGBA8Unorm}),
        Utility::Path::join(WHEE_TEST_DIR, "TextLayerTestFiles/draw-order.png"),
        DebugTools::CompareImageToFile{_importerManager});
}

void TextLayerGLTest::drawClipping() {
    auto&& data = DrawClippingData[testCaseInstanceId()];
    setTestCaseDescription(data.name);

    /* Based on BaseLayerGLTest::drawClipping(), with additional variability
       due to each text having a different size */

    /* X is divided by 10, Y by 100 when rendering. Window size (for events)
       isn't used for anything here. */
    AbstractUserInterface ui{{640.0f, 6400.0f}, {1.0f, 1.0f}, DrawSize};

    /* A font that just produces glyph ID 0 spaced 16 units apart */
    struct: Text::AbstractFont {
        Text::FontFeatures doFeatures() const override { return {}; }
        bool doIsOpened() const override { return _opened; }
        Properties doOpenFile(Containers::StringView, Float) override {
            _opened = true;
            return {8.0f, 80.0f, -80.0f, 160.0f, 1};
        }
        void doClose() override { _opened = false; }

        void doGlyphIdsInto(const Containers::StridedArrayView1D<const char32_t>&, const Containers::StridedArrayView1D<UnsignedInt>&) override {}
        Vector2 doGlyphSize(UnsignedInt) override { return {}; }
        Vector2 doGlyphAdvance(UnsignedInt) override { return {}; }
        Containers::Pointer<Text::AbstractShaper> doCreateShaper() override {
            struct Shaper: Text::AbstractShaper {
                using Text::AbstractShaper::AbstractShaper;

                UnsignedInt doShape(Containers::StringView text, UnsignedInt, UnsignedInt, Containers::ArrayView<const Text::FeatureRange>) override {
                    return text.size();
                }
                void doGlyphIdsInto(const Containers::StridedArrayView1D<UnsignedInt>& ids) const override {
                    for(std::size_t i = 0; i != ids.size(); ++i)
                        ids[i] = 0;
                }
                void doGlyphOffsetsAdvancesInto(const Containers::StridedArrayView1D<Vector2>& offsets, const Containers::StridedArrayView1D<Vector2>& advances) const override {
                    for(std::size_t i = 0; i != offsets.size(); ++i) {
                        offsets[i] = {0.0f, -80.0f};
                        advances[i] = {8.0f, 0.0f};
                    }
                }
                void doGlyphClustersInto(const Containers::StridedArrayView1D<UnsignedInt>&) const override {
                    /** @todo implement when it actually does get called for
                        cursor / selection */
                    CORRADE_FAIL("This shouldn't be called.");
                }
            };
            return Containers::pointer<Shaper>(*this);
        }

        bool _opened = false;
    } font;
    font.openFile({}, 8.0f);

    /* A full-white glyph cache, containing just one 7x160 glyph. Default
       padding is 1, resetting to 0 to make this work. */
    Text::GlyphCache cache{{8, 160}, {}};
    for(auto row: cache.image().pixels<UnsignedByte>()[0])
        for(UnsignedByte& pixel: row)
            pixel = 255;
    cache.flushImage({{}, {8, 160}});
    cache.addGlyph(cache.addFont(1, &font), 0, {}, {{}, {7, 160}});

    TextLayerGL::Shared layerShared{3, 5};
    layerShared.setGlyphCache(cache);

    FontHandle fontHandleLarge = layerShared.addFont(font, 160.0f);
    FontHandle fontHandleSmall = layerShared.addFont(font, 80.0f);
    /* Testing the styleToUniform initializer list overload, others cases use
       implicit mapping initializer list overloads */
    layerShared.setStyle(TextLayerCommonStyleUniform{}, {
        TextLayerStyleUniform{}
            .setColor(0xff0000_rgbf),
        TextLayerStyleUniform{}
            .setColor(0x00ff00_rgbf),
        TextLayerStyleUniform{}
            .setColor(0x0000ff_rgbf)
    }, {
        0, /* 0, red large */
        0, /* 1, red small */
        1, /* 2, green small */
        2, /* 3, blue large */
        2, /* 4, blue small */
    }, {
        fontHandleLarge,
        fontHandleSmall,
        fontHandleSmall,
        fontHandleLarge,
        fontHandleSmall
    }, {});

    TextLayerGL& layer = ui.setLayerInstance(Containers::pointer<TextLayerGL>(ui.createLayer(), layerShared));

    /* Two main clip nodes, each containing subnodes which areas should touch
       each other but not overlap if clipping is enabled. They're either
       top-level nodes with possibly swapped order, in which case they're
       submitted in two separate draws, or they're sub-nodes of a single
       top-level node in which case they're drawn together with two clip rect
       ranges. */
    NodeHandle parent = NodeHandle::Null;
    if(data.singleTopLevel) {
        parent = ui.createNode({}, {});
    }

    NodeHandle leftTop = ui.createNode(parent, {100.0f, 600.0f}, {320.0f, 3200.0f});
    NodeHandle leftTop1 = ui.createNode(leftTop, {0.0f, 0.0f}, {320.0f, 2400.0f});
    NodeHandle leftTop2 = ui.createNode(leftTop, {0.0f, 2400.0f}, {320.0f, 800.0f});
    /* Child of leftTop2, but should only be clipped against leftTop, not
       leftTop2 */
    NodeHandle leftTop21 = ui.createNode(leftTop2, {60.0f, -800.0f}, {80.0f, 2400.0f});
    layer.create(0, "abc", {}, leftTop1);
    layer.create(2, "abcdef", {}, leftTop2);
    layer.create(3, "a", {}, leftTop21);

    NodeHandle rightBottom = ui.createNode(parent, {420.0f, 3600.0f}, {160.0f, 2000.0f});
    NodeHandle rightBottom1 = ui.createNode(rightBottom, {0.0f, 0.0f}, {80.0f, 2000.0f});
    /* Completely outside the rightBottom area, should get culled, i.e. not
       even passed to draw() */
    NodeHandle rightBottom11 = ui.createNode(rightBottom1, {-400.0f, 1400.0f}, {80.0f, 800.0f});
    /* Data added to the clip node should get clipped as well */
    DataHandle rightBottomData = layer.create(4, "abc", {}, rightBottom);
    layer.setPadding(rightBottomData, {20.0f, 1600.0f, 0.0f, 0.0f});
    layer.create(1, "abcd", {}, rightBottom1);
    layer.create(2, "a", {}, rightBottom11);

    if(data.flipOrder) {
        CORRADE_COMPARE(ui.nodeOrderNext(rightBottom), NodeHandle::Null);
        ui.setNodeOrder(rightBottom, leftTop);
        CORRADE_COMPARE(ui.nodeOrderNext(rightBottom), leftTop);
    }

    if(data.clip) {
        ui.addNodeFlags(leftTop, NodeFlag::Clip);
        ui.addNodeFlags(rightBottom, NodeFlag::Clip);
    }

    ui.draw();

    MAGNUM_VERIFY_NO_GL_ERROR();

    if(!(_importerManager.load("AnyImageImporter") & PluginManager::LoadState::Loaded) ||
       !(_importerManager.load("StbImageImporter") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("AnyImageImporter / StbImageImporter plugins not found.");

    #if defined(MAGNUM_TARGET_GLES) && !defined(MAGNUM_TARGET_WEBGL)
    /* Same problem is with all builtin shaders, so this doesn't seem to be a
       bug in the base layer shader code */
    if(GL::Context::current().detectedDriver() & GL::Context::DetectedDriver::SwiftShader)
        CORRADE_SKIP("UBOs with dynamically indexed arrays don't seem to work on SwiftShader, can't test.");
    #endif
    CORRADE_COMPARE_WITH(_framebuffer.read({{}, DrawSize}, {PixelFormat::RGBA8Unorm}),
        Utility::Path::join({WHEE_TEST_DIR, "TextLayerTestFiles", data.filename}),
        DebugTools::CompareImageToFile{_importerManager});
}

void TextLayerGLTest::eventStyleTransition() {
    /* Switches between the "default" and "colored" cases from render() after a
       press event. Everything else is tested in AbstractVisualLayerTest
       already. */

    if(!(_fontManager.load("StbTrueTypeFont") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("StbTrueTypeFont plugin not found.");

    AbstractUserInterface ui{RenderSize};

    /* Opened in the constructor together with cache filling to circumvent
       stb_truetype's extreme rasterization slowness */
    CORRADE_VERIFY(_font && _font->isOpened());

    TextLayerGL::Shared layerShared{2};
    layerShared.setGlyphCache(_fontGlyphCache);

    FontHandle fontHandle = layerShared.addFont(*_font, 32.0f);
    layerShared
        .setStyle(TextLayerCommonStyleUniform{}, {
            TextLayerStyleUniform{},        /* default */
            TextLayerStyleUniform{}         /* colored */
                .setColor(0x3bd267_rgbf)
        }, {fontHandle, fontHandle}, {})
        .setStyleTransition(
            [](UnsignedInt style) -> UnsignedInt {
                if(style == 0) return 1;
                CORRADE_INTERNAL_ASSERT_UNREACHABLE();
            },
            [](UnsignedInt) -> UnsignedInt {
                CORRADE_INTERNAL_ASSERT_UNREACHABLE();
            });

    LayerHandle layer = ui.createLayer();
    ui.setLayerInstance(Containers::pointer<TextLayerGL>(layer, layerShared));

    NodeHandle node = ui.createNode({8.0f, 8.0f}, {112.0f, 48.0f});
    /* Using a text that has glyphs both above and below line and doesn't need
       too many glyphs */
    ui.layer<TextLayerGL>(layer).create(0, "Maggi", {}, node);

    ui.draw();

    MAGNUM_VERIFY_NO_GL_ERROR();
    Image2D before = _framebuffer.read({{}, RenderSize}, {PixelFormat::RGBA8Unorm});

    PointerEvent event{Pointer::MouseLeft};
    CORRADE_VERIFY(ui.pointerPressEvent({64.0f, 24.0f}, event));
    CORRADE_COMPARE(ui.state(), UserInterfaceState::NeedsDataUpdate);

    /* We have blending enabled, which means a subsequent draw would try to
       blend with the previous, causing unwanted difference */
    _framebuffer.clear(GL::FramebufferClear::Color);

    ui.draw();

    MAGNUM_VERIFY_NO_GL_ERROR();
    Image2D after = _framebuffer.read({{}, RenderSize}, {PixelFormat::RGBA8Unorm});

    if(!(_importerManager.load("AnyImageImporter") & PluginManager::LoadState::Loaded) ||
       !(_importerManager.load("StbImageImporter") & PluginManager::LoadState::Loaded))
        CORRADE_SKIP("AnyImageImporter / StbImageImporter plugins not found.");

    #if defined(MAGNUM_TARGET_GLES) && !defined(MAGNUM_TARGET_WEBGL)
    /* Same problem is with all builtin shaders, so this doesn't seem to be a
       bug in the base layer shader code */
    if(GL::Context::current().detectedDriver() & GL::Context::DetectedDriver::SwiftShader)
        CORRADE_SKIP("UBOs with dynamically indexed arrays don't seem to work on SwiftShader, can't test.");
    #endif
    CORRADE_COMPARE_WITH(before,
        Utility::Path::join(WHEE_TEST_DIR, "TextLayerTestFiles/default.png"),
        DebugTools::CompareImageToFile{_importerManager});
    CORRADE_COMPARE_WITH(after,
        Utility::Path::join(WHEE_TEST_DIR, "TextLayerTestFiles/colored.png"),
        DebugTools::CompareImageToFile{_importerManager});
}

}}}}

CORRADE_TEST_MAIN(Magnum::Whee::Test::TextLayerGLTest)