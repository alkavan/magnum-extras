#ifndef Magnum_Ui_TextLayerAnimator_h
#define Magnum_Ui_TextLayerAnimator_h
/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019,
                2020, 2021, 2022, 2023, 2024
              Vladimír Vondruš <mosra@centrum.cz>

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

/** @file
 * @brief Class @ref Magnum::Ui::TextLayerStyleAnimator, enum @ref Magnum::Ui::TextLayerStyleAnimation, enum set @ref Magnum::Ui::TextLayerStyleAnimations
 * @m_since_latest
 */

#include <Magnum/Math/Time.h>

#include "Magnum/Ui/AbstractVisualLayerAnimator.h"

namespace Magnum { namespace Ui {

/**
@brief Text layer style properties that are being animated
@m_since_latest

Depending on which of these are returned from
@ref TextLayerStyleAnimator::advance(), causes various @ref LayerState flags
and other internal @ref AbstractLayer state to be set after an
@ref AbstractUserInterface::advanceAnimations() (and transitively
@ref AbstractLayer::advanceAnimations(Nanoseconds, Containers::MutableBitArrayView, const Containers::StridedArrayView1D<Float>&, Containers::MutableBitArrayView, const Containers::Iterable<AbstractStyleAnimator>&))
call.
@see @ref TextLayerStyleAnimations
*/
enum class TextLayerStyleAnimation: UnsignedByte {
    /**
     * Style uniform data. Equivalently to calling
     * @ref TextLayer::setDynamicStyle(), causes
     * @ref LayerState::NeedsCommonDataUpdate to be set.
     */
    Uniform = 1 << 0,

    /**
     * Style padding. Equivalently to calling @ref TextLayer::setDynamicStyle()
     * with a different padding value, causes @ref LayerState::NeedsDataUpdate
     * to be set.
     */
    Padding = 1 << 1,

    /**
     * Editing style uniform data. Equivalently to calling
     * @ref TextLayer::setDynamicStyleWithCursorSelection(),
     * @relativeref{TextLayer,setDynamicStyleWithCursor()} or
     * @relativeref{TextLayer,setDynamicStyleWithSelection()}, causes
     * @ref LayerState::NeedsCommonDataUpdate to be set.
     */
    EditingUniform = 1 << 2,

    /**
     * Editing style padding. Equivalently to calling
     * @ref TextLayer::setDynamicStyleWithCursorSelection(),
     * @relativeref{TextLayer,setDynamicStyleWithCursor()} or
     * @relativeref{TextLayer,setDynamicStyleWithSelection()} with a different
     * editing padding value, causes @ref LayerState::NeedsDataUpdate to be
     * set.
     */
    EditingPadding = 1 << 3,

