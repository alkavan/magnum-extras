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

#include "BaseLayer.h"

#include <Corrade/Containers/Array.h>
#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/Math/Functions.h>
#include <Magnum/Math/Matrix3.h>

#include "Magnum/Whee/Event.h"
#include "Magnum/Whee/Handle.h"
#include "Magnum/Whee/Implementation/baseLayerState.h"

namespace Magnum { namespace Whee {

BaseLayer::Shared::Shared(Containers::Pointer<State>&& state): _state{Utility::move(state)} {}

BaseLayer::Shared::Shared(const UnsignedInt styleCount): Shared{Containers::pointer<State>(styleCount)} {}

BaseLayer::Shared::Shared(NoCreateT) noexcept {}

BaseLayer::Shared::Shared(Shared&&) noexcept = default;

BaseLayer::Shared::~Shared() = default;

BaseLayer::Shared& BaseLayer::Shared::operator=(Shared&&) noexcept = default;

UnsignedInt BaseLayer::Shared::styleCount() const {
    return _state->styleCount;
}

BaseLayer::Shared& BaseLayer::Shared::setStyleTransition(UnsignedInt(*const toPressedBlur)(UnsignedInt), UnsignedInt(*const toPressedHover)(UnsignedInt), UnsignedInt(*const toInactiveBlur)(UnsignedInt), UnsignedInt(*const toInactiveHover)(UnsignedInt)) {
    _state->styleTransitionToPressedBlur = toPressedBlur ? toPressedBlur :
        Implementation::styleTransitionPassthrough;
    _state->styleTransitionToPressedHover = toPressedHover ? toPressedHover :
        Implementation::styleTransitionPassthrough;
    _state->styleTransitionToInactiveBlur = toInactiveBlur ? toInactiveBlur :
        Implementation::styleTransitionPassthrough;
    _state->styleTransitionToInactiveHover = toInactiveHover ? toInactiveHover :
        Implementation::styleTransitionPassthrough;
    return *this;
}

BaseLayer::BaseLayer(const LayerHandle handle, Containers::Pointer<State>&& state): AbstractLayer{handle}, _state{Utility::move(state)} {}

BaseLayer::BaseLayer(const LayerHandle handle, Shared& shared): BaseLayer{handle, Containers::pointer<State>(*shared._state)} {}

BaseLayer::BaseLayer(BaseLayer&&) noexcept = default;

BaseLayer::~BaseLayer() = default;

BaseLayer& BaseLayer::operator=(BaseLayer&&) noexcept = default;

DataHandle BaseLayer::create(const UnsignedInt style, const Color3& color, const Vector4& outlineWidth, const NodeHandle node) {
    CORRADE_ASSERT(style < _state->shared.styleCount,
        "Whee::BaseLayer::create(): style" << style << "out of range for" << _state->shared.styleCount << "styles", {});

    const DataHandle handle = AbstractLayer::create(node);
    const UnsignedInt id = dataHandleId(handle);
    if(id >= _state->data.size())
        arrayAppend(_state->data, NoInit, id - _state->data.size() + 1);

    Implementation::BaseLayerData& data = _state->data[id];
    data.outlineWidth = outlineWidth;
    data.color = color;
    data.style = style;
    return handle;
}

UnsignedInt BaseLayer::style(const DataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::style(): invalid handle" << handle, {});
    return _state->data[dataHandleId(handle)].style;
}

UnsignedInt BaseLayer::style(const LayerDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::style(): invalid handle" << handle, {});
    return _state->data[layerDataHandleId(handle)].style;
}

void BaseLayer::setStyle(const DataHandle handle, const UnsignedInt style) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::setStyle(): invalid handle" << handle, );
    CORRADE_ASSERT(style < _state->shared.styleCount,
        "Whee::BaseLayer::setStyle(): style" << style << "out of range for" << _state->shared.styleCount << "styles", );
    setStyleInternal(dataHandleId(handle), style);
}

void BaseLayer::setStyle(const LayerDataHandle handle, const UnsignedInt style) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::setStyle(): invalid handle" << handle, );
    CORRADE_ASSERT(style < _state->shared.styleCount,
        "Whee::BaseLayer::setStyle(): style" << style << "out of range for" << _state->shared.styleCount << "styles", );
    setStyleInternal(layerDataHandleId(handle), style);
}

void BaseLayer::setStyleInternal(const UnsignedInt id, const UnsignedInt style) {
    _state->data[id].style = style;
    setNeedsUpdate();
}

