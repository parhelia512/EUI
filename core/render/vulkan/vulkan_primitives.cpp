#include "core/render/primitive.h"

#include <algorithm>
#include <utility>

namespace core {

struct RoundedRectPrimitive::Impl {
    explicit Impl(float x = 0.0f, float y = 0.0f, float width = 0.0f, float height = 0.0f)
        : bounds{x, y, width, height} {}

    Rect bounds{};
    Color color{};
    Gradient gradient{};
    Border border{};
    Shadow shadow{};
    Transform transform{};
    TransformMatrix transformMatrix{};
    float blur = 0.0f;
    float cornerRadius = 0.0f;
    float opacity = 1.0f;
    bool hasTransformMatrix = false;
};

RoundedRectPrimitive::RoundedRectPrimitive()
    : impl_(std::make_unique<Impl>()) {}

RoundedRectPrimitive::RoundedRectPrimitive(float x, float y, float width, float height)
    : impl_(std::make_unique<Impl>(x, y, width, height)) {}

RoundedRectPrimitive::~RoundedRectPrimitive() = default;
RoundedRectPrimitive::RoundedRectPrimitive(RoundedRectPrimitive&&) noexcept = default;
RoundedRectPrimitive& RoundedRectPrimitive::operator=(RoundedRectPrimitive&&) noexcept = default;

bool RoundedRectPrimitive::initialize() { return true; }
void RoundedRectPrimitive::destroy() {}
void RoundedRectPrimitive::setBounds(float x, float y, float width, float height) { impl_->bounds = {x, y, width, height}; }
void RoundedRectPrimitive::setColor(const Color& color) { impl_->color = color; }
void RoundedRectPrimitive::setGradient(const Gradient& gradient) { impl_->gradient = gradient; }
void RoundedRectPrimitive::setCornerRadius(float radius) { impl_->cornerRadius = radius; }
void RoundedRectPrimitive::setOpacity(float opacity) { impl_->opacity = std::clamp(opacity, 0.0f, 1.0f); }
void RoundedRectPrimitive::setBorder(const Border& border) { impl_->border = border; }
void RoundedRectPrimitive::setShadow(const Shadow& shadow) { impl_->shadow = shadow; }
void RoundedRectPrimitive::setBlur(float blur) { impl_->blur = std::max(0.0f, blur); }
void RoundedRectPrimitive::setTranslate(float x, float y) {
    impl_->transform.translate = {x, y};
    impl_->hasTransformMatrix = false;
}
void RoundedRectPrimitive::setScale(float x, float y) {
    impl_->transform.scale = {x, y};
    impl_->hasTransformMatrix = false;
}
void RoundedRectPrimitive::setRotate(float radians) {
    impl_->transform.rotate = radians;
    impl_->hasTransformMatrix = false;
}
void RoundedRectPrimitive::setTransformOrigin(float x, float y) {
    impl_->transform.origin = {x, y};
    impl_->hasTransformMatrix = false;
}
void RoundedRectPrimitive::setTransform(const Transform& transform) {
    impl_->transform = transform;
    impl_->hasTransformMatrix = false;
}
void RoundedRectPrimitive::setTransformMatrix(const TransformMatrix& matrix) {
    impl_->transformMatrix = matrix;
    impl_->hasTransformMatrix = true;
}
const Rect& RoundedRectPrimitive::bounds() const { return impl_->bounds; }
const Color& RoundedRectPrimitive::color() const { return impl_->color; }
const Gradient& RoundedRectPrimitive::gradient() const { return impl_->gradient; }
const Border& RoundedRectPrimitive::border() const { return impl_->border; }
const Shadow& RoundedRectPrimitive::shadow() const { return impl_->shadow; }
float RoundedRectPrimitive::blur() const { return impl_->blur; }
const Transform& RoundedRectPrimitive::transform() const { return impl_->transform; }
float RoundedRectPrimitive::cornerRadius() const { return impl_->cornerRadius; }
float RoundedRectPrimitive::opacity() const { return impl_->opacity; }
void RoundedRectPrimitive::render(int, int) const {}

struct PolygonPrimitive::Impl {
    Rect bounds{};
    std::vector<Vec2> points;
    Color color{};
    Transform transform{};
    TransformMatrix transformMatrix{};
    float opacity = 1.0f;
    bool hasTransformMatrix = false;
};

PolygonPrimitive::PolygonPrimitive()
    : impl_(std::make_unique<Impl>()) {}

PolygonPrimitive::~PolygonPrimitive() = default;
PolygonPrimitive::PolygonPrimitive(PolygonPrimitive&&) noexcept = default;
PolygonPrimitive& PolygonPrimitive::operator=(PolygonPrimitive&&) noexcept = default;

bool PolygonPrimitive::initialize() { return true; }
void PolygonPrimitive::destroy() {}
void PolygonPrimitive::setBounds(float x, float y, float width, float height) { impl_->bounds = {x, y, width, height}; }
void PolygonPrimitive::setPoints(const std::vector<Vec2>& points) { impl_->points = points; }
void PolygonPrimitive::setColor(const Color& color) { impl_->color = color; }
void PolygonPrimitive::setOpacity(float opacity) { impl_->opacity = std::clamp(opacity, 0.0f, 1.0f); }
void PolygonPrimitive::setTransform(const Transform& transform) {
    impl_->transform = transform;
    impl_->hasTransformMatrix = false;
}
void PolygonPrimitive::setTransformMatrix(const TransformMatrix& matrix) {
    impl_->transformMatrix = matrix;
    impl_->hasTransformMatrix = true;
}
void PolygonPrimitive::render(int, int) const {}

} // namespace core
