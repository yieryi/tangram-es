#pragma once

#include "style.h"
#include "labels/label.h"
#include "style/labelProperty.h"
#include "util/hash.h"

#include <memory>
#include <vector>
#include <string>

namespace alfons { class Font; }

namespace Tangram {

class FontContext;
class LabelContainer;
struct GlyphQuad;
struct TextMesh;
struct Properties;

class TextStyle : public Style {

public:

    struct Parameters {
        std::shared_ptr<alfons::Font> font;
        std::string text = "";
        bool interactive = false;
        uint32_t fill = 0xff000000;
        uint32_t strokeColor = 0xffffffff;
        float strokeWidth = 0.0f;
        float fontSize = 16.0f;
        float blurSpread = 0.0f;
        Label::Options labelOptions;
        bool wordWrap = true;
        uint32_t maxLineWidth = 15;

        TextLabelProperty::Transform transform = TextLabelProperty::Transform::none;
        TextLabelProperty::Align align = TextLabelProperty::Align::center;
        LabelProperty::Anchor anchor = LabelProperty::Anchor::center;
    };

    auto& context() const { return m_context; }

    void onBeginFrame() override;

protected:

    void constructVertexLayout() override;
    void constructShaderProgram() override;

    std::unique_ptr<StyleBuilder> createBuilder() const override;

    bool m_sdf;

    std::shared_ptr<FontContext> m_context;

    UniformLocation m_uTexScaleFactor{"u_uv_scale_factor"};
    UniformLocation m_uTex{"u_tex"};
    UniformLocation m_uOrtho{"u_ortho"};
    UniformLocation m_uPass{"u_pass"};
    UniformLocation m_uMaxStrokeWidth{"u_max_stroke_width"};

    mutable std::vector<std::unique_ptr<LabelMesh>> m_meshes;

public:

    TextStyle(std::string _name, bool _sdf = false,
              Blending _blendMode = Blending::overlay,
              GLenum _drawMode = GL_TRIANGLES);

    /* Upload the buffers of the text batches
     * Upload the textur atlases
     */
    void onBeginDrawFrame(const View& _view, Scene& _scene) override;

    /* Performs the actual drawing of the meshes in two passes
     * - First pass if signed distance field is on, draw outlines
     * - Second pass, draw the inner glyph pixels
     */
    void onEndDrawFrame() override;

    /* Dequeue the meshes (glyph batches) and create the associated gl mesh
     * No GL involved, called from Tangram::update()
     */
    void onBeginUpdate() override;

    LabelMesh& mesh(size_t id) const { return *m_meshes[id]; }

    virtual ~TextStyle() override;

private:

    const std::string& applyTextSource(const Parameters& _parameters, const Properties& _props) const;
};

}

namespace std {
    template <>
    struct hash<Tangram::TextStyle::Parameters> {
        size_t operator() (const Tangram::TextStyle::Parameters& p) const {
            std::hash<Tangram::Label::Options> optionsHash;
            std::size_t seed = 0;
            // TODO
            //hash_combine(seed, p.fontId);
            hash_combine(seed, p.text);
            hash_combine(seed, p.interactive);
            hash_combine(seed, p.fill);
            hash_combine(seed, p.strokeColor);
            hash_combine(seed, p.strokeWidth);
            hash_combine(seed, p.fontSize);
            hash_combine(seed, p.wordWrap);
            hash_combine(seed, p.maxLineWidth);
            hash_combine(seed, int(p.transform));
            hash_combine(seed, int(p.align));
            hash_combine(seed, int(p.anchor));
            hash_combine(seed, optionsHash(p.labelOptions));
            return seed;
        }
    };
}