Color3 BaseLayer::color(const DataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::color(): invalid handle" << handle, {});
    return _state->data[dataHandleId(handle)].color;
}

Color3 BaseLayer::color(const LayerDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::color(): invalid handle" << handle, {});
    return _state->data[layerDataHandleId(handle)].color;
}

void BaseLayer::setColor(const DataHandle handle, const Color3& color) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::setColor(): invalid handle" << handle, );
    setColorInternal(dataHandleId(handle), color);
}

void BaseLayer::setColor(const LayerDataHandle handle, const Color3& color) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::setColor(): invalid handle" << handle, );
    setColorInternal(layerDataHandleId(handle), color);
}

void BaseLayer::setColorInternal(const UnsignedInt id, const Color3& color) {
    _state->data[id].color = color;
    setNeedsUpdate();
}

void BaseLayer::setOutlineWidth(const DataHandle handle, const Vector4& width) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::setOutlineWidth(): invalid handle" << handle, );
    setOutlineWidthInternal(dataHandleId(handle), width);
}

Vector4 BaseLayer::outlineWidth(const DataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::outlineWidth(): invalid handle" << handle, {});
    return _state->data[dataHandleId(handle)].outlineWidth;
}

Vector4 BaseLayer::outlineWidth(const LayerDataHandle handle) const {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::outlineWidth(): invalid handle" << handle, {});
    return _state->data[layerDataHandleId(handle)].outlineWidth;
}

void BaseLayer::setOutlineWidth(const LayerDataHandle handle, const Vector4& width) {
    CORRADE_ASSERT(isHandleValid(handle),
        "Whee::BaseLayer::setOutlineWidth(): invalid handle" << handle, );
    setOutlineWidthInternal(layerDataHandleId(handle), width);
}

void BaseLayer::setOutlineWidthInternal(const UnsignedInt id, const Vector4& width) {
    _state->data[id].outlineWidth = width;
    setNeedsUpdate();
}

LayerFeatures BaseLayer::doFeatures() const {
    return LayerFeature::Event|LayerFeature::Draw;
}

void BaseLayer::doUpdate(const Containers::StridedArrayView1D<const UnsignedInt>& dataIds, const Containers::StridedArrayView1D<const UnsignedInt>&, const Containers::StridedArrayView1D<const UnsignedInt>&, const Containers::StridedArrayView1D<const Vector2>& nodeOffsets, const Containers::StridedArrayView1D<const Vector2>& nodeSizes, const Containers::StridedArrayView1D<const Vector2>&, const Containers::StridedArrayView1D<const Vector2>&) {
    State& state = *_state;

    /* Fill in indices in desired order */
    arrayResize(state.indices, NoInit, dataIds.size()*6);
    for(UnsignedInt i = 0; i != dataIds.size(); ++i) {
        const UnsignedInt vertexOffset = dataIds[i]*4;
        UnsignedInt indexOffset = i*6;

        /* 0---1 0---2 5
           |   | |  / /|
           |   | | / / |
           |   | |/ /  |
           2---3 1 3---4 */
        state.indices[indexOffset++] = vertexOffset + 0;
        state.indices[indexOffset++] = vertexOffset + 2;
        state.indices[indexOffset++] = vertexOffset + 1;
        state.indices[indexOffset++] = vertexOffset + 2;
        state.indices[indexOffset++] = vertexOffset + 3;
        state.indices[indexOffset++] = vertexOffset + 1;
    }

    const Containers::StridedArrayView1D<const NodeHandle> nodes = this->nodes();

    /* Fill in quad corner positions and colors */
    arrayResize(state.vertices, NoInit, capacity()*4);
    for(const UnsignedInt dataId: dataIds) {
        const UnsignedInt nodeId = nodeHandleId(nodes[dataId]);
        const Implementation::BaseLayerData& data = state.data[dataId];

        /* 0---1
           |   |
           |   |
           |   |
           2---3 */
        const Vector2 size = nodeSizes[nodeId];
        const Vector2 min = nodeOffsets[nodeId];
        const Vector2 max = min + size;
        const Vector2 sizeHalf = size*0.5f;
        const Vector2 sizeHalfNegative = -sizeHalf;
        for(UnsignedByte i = 0; i != 4; ++i) {
            Implementation::BaseLayerVertex& vertex = state.vertices[dataId*4 + i];

            /* ✨ */
            vertex.position = Math::lerp(min, max, BitVector2{i});
            vertex.centerDistance = Math::lerp(sizeHalfNegative, sizeHalf, BitVector2{i});
            vertex.outlineWidth = data.outlineWidth;
            vertex.color = data.color;
            vertex.style = data.style;
        }
    }
}