    /**
     * Style assignment. Equivalently to calling @ref TextLayer::setStyle(),
     * causes @ref LayerState::NeedsDataUpdate to be set.
     */
    Style = 1 << 4
};

/**
@debugoperatorenum{TextLayerStyleAnimation}
@m_since_latest
*/
MAGNUM_UI_EXPORT Debug& operator<<(Debug& debug, TextLayerStyleAnimation value);

/**
@brief Set of text layer style properties that are being animated
@m_since_latest

@see @ref TextLayerStyleAnimator::advance()
*/
typedef Containers::EnumSet<TextLayerStyleAnimation> TextLayerStyleAnimations;

/**
@debugoperatorenum{TextLayerStyleAnimations}
@m_since_latest
*/
MAGNUM_UI_EXPORT Debug& operator<<(Debug& debug, TextLayerStyleAnimations value);

CORRADE_ENUMSET_OPERATORS(TextLayerStyleAnimations)

/**
@brief Text layer style animator
@m_since_latest

Each animation is a transition between two @ref TextLayer styles, with
individual properties interpolated with an easing function.
@ref BaseLayerStyleAnimator is a matching animator for the @ref BaseLayer.

@section Ui-TextLayerStyleAnimator-setup Setting up an animator instance

The animator doesn't have any shared state or configuration, so it's just about
constructing it from a fresh @ref AbstractUserInterface::createAnimator()
handle and passing it to @relativeref{AbstractUserInterface,setStyleAnimatorInstance()}.

@snippet Ui.cpp TextLayerStyleAnimator-setup1

After that, the animator has to be registered with a concrete layer instance.
The animations make use of dynamic styles, so the text layer is expected to
have at least one dynamic style enabled with
@ref TextLayer::Shared::Configuration::setDynamicStyleCount(). The more dynamic
styles are enabled, the more style animations can be running for given layer at
the same time, but also more data need to get uploaded to the GPU every frame.
Finally, call @ref TextLayer::assignAnimator(TextLayerStyleAnimator&) to assign
the animator to the layer instance. Then, assuming
@ref AbstractUserInterface::advanceAnimations() is called in an appropriate
place, the animator is ready to use.

@snippet Ui-gl.cpp TextLayerStyleAnimator-setup2

Unlike builtin layers or layouters, the default @ref UserInterface
implementation doesn't implicitly provide a @ref TextLayerStyleAnimator
instance.

@todoc setDefaultStyleAnimator(), once it's actually used by styles; then also
    update the sentence about UserInterface not having any animator instance

@section Ui-TextLayerStyleAnimator-create Creating animations

Assuming an enum is used to index the styles defined in @ref TextLayer::Shared
of the associated layer instance, an animation is created by calling
@ref create() with the source and target style indices, an easing function from
@ref Animation::BasicEasing "Animation::Easing" or a custom one, time at which
it's meant to be played, its duration, and a @ref DataHandle which the style
animation should affect. Here, for example, to fade out a button hover style
over half a second:

@snippet Ui.cpp TextLayerStyleAnimator-create

Internally, once the animation starts playing, the animator allocates a new
dynamic style index using @ref TextLayer::allocateDynamicStyle() and switches
the style index of given @ref DataHandle to the allocated dynamic style with
@ref BaseLayer::setStyle(). During the animation the style data are updated to
corresponding interpolation between the source and target styles, equivalent to
calling @ref TextLayer::setDynamicStyle(). When the animation stops, the data
style index is switched to the target ID specified in @ref create() and the
dynamic style index is recycled with @ref TextLayer::recycleDynamicStyle() is
called for the dynamic style.

If the animator runs out of dynamic styles, newly played animations are left at
the source style index until another dynamic style is recycled. If no dynamic
style gets recycled until the animation ends, the data gets switched directly
to the target style without animating.

The animation interpolates all properties of @ref TextLayerStyleUniform as well
as the style padding value. The font, alignment or text feature style
properties cannot be animated and thus are set to properties of source style at
the beginning. If the styles reference a cursor or editing style, the
corresponding @ref TextLayerEditingStyleUniform including its padding value,
and the selection @ref TextLayerStyleUniform override is animated as well.

@m_class{m-note m-warning}

@par
    If either of the styles references a cursor or a selection style, the other
    is expected to reference a cursor or a selection as well. The reason is
    that there's several possible ways how to perform a fade-in / fade-out of
    the cursor or selection, be it the quad changing size, or transparency, or
    any other way, and the animator just picking something would be too
    arbitrary.

At the moment, only animation between predefined styles is possible.

@section Ui-TextLayerStyleAnimator-lifetime Animation lifetime and data attachment

As with all other animations, they're implicitly removed once they're played.
Pass @ref AnimationFlag::KeepOncePlayed to @ref create() or @ref addFlags() to
disable this behavior.

Style animations are associated with data they animate, and thus as soon as the
data or node the data is attached to is removed, the animation gets removed as
well. If you want to preserve the animation when the data is removed, call
@ref attach(AnimationHandle, DataHandle) with @ref DataHandle::Null to detach
it from the data before removing.

If you call @ref create() with @ref DataHandle::Null, the animation will still
allocate and interpolate a dynamic style, but the style won't be used anywhere.
You can then retrieve the dynamic style index with @ref dynamicStyle() and use
it for example to make the same style animation on multiple different data.
Note that in this case you're also responsible also for switching to the target
style once the animation finishes --- once the dynamic style is recycled, the
same index may get used for arbitrary other style either by this animator or
any other code, causing visual bugs.

@todoc update this section once there's robustness against switching styles
*/
class MAGNUM_UI_EXPORT TextLayerStyleAnimator: public AbstractVisualLayerStyleAnimator {
    public:
        /**
         * @brief Constructor
         * @param handle    Handle returned by
         *      @ref AbstractUserInterface::createAnimator()
         */
        explicit TextLayerStyleAnimator(AnimatorHandle handle);

        /** @brief Copying is not allowed */
        TextLayerStyleAnimator(const AbstractStyleAnimator&) = delete;

        /** @copydoc AbstractAnimator::AbstractAnimator(AbstractAnimator&&) */
        TextLayerStyleAnimator(TextLayerStyleAnimator&&) noexcept;

        ~TextLayerStyleAnimator();

        /** @brief Copying is not allowed */
        TextLayerStyleAnimator& operator=(const TextLayerStyleAnimator&) = delete;

        /** @brief Move assignment */
        TextLayerStyleAnimator& operator=(TextLayerStyleAnimator&&) noexcept;

        /**
         * @brief Create an animation
         * @param sourceStyle   Style index to animate from
         * @param targetStyle   Style index to animate to
         * @param easing        Easing function between @cpp 0.0f @ce and
         *      @cpp 1.0f @ce, used for all style uniform values as well as the
         *      padding. Pick one from @ref Animation::BasicEasing "Animation::Easing"
         *      or supply a custom one.
         * @param played        Time at which the animation is played. Use
         *      @ref Nanoseconds::max() for creating a stopped animation.
         * @param duration      Duration of a single play of the animation
         * @param data          Data the animation is attached to. Use
         *      @ref DataHandle::Null to create an animation that isn't
         *      attached to any data.
         * @param repeatCount   Repeat count. Use @cpp 0 @ce for an
         *      indefinitely repeating animation.
         * @param flags         Flags
         *
         * Expects that @ref TextLayer::assignAnimator(TextLayerStyleAnimator&)
         * has been already called for this animator, that both @p sourceStyle
         * and @p targetStyle are less than @ref TextLayer::Shared::styleCount()
         * (not @ref TextLayer::Shared::totalStyleCount() --- the style
         * animation is not allowed to use the dynamic style indices) and that
         * @p easing is not @cpp nullptr @ce.
         *
         * @m_class{m-note m-warning}
         *
         * @par
         *      If either of the styles references a cursor or a selection
         *      style, the other is expected to reference a cursor or a
         *      selection as well. The reason is that there's several possible
         *      ways how to perform a fade-in / fade-out of the cursor or
         *      selection, be it the quad changing size, or transparency, or
         *      any other way, and the animator just picking something would be
         *      too arbitrary.
         *
         * Delegates to @ref AbstractAnimator::create(Nanoseconds, Nanoseconds, DataHandle, UnsignedInt, AnimationFlags),
         * see its documentation for more information.
         *
         * The animation affects the @ref TextLayerStyleUniform and the padding
         * value, if it differs between the styles. The animated dynamic style
         * is initialized from font, alignment and features from @p styleSrc,
         * if @p styleDst has them different, they don't affect the animation
         * in any way. If given style references a cursor or editing style, it
         * affects also the corresponding @ref TextLayerEditingStyleUniform,
         * the editing padding value, if it differs between the editing styles,
         * and the @ref TextLayerStyleUniform override for selected portions of
         * the text, if referenced.
         */
        AnimationHandle create(UnsignedInt sourceStyle, UnsignedInt targetStyle, Float(*easing)(Float), Nanoseconds played, Nanoseconds duration, DataHandle data, UnsignedInt repeatCount = 1, AnimationFlags flags = {});

        /**
         * @brief Create an animation with a style index in a concrete enum type
         *
         * Casts @p sourceStyle and @p targetStyle to
         * @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref create(UnsignedInt, UnsignedInt, Float(*)(Float), Nanoseconds, Nanoseconds, DataHandle, UnsignedInt, AnimationFlags).
         */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > AnimationHandle create(StyleIndex sourceStyle, StyleIndex targetStyle, Float(*easing)(Float), Nanoseconds played, Nanoseconds duration, DataHandle data, UnsignedInt repeatCount = 1, AnimationFlags flags = {}) {
            return create(UnsignedInt(sourceStyle), UnsignedInt(targetStyle), easing, played, duration, data, repeatCount, flags);
        }

        /**
         * @brief Create an animation
         *
         * Same as calling @ref create(UnsignedInt, UnsignedInt, Float(*)(Float), Nanoseconds, Nanoseconds, DataHandle, UnsignedInt, AnimationFlags)
         * with @p repeatCount set to @cpp 1 @ce.
         */
        AnimationHandle create(UnsignedInt sourceStyle, UnsignedInt targetStyle, Float(*easing)(Float), Nanoseconds played, Nanoseconds duration, DataHandle data, AnimationFlags flags);