void BaseLayer::doPointerPressEvent(const UnsignedInt dataId, PointerEvent& event) {
    /* Only reacting to pointer types typically used to click/tap on things */
    if(event.type() != Pointer::MouseLeft &&
       event.type() != Pointer::Finger &&
       event.type() != Pointer::Pen)
        return;

    /* A press can be not hovering if it happened without a preceding move
       event (such as for pointer types that don't support hover like touches,
       or if move events aren't propagated from the application) */
    State& state = *_state;
    const Shared::State& sharedState = state.shared;
    UnsignedInt(*const transition)(UnsignedInt) = event.isHovering() ?
        sharedState.styleTransitionToPressedHover :
        sharedState.styleTransitionToPressedBlur;

    Implementation::BaseLayerData& data = state.data[dataId];
    const UnsignedInt nextStyle = transition(data.style);
    CORRADE_ASSERT(nextStyle < sharedState.styleCount,
        "Whee::BaseLayer::pointerPressEvent(): style transition from" << data.style << "to" << nextStyle << "out of range for" << sharedState.styleCount << "styles", );
    if(nextStyle != data.style) {
        data.style = nextStyle;
        setNeedsUpdate();
    }
    event.setAccepted();
}

void BaseLayer::doPointerReleaseEvent(const UnsignedInt dataId, PointerEvent& event) {
    /* Only reacting to pointer types typically used to click/tap on things */
    if(event.type() != Pointer::MouseLeft &&
       event.type() != Pointer::Finger &&
       event.type() != Pointer::Pen)
        return;

    /* A release can be not hovering if it happened without a preceding move
       event (such as for pointer types that don't support hover like touches,
       or if move events aren't propagated from the application) */
    State& state = *_state;
    const Shared::State& sharedState = state.shared;
    UnsignedInt(*const transition)(UnsignedInt) = event.isHovering() ?
        sharedState.styleTransitionToInactiveHover :
        sharedState.styleTransitionToInactiveBlur;

    Implementation::BaseLayerData& data = state.data[dataId];
    const UnsignedInt nextStyle = transition(data.style);
    CORRADE_ASSERT(nextStyle < sharedState.styleCount,
        "Whee::BaseLayer::pointerReleaseEvent(): style transition from" << data.style << "to" << nextStyle << "out of range for" << sharedState.styleCount << "styles", );
    if(nextStyle != data.style) {
        data.style = nextStyle;
        setNeedsUpdate();
    }
    event.setAccepted();
}

void BaseLayer::doPointerMoveEvent(UnsignedInt, PointerMoveEvent& event) {
    /* In order to have Enter/Leave emitted as well */
    event.setAccepted();
}

void BaseLayer::doPointerEnterEvent(const UnsignedInt dataId, PointerMoveEvent& event) {
    State& state = *_state;
    const Shared::State& sharedState = state.shared;
    UnsignedInt(*const transition)(UnsignedInt) = event.isCaptured() ?
        sharedState.styleTransitionToPressedHover :
        sharedState.styleTransitionToInactiveHover;

    Implementation::BaseLayerData& data = state.data[dataId];
    const UnsignedInt nextStyle = transition(data.style);
    CORRADE_ASSERT(nextStyle < sharedState.styleCount,
        "Whee::BaseLayer::pointerEnterEvent(): style transition from" << data.style << "to" << nextStyle << "out of range for" << sharedState.styleCount << "styles", );
    if(nextStyle != data.style) {
        data.style = nextStyle;
        setNeedsUpdate();
    }
}

void BaseLayer::doPointerLeaveEvent(const UnsignedInt dataId, PointerMoveEvent& event) {
    State& state = *_state;
    const Shared::State& sharedState = state.shared;
    UnsignedInt(*const transition)(UnsignedInt) = event.isCaptured() ?
        sharedState.styleTransitionToPressedBlur :
        sharedState.styleTransitionToInactiveBlur;

    Implementation::BaseLayerData& data = state.data[dataId];
    const UnsignedInt nextStyle = transition(data.style);
    CORRADE_ASSERT(nextStyle < sharedState.styleCount,
        "Whee::BaseLayer::pointerLeaveEvent(): style transition from" << data.style << "to" << nextStyle << "out of range for" << sharedState.styleCount << "styles", );
    if(nextStyle != data.style) {
        data.style = nextStyle;
        setNeedsUpdate();
    }
}

}}