        /**
         * @brief Create an animation with a style index in a concrete enum type
         *
         * Casts @p sourceStyle and @p targetStyle to
         * @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref create(UnsignedInt, UnsignedInt, Float(*)(Float), Nanoseconds, Nanoseconds, DataHandle, AnimationFlags).
         */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > AnimationHandle create(StyleIndex sourceStyle, StyleIndex targetStyle, Float(*easing)(Float), Nanoseconds played, Nanoseconds duration, DataHandle data, AnimationFlags flags) {
            return create(UnsignedInt(sourceStyle), UnsignedInt(targetStyle), easing, played, duration, data, flags);
        }

        /**
         * @brief Create an animation assuming the data it's attached to belongs to the layer the animator is registered with
         *
         * Compared to @ref create(UnsignedInt, UnsignedInt, Float(*)(Float), Nanoseconds, Nanoseconds, DataHandle, UnsignedInt, AnimationFlags)
         * delegates to @ref AbstractAnimator::create(Nanoseconds, Nanoseconds, LayerDataHandle, UnsignedInt, AnimationFlags)
         * instead.
         */
        AnimationHandle create(UnsignedInt sourceStyle, UnsignedInt targetStyle, Float(*easing)(Float), Nanoseconds played, Nanoseconds duration, LayerDataHandle data, UnsignedInt repeatCount = 1, AnimationFlags flags = {});

        /**
         * @brief Create an animation with a style index in a concrete enum type assuming the data it's attached to belongs to the layer the animator is registered with
         *
         * Casts @p sourceStyle and @p targetStyle to
         * @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref create(UnsignedInt, UnsignedInt, Float(*)(Float), Nanoseconds, Nanoseconds, LayerDataHandle, UnsignedInt, AnimationFlags).
         */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > AnimationHandle create(StyleIndex sourceStyle, StyleIndex targetStyle, Float(*easing)(Float), Nanoseconds played, Nanoseconds duration, LayerDataHandle data, UnsignedInt repeatCount = 1, AnimationFlags flags = {}) {
            return create(UnsignedInt(sourceStyle), UnsignedInt(targetStyle), easing, played, duration, data, repeatCount, flags);
        }

        /**
         * @brief Create an animation assuming the data it's attached to belongs to the layer the animator is registered with
         *
         * Same as calling @ref create(UnsignedInt, UnsignedInt, Float(*)(Float), Nanoseconds, Nanoseconds, LayerDataHandle, UnsignedInt, AnimationFlags)
         * with @p repeatCount set to @cpp 1 @ce.
         */
        AnimationHandle create(UnsignedInt sourceStyle, UnsignedInt targetStyle, Float(*easing)(Float), Nanoseconds played, Nanoseconds duration, LayerDataHandle data, AnimationFlags flags);

        /**
         * @brief Create an animation with a style index in a concrete enum type assuming the data it's attached to belongs to the layer the animator is registered with
         *
         * Casts @p sourceStyle and @p targetStyle to
         * @relativeref{Magnum,UnsignedInt} and delegates to
         * @ref create(UnsignedInt, UnsignedInt, Float(*)(Float), Nanoseconds, Nanoseconds, LayerDataHandle, UnsignedInt, AnimationFlags).
         */
        template<class StyleIndex
            #ifndef DOXYGEN_GENERATING_OUTPUT
            , class = typename std::enable_if<std::is_enum<StyleIndex>::value>::type
            #endif
        > AnimationHandle create(StyleIndex sourceStyle, StyleIndex targetStyle, Float(*easing)(Float), Nanoseconds played, Nanoseconds duration, LayerDataHandle data, AnimationFlags flags) {
            return create(UnsignedInt(sourceStyle), UnsignedInt(targetStyle), easing, played, duration, data, flags);
        }

        /**
         * @brief Remove an animation
         *
         * Expects that @p handle is valid. Recycles a dynamic style used by
         * given animation with @ref TextLayer::recycleDynamicStyle() and
         * delegates to @ref AbstractAnimator::remove(AnimationHandle), see its
         * documentation for more information.
         *
         * @m_class{m-note m-warning}
         *
         * @par
         *      Note that removing an animation with this function doesn't
         *      cause any change to the style index of a @ref DataHandle it's
         *      attached to, if any. In other words, given data will still keep
         *      the original (dynamic) style index even after it's reused by a
         *      different animation. To fix this, either call
         *      @ref TextLayer::setStyle() to change the style to a different
         *      one afterwards or @ref stop() the animation instead ---
         *      assuming @ref AnimationFlag::KeepOncePlayed isn't set, it will
         *      cause the animation to gracefully switch to the target style
         *      during the next @ref advance(), and then be removed
         *      automatically.
         */
        void remove(AnimationHandle handle);

        /**
         * @brief Remove an animation assuming it belongs to this animator
         *
         * Compared to @ref remove(AnimationHandle) delegates to
         * @ref AbstractAnimator::remove(AnimatorDataHandle) instead.
         */
        void remove(AnimatorDataHandle handle);

        /**
         * @brief Animation easing function
         *
         * Expects that @p handle is valid. The returned pointer is never
         * @cpp nullptr @ce.
         */
        auto easing(AnimationHandle handle) const -> Float(*)(Float);

        /**
         * @brief Animation easing function assuming it belongs to this animator
         *
         * Like @ref easing(AnimationHandle) const but without checking that
         * @p handle indeed belongs to this animator. See its documentation for
         * more information.
         * @see @ref animationHandleData()
         */
        auto easing(AnimatorDataHandle handle) const -> Float(*)(Float);

        /**
         * @brief Animation source and target uniforms
         *
         * Expects that @p handle is valid. The uniforms are queried from
         * @ref TextLayer::Shared based on style IDs passed to @ref create().
         * @see @ref paddings(), @ref cursorUniforms(),
         *      @ref selectionUniforms(), @ref selectionTextUniforms()
         */
        Containers::Pair<TextLayerStyleUniform, TextLayerStyleUniform> uniforms(AnimationHandle handle) const;

        /**
         * @brief Animation source and target uniforms assuming it belongs to this animator
         *
         * Like @ref uniforms(AnimationHandle) const but without checking that
         * @p handle indeed belongs to this animator. See its documentation for
         * more information.
         * @see @ref animationHandleData()
         */
        Containers::Pair<TextLayerStyleUniform, TextLayerStyleUniform> uniforms(AnimatorDataHandle handle) const;

        /**
         * @brief Animation source and target paddings
         *
         * Expects that @p handle is valid. The paddings are queried from
         * @ref TextLayer::Shared based on style IDs passed to @ref create().
         * @see @ref uniforms(), @ref cursorPaddings(),
         *      @ref selectionPaddings()
         */
        Containers::Pair<Vector4, Vector4> paddings(AnimationHandle handle) const;

        /**
         * @brief Animation source and target paddings assuming it belongs to this animator
         *
         * Like @ref paddings(AnimationHandle) const but without checking that
         * @p handle indeed belongs to this animator. See its documentation for
         * more information.
         * @see @ref animationHandleData()
         */
        Containers::Pair<Vector4, Vector4> paddings(AnimatorDataHandle handle) const;

        /**
         * @brief Animation source and destination cursor uniforms
         *
         * Expects that @p handle is valid. The uniforms are queried from
         * @ref TextLayer::Shared based on style IDs passed to @ref create().
         * If given animated style doesn't have an associated cursor style,
         * returns @relativeref{Corrade,Containers::NullOpt}.
         * @see @ref cursorPaddings(), @ref selectionUniforms(),
         *      @ref selectionTextUniforms(), @ref uniforms()
         */
        Containers::Optional<Containers::Pair<TextLayerEditingStyleUniform, TextLayerEditingStyleUniform>> cursorUniforms(AnimationHandle handle) const;

        /**
         * @brief Animation source and destination cursor uniforms assuming it belongs to this animator
         *
         * Like @ref cursorUniforms(AnimationHandle) const but without checking
         * that @p handle indeed belongs to this animator. See its
         * documentation for more information.
         * @see @ref animationHandleData()
         */
        Containers::Optional<Containers::Pair<TextLayerEditingStyleUniform, TextLayerEditingStyleUniform>> cursorUniforms(AnimatorDataHandle handle) const;

        /**
         * @brief Animation source and destination cursor paddings
         *
         * Expects that @p handle is valid. The paddings are queried from
         * @ref TextLayer::Shared based on style IDs passed to @ref create().
         * If given animated style doesn't have an associated cursor style,
         * returns @relativeref{Corrade,Containers::NullOpt}.
         * @see @ref cursorUniforms(), @ref selectionPaddings(),
         *      @ref paddings()
         */
        Containers::Optional<Containers::Pair<Vector4, Vector4>> cursorPaddings(AnimationHandle handle) const;

        /**
         * @brief Animation source and destination cursor paddings assuming it belongs to this animator
         *
         * Like @ref cursorPaddings(AnimationHandle) const but without checking
         * that @p handle indeed belongs to this animator. See its
         * documentation for more information.
         * @see @ref animationHandleData()
         */
        Containers::Optional<Containers::Pair<Vector4, Vector4>> cursorPaddings(AnimatorDataHandle handle) const;

        /**
         * @brief Animation source and destination selection uniforms
         *
         * Expects that @p handle is valid. The uniforms are queried from
         * @ref TextLayer::Shared based on style IDs passed to @ref create().
         * If given animated style doesn't have an associated selection style,
         * returns @relativeref{Corrade,Containers::NullOpt}.
         * @see @ref selectionTextUniforms(), @ref selectionPaddings(),
         *      @ref cursorUniforms(), @ref uniforms()
         */
        Containers::Optional<Containers::Pair<TextLayerEditingStyleUniform, TextLayerEditingStyleUniform>> selectionUniforms(AnimationHandle handle) const;

        /**
         * @brief Animation source and destination selection uniforms assuming it belongs to this animator
         *
         * Like @ref selectionUniforms(AnimationHandle) const but without
         * checking that @p handle indeed belongs to this animator. See its
         * documentation for more information.
         * @see @ref animationHandleData()
         */
        Containers::Optional<Containers::Pair<TextLayerEditingStyleUniform, TextLayerEditingStyleUniform>> selectionUniforms(AnimatorDataHandle handle) const;

        /**
         * @brief Animation source and destination selection paddings
         *
         * Expects that @p handle is valid. The paddings are queried from
         * @ref TextLayer::Shared based on style IDs passed to @ref create().
         * If given animated style doesn't have an associated selection style,
         * returns @relativeref{Corrade,Containers::NullOpt}.
         * @see @ref selectionUniforms(), @ref selectionTextUniforms(),
         *      @ref cursorPaddings(), @ref paddings()
         */
        Containers::Optional<Containers::Pair<Vector4, Vector4>> selectionPaddings(AnimationHandle handle) const;

        /**
         * @brief Animation source and destination selection paddings assuming it belongs to this animator
         *
         * Like @ref selectionPaddings(AnimationHandle) const but without
         * checking that @p handle indeed belongs to this animator. See its
         * documentation for more information.
         * @see @ref animationHandleData()
         */
        Containers::Optional<Containers::Pair<Vector4, Vector4>> selectionPaddings(AnimatorDataHandle handle) const;

        /**
         * @brief Animation source and destination selection text uniforms
         *
         * Expects that @p handle is valid. The uniforms are queried from
         * @ref TextLayer::Shared based on style IDs passed to @ref create().
         * If given animated style doesn't have an associated selection style,
         * returns @relativeref{Corrade,Containers::NullOpt}.
         * @see @ref selectionUniforms(), @ref selectionPaddings(),
         *      @ref cursorUniforms(), @ref uniforms()
         */
        Containers::Optional<Containers::Pair<TextLayerStyleUniform, TextLayerStyleUniform>> selectionTextUniforms(AnimationHandle handle) const;

        /**
         * @brief Animation source and destination selection text uniforms assuming it belongs to this animator
         *
         * Like @ref selectionTextUniforms(AnimationHandle) const but without
         * checking that @p handle indeed belongs to this animator. See its
         * documentation for more information.
         * @see @ref animationHandleData()
         */
        Containers::Optional<Containers::Pair<TextLayerStyleUniform, TextLayerStyleUniform>> selectionTextUniforms(AnimatorDataHandle handle) const;

        /**
         * @brief Advance the animations
         * @param[in] active                    Animation IDs that are active
         * @param[in] factors                   Interpolation factors indexed
         *      by animation ID
         * @param[in] remove                    Animation IDs to be removed
         * @param[in,out] dynamicStyleUniforms  Uniforms to animate indexed by
         *      dynamic style ID or dynamic editing style text uniform ID
         * @param[in,out] dynamicStyleCursorStyles  Cursor style association to
         *      animate indexed by dynamic style ID
         * @param[in,out] dynamicStyleSelectionStyles  Selection style
         *      association to animate indexed by dynamic style ID
         * @param[in,out] dynamicStylePaddings  Paddings to animate indexed by
         *      dynamic style ID
         * @param[in,out] dynamicEditingStyleUniforms  Editing uniforms to
         *      animate indexed by dynamic editing style ID
         * @param[in,out] dynamicEditingStylePaddings  Editing paddings to
         *      animate indexed by dynamic editing style ID
         * @param[in,out] dataStyles            Style assignments of all layer
         *      data indexed by data ID
         * @return Style properties that were affected by the animation
         *
         * Used internally from @ref TextLayer::advanceAnimations(Nanoseconds, Containers::MutableBitArrayView, const Containers::StridedArrayView1D<Float>&, Containers::MutableBitArrayView, const Containers::Iterable<AbstractStyleAnimator>&),
         * which is called from @ref AbstractUserInterface::advanceAnimations().
         * Exposed just for testing purposes, there should be no need to call
         * this function directly and doing so may cause internal
         * @ref AbstractUserInterface state update to misbehave.
         *
         * Expects that size of @p active,  @p factors and @p remove matches
         * @ref capacity(), it's assumed that their contents were filled by
         * @ref update() before. Assuming the layer the animator is associated
         * with doesn't contain editing styles, the @p dynamicStyleUniforms,
         * @p dynamicStyleCursorStyles, @p dynamicStyleSelectionStyles and
         * @p dynamicStylePaddings, are expected to all have the same size and
         * should be large enough to contain any valid dynamic style ID, the
         * @p dynamicEditingStyleUniforms and @p dynamicEditingStylePaddings
         * views are then expected to be empty. Assuming the layer contains
         * editing styles, the @p dynamicStyleUniforms is expected to be three
         * times as large as the others, and the @p dynamicEditingStyleUniforms
         * and @p dynamicEditingStylePaddings views twice as large as the
         * others instead. The @p dataStyles view should be large enough to
         * contain any valid layer data ID.
         */
        TextLayerStyleAnimations advance(Containers::BitArrayView active, const Containers::StridedArrayView1D<const Float>& factors, Containers::BitArrayView remove, Containers::ArrayView<TextLayerStyleUniform> dynamicStyleUniforms, Containers::MutableBitArrayView dynamicStyleCursorStyles, Containers::MutableBitArrayView dynamicStyleSelectionStyles, const Containers::StridedArrayView1D<Vector4>& dynamicStylePaddings, Containers::ArrayView<TextLayerEditingStyleUniform> dynamicEditingStyleUniforms, const Containers::StridedArrayView1D<Vector4>& dynamicEditingStylePaddings, const Containers::StridedArrayView1D<UnsignedInt>& dataStyles);

    private:
        struct State;

        MAGNUM_UI_LOCAL void createInternal(AnimationHandle handle, UnsignedInt sourceStyle, UnsignedInt targetStyle, Float(*easing)(Float));

        /* used by foo(AnimationHandle) and foo(AnimatorDataHandle) */
        MAGNUM_UI_LOCAL Containers::Optional<Containers::Pair<TextLayerEditingStyleUniform, TextLayerEditingStyleUniform>> cursorUniformsInternal(UnsignedInt id) const;
        MAGNUM_UI_LOCAL Containers::Optional<Containers::Pair<Vector4, Vector4>> cursorPaddingsInternal(UnsignedInt id) const;
        MAGNUM_UI_LOCAL Containers::Optional<Containers::Pair<TextLayerEditingStyleUniform, TextLayerEditingStyleUniform>> selectionUniformsInternal(UnsignedInt id) const;
        MAGNUM_UI_LOCAL Containers::Optional<Containers::Pair<Vector4, Vector4>> selectionPaddingsInternal(UnsignedInt id) const;
        MAGNUM_UI_LOCAL Containers::Optional<Containers::Pair<TextLayerStyleUniform, TextLayerStyleUniform>> selectionTextUniformsInternal(UnsignedInt id) const;
};

}}

#endif
