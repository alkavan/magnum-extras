#ifndef Magnum_Ui_AbstractUserInterface_h
#define Magnum_Ui_AbstractUserInterface_h
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
 * @brief Class @ref Magnum::Ui::AbstractUserInterface, enum @ref Magnum::Ui::UserInterfaceState, enum set @ref Magnum::Ui::UserInterfaceStates
 * @m_since_latest
 */

#include <Corrade/Containers/EnumSet.h>
#include <Corrade/Containers/Pointer.h>
#include <Magnum/Magnum.h>

#include "Magnum/Ui/Ui.h"
#include "Magnum/Ui/visibility.h"

namespace Magnum { namespace Ui {

/**
@brief User interface state
@m_since_latest

Used to decide whether @ref AbstractUserInterface::clean() or
@ref AbstractUserInterface::update() need to be called to refresh the internal
state before the interface is drawn or an event is handled. See also
@ref LayerState for layer-specific state, @ref LayouterState for
layouter-specific state and @ref AnimatorState for animator-specific state.
@see @ref UserInterfaceStates, @ref AbstractUserInterface::state()
*/
enum class UserInterfaceState: UnsignedShort {
    /**
     * @ref AbstractUserInterface::update() needs to be called to recalculate
     * or reupload data attached to visible node hierarchy after they've been
     * changed. Set implicitly if any of the layers have
     * @ref LayerState::NeedsDataUpdate,
     * @relativeref{LayerState,NeedsCommonDataUpdate} or
     * @relativeref{LayerState,NeedsSharedDataUpdate} set, is reset next time
     * @ref AbstractUserInterface::update() is called. Implied by
     * @ref UserInterfaceState::NeedsDataAttachmentUpdate,
     * @relativeref{UserInterfaceState,NeedsNodeEnabledUpdate},
     * @relativeref{UserInterfaceState,NeedsNodeClipUpdate},
     * @relativeref{UserInterfaceState,NeedsLayoutUpdate},
     * @relativeref{UserInterfaceState,NeedsLayoutAssignmentUpdate},
     * @relativeref{UserInterfaceState,NeedsNodeUpdate} and
     * @relativeref{UserInterfaceState,NeedsNodeClean}, so it's also set by
     * everything that sets those flags.
     */
    NeedsDataUpdate = 1 << 0,

    /**
     * @ref AbstractUserInterface::update() needs to be called to refresh the
     * data attached to visible node hierarchy after new data were attached or
     * after existing attachments were removed and
     * @ref AbstractUserInterface::clean() was called. Set implicitly if any of
     * the layers have @ref LayerState::NeedsAttachmentUpdate set, after every
     * @ref AbstractUserInterface::removeLayer() call and transitively after
     * every @ref AbstractUserInterface::attachData() call, is reset next time
     * @ref AbstractUserInterface::update() is called. Implies
     * @ref UserInterfaceState::NeedsDataUpdate. Implied by
     * @relativeref{UserInterfaceState,NeedsNodeEnabledUpdate},
     * @relativeref{UserInterfaceState,NeedsNodeClipUpdate},
     * @relativeref{UserInterfaceState,NeedsLayoutUpdate},
     * @relativeref{UserInterfaceState,NeedsLayoutAssignmentUpdate},
     * @relativeref{UserInterfaceState,NeedsNodeUpdate} and
     * @relativeref{UserInterfaceState,NeedsNodeClean}, so it's also set by
     * everything that sets those flags.
     */
    NeedsDataAttachmentUpdate = NeedsDataUpdate|(1 << 1),

    /**
     * @ref AbstractUserInterface::update() needs to be called to refresh the
     * enabled node set after node flags changed. Set implicitly after every
     * @relativeref{AbstractUserInterface,setNodeFlags()},
     * @relativeref{AbstractUserInterface,addNodeFlags()} and
     * @relativeref{AbstractUserInterface,clearNodeFlags()} that changes the
     * presence of the @ref NodeFlag::NoEvents, @ref NodeFlag::Disabled or
     * @ref NodeFlag::Focusable flag; is reset next time
     * @ref AbstractUserInterface::update() is called. Implies
     * @ref UserInterfaceState::NeedsDataAttachmentUpdate. Implied by
     * @relativeref{UserInterfaceState,NeedsNodeClipUpdate},
     * @relativeref{UserInterfaceState,NeedsLayoutUpdate},
     * @relativeref{UserInterfaceState,NeedsLayoutAssignmentUpdate},
     * @relativeref{UserInterfaceState,NeedsNodeUpdate} and
     * @relativeref{UserInterfaceState,NeedsNodeClean}, so it's also set by
     * everything that sets those flags.
     */
    NeedsNodeEnabledUpdate = NeedsDataAttachmentUpdate|(1 << 2),

    /**
     * @ref AbstractUserInterface::update() needs to be called to refresh the
     * visible node set after the user interface size or node flags changed.
     * Set implicitly after every @relativeref{AbstractUserInterface,setSize()}
     * call that changes the user interface size if any nodes were already
     * created and after every
     * @relativeref{AbstractUserInterface,setNodeFlags()},
     * @relativeref{AbstractUserInterface,addNodeFlags()} and
     * @relativeref{AbstractUserInterface,clearNodeFlags()} that changes the
     * presence of the @ref NodeFlag::Clip flag; is reset next time
     * @ref AbstractUserInterface::update() is called. Implies
     * @ref UserInterfaceState::NeedsNodeEnabledUpdate. Implied by
     * @relativeref{UserInterfaceState,NeedsLayoutUpdate},
     * @relativeref{UserInterfaceState,NeedsLayoutAssignmentUpdate},
     * @relativeref{UserInterfaceState,NeedsNodeUpdate} and
     * @relativeref{UserInterfaceState,NeedsNodeClean}, so it's also set by
     * everything that sets those flags.
     */
    NeedsNodeClipUpdate = NeedsNodeEnabledUpdate|(1 << 3),

    /**
     * @ref AbstractUserInterface::update() needs to be called to refresh the
     * visible node hierarchy layout after node sizes or offsets changed. Set
     * implicitly if any of the layouters have
     * @ref LayouterState::NeedsUpdate set and after every
     * @ref AbstractUserInterface::setNodeOffset() and
     * @ref AbstractUserInterface::setNodeSize(), is reset next time
     * @ref AbstractUserInterface::update() is called. Implies
     * @ref UserInterfaceState::NeedsNodeClipUpdate. Implied by
     * @relativeref{UserInterfaceState,NeedsLayoutAssignmentUpdate},
     * @relativeref{UserInterfaceState,NeedsNodeUpdate} and
     * @relativeref{UserInterfaceState,NeedsNodeClean}, so it's also set by
     * everything that sets those flags.
     */
    NeedsLayoutUpdate = NeedsNodeClipUpdate|(1 << 4),

    /**
     * @ref AbstractUserInterface::update() needs to be called to refresh the
     * layouts assigned to visible node hierarchy after new layouts were added
     * or after existing layouts were removed and
     * @ref AbstractUserInterface::clean() was called. Set implicitly if any of
     * the layouters have @ref LayouterState::NeedsAssignmentUpdate set and
     * after every @ref AbstractUserInterface::removeLayouter() call, is reset
     * next time @ref AbstractUserInterface::update() is called. Implies
     * @ref UserInterfaceState::NeedsLayoutUpdate. Implied by
     * @relativeref{UserInterfaceState,NeedsNodeUpdate} and
     * @relativeref{UserInterfaceState,NeedsNodeClean}, so it's also set by
     * everything that sets those flags.
     */
    NeedsLayoutAssignmentUpdate = NeedsLayoutUpdate|(1 << 5),

    /**
     * @ref AbstractUserInterface::update() needs to be called to refresh the
     * the hierarchical opacity information after node opacity was changed. Set
     * implicitly after every @ref AbstractUserInterface::setNodeOpacity()
     * call, is reset next time @ref AbstractUserInterface::update() is
     * called. Implies @ref UserInterfaceState::NeedsDataUpdate. Implied by
     * @relativeref{UserInterfaceState,NeedsNodeUpdate} and
     * @relativeref{UserInterfaceState,NeedsNodeClean}, so it's also set by
     * everything that sets those flags.
     */
    NeedsNodeOpacityUpdate = NeedsDataUpdate|(1 << 6),

    /**
     * @ref AbstractUserInterface::update() needs to be called to refresh the
     * visible node hierarchy and data attached to it after nodes were added or
     * removed, made hidden or visible again or the top-level node order
     * changed. Set implicitly after every
     * @ref AbstractUserInterface::createNode(),
     * @relativeref{AbstractUserInterface,setNodeOrder()} and
     * @relativeref{AbstractUserInterface,clearNodeOrder()} call and after
     * every @relativeref{AbstractUserInterface,setNodeFlags()},
     * @relativeref{AbstractUserInterface,addNodeFlags()} and
     * @relativeref{AbstractUserInterface,clearNodeFlags()} call that changes
     * the presence of the @ref NodeFlag::Hidden flag; is reset next time
     * @ref AbstractUserInterface::update() is called. Implies
     * @ref UserInterfaceState::NeedsLayoutAssignmentUpdate. Implied by
     * @relativeref{UserInterfaceState,NeedsNodeClean}, so it's also set by
     * everything that sets that flag.
     */
    NeedsNodeUpdate = NeedsLayoutAssignmentUpdate|NeedsNodeOpacityUpdate|(1 << 7),

    /**
     * @ref AbstractUserInterface::clean() needs to be called to prune
     * animations attached to removed data. Set implicitly if any of the layers
     * have @ref LayerState::NeedsDataClean set and transitively after every
     * @ref AbstractUserInterface::removeNode() call, is reset next time
     * @ref AbstractUserInterface::clean() is called. Implied by
     * @ref UserInterfaceState::NeedsNodeClean.
     */
    NeedsDataClean = 1 << 8,

    /**
     * @ref AbstractUserInterface::clean() needs to be called to prune child
     * hierarchies of removed nodes and data, layouts and animation assigned to
     * those. Set implicitly after every
     * @ref AbstractUserInterface::removeNode() call, is reset to
     * @ref UserInterfaceState::NeedsNodeUpdate next time
     * @ref AbstractUserInterface::clean() is called. Implies
     * @ref UserInterfaceState::NeedsNodeUpdate and
     * @relativeref{UserInterfaceState,NeedsDataClean}.
     */
    NeedsNodeClean = NeedsNodeUpdate|NeedsDataClean|(1 << 9),

    /**
     * @ref AbstractUserInterface::advanceAnimations() needs to be called to
     * advance active animations. Set implicitly if any of the animators have
     * @ref AnimatorState::NeedsAdvance set, is reset next time
     * @ref AbstractUserInterface::advanceAnimations() is called if no
     * animations are @ref AnimationState::Scheduled,
     * @ref AnimationState::Playing or @ref AnimationState::Paused anymore.
     */
    NeedsAnimationAdvance = 1 << 10,
};

/**
@debugoperatorenum{UserInterfaceState}
@m_since_latest
*/
MAGNUM_UI_EXPORT Debug& operator<<(Debug& debug, UserInterfaceState value);

/**
@brief User interface states
@m_since_latest

@see @ref AbstractUserInterface::state()
*/
typedef Containers::EnumSet<UserInterfaceState> UserInterfaceStates;

/**
@debugoperatorenum{UserInterfaceStates}
@m_since_latest
*/
MAGNUM_UI_EXPORT Debug& operator<<(Debug& debug, UserInterfaceStates value);

CORRADE_ENUMSET_OPERATORS(UserInterfaceStates)

namespace Implementation {
    template<class, class = void> struct PointerEventConverter;
    template<class, class = void> struct PointerMoveEventConverter;
    template<class, class = void> struct KeyEventConverter;
    template<class, class = void> struct TextInputEventConverter;
}

/**
@brief Base for the main user interface
@m_since_latest

Doesn't contain any implicit layers, layouters or animators. You'll most likely
want to instantiate the @ref UserInterface subclass instead.

@section Ui-AbstractUserInterface-application Integration with application libraries

By including @ref Magnum/Ui/Application.h it's possible to pass event
instances from @ref Platform::Sdl2Application "Platform::*Application" classes directly to event handlers. They get internally converted to a corresponding
@ref PointerEvent, @ref PointerMoveEvent, @ref KeyEvent or @ref TextInputEvent
instance with a subset of information given application provides. Then, if the
event is accepted by the user interface, the original application event is
marked as accepted as well, to prevent it from propagating further in certain
circumstances (such as to the browser window when compiling for the web).
Example usage:

@snippet Ui-sdl2.cpp AbstractUserInterface-application-events

Currently, none of the application classes expose event timestamps and thus
@ref PointerEvent::time() and the corresponding field in all other events is
default-constructed. If timestamps are needed, for example for animations, they
can be supplied as a second @ref Nanoseconds argument when calling
@ref pointerPressEvent(Event&, Args&&... args) and others.

@section Ui-AbstractUserInterface-dpi DPI awareness

There are three separate concepts for DPI-aware UI rendering:

-   UI size --- size of the user interface to which all widgets are positioned
-   Window size --- size of the window to which all input events are related
-   Framebuffer size --- size of the framebuffer the UI is being rendered to

Depending on the platform and use case, each of these three values can be
different. For example, a game menu screen can have the UI size the same
regardless of window size. Or on Retina macOS you can have different window and
framebuffer size and the UI size might be related to window size but
independent on the framebuffer size.

When using for example @ref Platform::Sdl2Application or other `*Application`
implementations, you usually have three values at your disposal ---
@ref Platform::Sdl2Application::windowSize() "windowSize()",
@ref Platform::Sdl2Application::framebufferSize() "framebufferSize()" and
@ref Platform::Sdl2Application::dpiScaling() "dpiScaling()". If you want the UI
to have the same layout and just scale on bigger window sizes, pass a fixed
value to the UI size:

@snippet Ui-sdl2.cpp AbstractUserInterface-dpi-fixed

If you want the UI to get more room with larger window sizes and behave
properly with different DPI scaling values, pass a ratio of window size and DPI
scaling to the UI size:

@snippet Ui-sdl2.cpp AbstractUserInterface-dpi-ratio

Finally, to gracefully deal with extremely small or extremely large windows,
you can apply @ref Math::clamp() on top with some defined bounds. Windows
outside of the reasonable size range will then get a scaled version of the UI
at boundary size:

@snippet Ui-sdl2.cpp AbstractUserInterface-dpi-clamp
*/
class MAGNUM_UI_EXPORT AbstractUserInterface {
    public:
        /**
         * @brief Construct without creating the user interface with concrete parameters
         *
         * You're expected to call @ref setSize() afterwards in order to define
         * scaling of event coordinates, node positions and projection matrices
         * for drawing.
         */
        explicit AbstractUserInterface(NoCreateT);

        /**
         * @brief Construct
         * @param size                  Size of the user interface to which
         *      everything is positioned
         * @param windowSize            Size of the window to which all input
         *      events are related
         * @param framebufferSize       Size of the window framebuffer. On
         *      some platforms with HiDPI screens may be different from window
         *      size.
         *
         * Equivalent to constructing with @ref AbstractUserInterface(NoCreateT)
         * and then calling @ref setSize(const Vector2&, const Vector2&, const Vector2i&).
         * See its documentation for more information.
         */
        explicit AbstractUserInterface(const Vector2& size, const Vector2& windowSize, const Vector2i& framebufferSize);

        /**
         * @brief Construct with an unscaled size
         *
         * Delegates to @ref AbstractUserInterface(const Vector2&, const Vector2&, const Vector2i&)
         * with all sizes set to @p size. Doing so assumes that the coordinate
         * system in which events are passed matches framebuffer size.
         */
        explicit AbstractUserInterface(const Vector2i& size);

        /** @brief Copying is not allowed */
        AbstractUserInterface(const AbstractUserInterface&) = delete;

        /**
         * @brief Move constructor
         *
         * Performs a destructive move, i.e. the original object isn't usable
         * afterwards anymore.
         */
        AbstractUserInterface(AbstractUserInterface&&) noexcept;

        /** @brief Copying is not allowed */
        AbstractUserInterface& operator=(const AbstractUserInterface&) = delete;

        /** @brief Move assignment */
        AbstractUserInterface& operator=(AbstractUserInterface&&) noexcept;

        ~AbstractUserInterface();

        /**
         * @brief User interface size
         *
         * Node positioning is in respect to this size. If @ref setSize() or
         * @ref AbstractUserInterface(const Vector2&, const Vector2&, const Vector2i&)
         * wasn't called yet, initial value is a zero vector.
         */
        Vector2 size() const;

        /**
         * @brief Window size
         *
         * Global event position in @ref pointerPressEvent(),
         * @ref pointerReleaseEvent() and @ref pointerMoveEvent() is in respect
         * to this size. If @ref setSize() or @ref AbstractUserInterface(const Vector2&, const Vector2&, const Vector2i&)
         * wasn't called yet, initial value is a zero vector.
         */
        Vector2 windowSize() const;

        /**
         * @brief Framebuffer size
         *
         * Rendering performed by layers is in respect to this size. If
         * @ref setSize() or
         * @ref AbstractUserInterface(const Vector2&, const Vector2&, const Vector2i&)
         * wasn't called yet, initial value is a zero vector.
         */
        Vector2i framebufferSize() const;

        /**
         * @brief Set user interface size
         * @param size                  Size of the user interface to which
         *      everything is positioned
         * @param windowSize            Size of the window to which all input
         *      events are related
         * @param framebufferSize       Size of the window framebuffer. On
         *      some platforms with HiDPI screens may be different from window
         *      size.
         * @return Reference to self (for method chaining)
         *
         * All sizes are expected to be non-zero, origin is top left for all.
         *
         * After calling this function, the @ref pointerPressEvent(),
         * @ref pointerReleaseEvent() and @ref pointerMoveEvent() functions
         * take the global event position with respect to @p windowSize, which
         * is then rescaled to match @p size when exposed through
         * @ref PointerEvent. The @p size and @p framebufferSize is passed
         * through to @ref AbstractLayer::setSize() to all layers with
         * @ref LayerFeature::Draw so they can set appropriate projection and
         * other framebuffer-related properties, similarly the @p size is
         * passed through to @ref AbstractLayouter::setSize() to all layouters.
         *
         * There's no default size and this function is expected to be called
         * before the first @ref update() happens, either directly or through
         * the @ref AbstractUserInterface(const Vector2&, const Vector2&, const Vector2i&)
         * constructor. It's allowed to call this function for the first time
         * even after node, layers or data were created.
         *
         * Calling this function with new values will update the event position
         * scaling accordingly. If @p size or @p framebufferSize changes,
         * @ref AbstractLayer::setSize() is called on all layers; if @p size
         * changes, @ref AbstractLayouter::setSize() is called on all
         * layouters. If @p size changes and any nodes were already created,
         * @ref UserInterfaceState::NeedsNodeClipUpdate is set. If a renderer
         * instance is set, @ref AbstractRenderer::setupFramebuffers() is
         * called to make the renderer populate or update its internal state.
         * If a renderer instance isn't set yet when calling this function, the
         * framebuffer setup is performed in the next
         * @ref setRendererInstance() call instead.
         */
        AbstractUserInterface& setSize(const Vector2& size, const Vector2& windowSize, const Vector2i& framebufferSize);

        /**
         * @brief Set unscaled user interface size
         * @return Reference to self (for method chaining)
         *
         * Calls @ref setSize(const Vector2&, const Vector2&, const Vector2i&)
         * with all sizes set to @p size. Doing so assumes that the coordinate
         * system in which events are passed matches framebuffer size.
         */
        AbstractUserInterface& setSize(const Vector2i& size);

        /**
         * @brief User interface state
         *
         * See the @ref UserInterfaceState enum for more information. By
         * default no flags are set.
         */
        UserInterfaceStates state() const;

        /**
         * @brief Animation time
         *
         * Time value last passed to @ref advanceAnimations(). Initial value is
         * @cpp 0_nsec @ce.
         * @see @ref AbstractAnimator::time()
         */
        Nanoseconds animationTime() const;

        /** @{
         * @name Renderer management
         */

        /**
         * @brief Set renderer instance
         *
         * Expects that the instance hasn't been set yet. A renderer instance
         * has to be set in order to draw anything, it's the user
         * responsibility to ensure that the GPU API used by the renderer
         * matches the GPU API used by all layer instances, such as
         * @ref RendererGL being used for @ref BaseLayerGL and
         * @ref TextLayerGL. The instance is subsequently available through
         * @ref renderer().
         *
         * If framebuffer size was set with @ref setSize() already, calling
         * this function causes @ref AbstractRenderer::setupFramebuffers() to
         * be called. Otherwise the setup gets performed in the next
         * @ref setSize() call.
         * @see @ref hasRenderer()
         */
        AbstractRenderer& setRendererInstance(Containers::Pointer<AbstractRenderer>&& instance);
        /** @overload */
        template<class T> T& setRendererInstance(Containers::Pointer<T>&& instance) {
            return static_cast<T&>(setRendererInstance(Containers::Pointer<AbstractRenderer>{Utility::move(instance)}));
        }

        /**
         * @brief Whether a renderer instance has been set
         *
         * @see @ref renderer(), @ref setRendererInstance()
         */
        bool hasRenderer() const;

        /**
         * @brief Renderer instance
         *
         * Expects that @ref setRendererInstance() was called.
         * @see @ref UserInterfaceGL::renderer()
         */
        AbstractRenderer& renderer();
        const AbstractRenderer& renderer() const; /**< @overload */

        /**
         * @brief Renderer instance in a concrete type
         *
         * Expected that @ref setRendererInstance() was called. It's the user
         * responsibility to ensure that @p T matches the actual instance type.
         */
        template<class T> T& renderer() {
            return static_cast<T&>(renderer());
        }
        /** @overload */
        template<class T> const T& renderer() const {
            return static_cast<const T&>(renderer());
        }

        /**
         * @}
         */

        /** @{
         * @name Layer and data management
         */

        /**
         * @brief Capacity of the layer storage
         *
         * Can be at most 256. If @ref createLayer() is called and there's no
         * free slots left, the internal storage gets grown.
         * @see @ref layerUsedCount()
         */
        std::size_t layerCapacity() const;

        /**
         * @brief Count of used items in the layer storage
         *
         * Always at most @ref layerCapacity(). Expired handles are counted
         * among used as well. The operation is done with a
         * @f$ \mathcal{O}(n) @f$ complexity where @f$ n @f$ is
         * @ref layerCapacity().
         */
        std::size_t layerUsedCount() const;

        /**
         * @brief Whether a layer handle is valid
         *
         * A handle is valid if it has been returned from @ref createLayer()
         * before and @ref removeLayer() wasn't called on it yet. Note that a
         * handle is valid even if the layer instance wasn't set with
         * @ref setLayerInstance() yet. For @ref LayerHandle::Null always
         * returns @cpp false @ce.
         */
        bool isHandleValid(LayerHandle handle) const;

        /**
         * @brief Whether a data handle is valid
         *
         * A shorthand for extracting a @ref LayerHandle from @p handle using
         * @ref dataHandleLayer(), calling @ref isHandleValid(LayerHandle) const
         * on it, if it's valid and set then retrieving the particular layer
         * instance using @ref layer() and then calling
         * @ref AbstractLayer::isHandleValid(LayerDataHandle) const with a
         * @ref LayerDataHandle extracted from @p handle using
         * @ref dataHandleData(). See these functions for more information. For
         * @ref DataHandle::Null, @ref LayerHandle::Null or
         * @ref LayerDataHandle::Null always returns @cpp false @ce.
         */
        bool isHandleValid(DataHandle handle) const;

        /**
         * @brief First layer in draw and event processing order
         *
         * The first layer gets drawn first (thus is at the back) and reacts to
         * events after all others. Returns @ref LayerHandle::Null if there's
         * no layers yet. The returned handle is always either valid or null.
         * @see @ref layerNext(), @ref layerLast(), @ref layerPrevious()
         */
        LayerHandle layerFirst() const;

        /**
         * @brief Last layer in draw and event processing order
         *
         * The last layer gets drawn last (thus is at the front) and reacts to
         * event before all others. Returns @ref LayerHandle::Null if there's
         * no layers yet. The returned handle is always either valid or null.
         * @see @ref layerPrevious(), @ref layerFirst(), @ref layerNext()
         */
        LayerHandle layerLast() const;

        /**
         * @brief Previous layer in draw and event processing order
         *
         * The previous layer gets drawn earlier (thus is behind) and reacts to
         * events later. Expects that @p handle is valid. Returns
         * @ref LayerHandle::Null if the layer is first. The returned handle
         * is always either valid or null.
         * @see @ref isHandleValid(LayerHandle) const, @ref layerNext(),
         *      @ref layerFirst(), @ref layerLast()
         */
        LayerHandle layerPrevious(LayerHandle handle) const;

        /**
         * @brief Next layer in draw and event processing order
         *
         * The next layer gets drawn later (thus is in front) and reacts to
         * events earlier. Expects that @p handle is valid. Returns
         * @ref LayerHandle::Null if the layer is last. The returned handle is
         * always either valid or null.
         * @see @ref isHandleValid(LayerHandle) const, @ref layerPrevious(),
         *      @ref layerLast(), @ref layerFirst()
         */
        LayerHandle layerNext(LayerHandle handle) const;

        /**
         * @brief Create a layer
         * @param before    A layer to order before for draw and event
         *      processing or @ref LayerHandle::Null if ordered as last (i.e.,
         *      at the front, being drawn last and receiving events first).
         *      Expected to be valid if not null.
         * @return New layer handle
         *
         * Allocates a new handle in a free slot in the internal storage or
         * grows the storage if there's no free slots left. Expects that
         * there's at most 256 layers. The returned handle is meant to be used
         * to construct an @ref AbstractLayer subclass and the instance then
         * passed to @ref setLayerInstance(). A layer can be removed again with
         * @ref removeLayer().
         * @see @ref isHandleValid(LayerHandle) const,
         *      @ref layerCapacity(), @ref layerUsedCount()
         */
        LayerHandle createLayer(LayerHandle before =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            LayerHandle::Null
            #else
            LayerHandle{} /* To not have to include Handle.h */
            #endif
        );

        /**
         * @brief Set a layer instance
         * @return Reference to @p instance
         *
         * Expects that @p instance was created with a @ref LayerHandle
         * returned from @ref createLayer() earlier, the handle is valid and
         * @ref setLayerInstance() wasn't called for the same handle yet.
         *
         * Calls @ref AbstractLayer::setSize() on the layer, unless neither
         * @ref setSize() nor @ref AbstractUserInterface(const Vector2&, const Vector2&, const Vector2i&)
         * was called yet.
         * @see @ref AbstractLayer::handle(),
         *      @ref isHandleValid(LayerHandle) const
         */
        AbstractLayer& setLayerInstance(Containers::Pointer<AbstractLayer>&& instance);
        /** @overload */
        template<class T> T& setLayerInstance(Containers::Pointer<T>&& instance) {
            return static_cast<T&>(setLayerInstance(Containers::Pointer<AbstractLayer>{Utility::move(instance)}));
        }

        /**
         * @brief Layer instance
         *
         * Expects that @p handle is valid and that @ref setLayerInstance() was
         * called for it.
         * @see @ref isHandleValid(LayerHandle) const
         */
        AbstractLayer& layer(LayerHandle handle);
        const AbstractLayer& layer(LayerHandle handle) const; /**< @overload */

        /**
         * @brief Layer instance in a concrete type
         *
         * Expects that @p handle is valid and that @ref setLayerInstance() was
         * called for it. It's the user responsibility to ensure that @p T
         * matches the actual instance type.
         * @see @ref isHandleValid(LayerHandle) const
         */
        template<class T> T& layer(LayerHandle handle) {
            return static_cast<T&>(layer(handle));
        }
        /** @overload */
        template<class T> const T& layer(LayerHandle handle) const {
            return static_cast<const T&>(layer(handle));
        }

        /**
         * @brief Remove a layer
         *
         * Expects that @p handle is valid. After this call,
         * @ref isHandleValid(LayerHandle) const returns @cpp false @ce for
         * @p handle and @ref isHandleValid(DataHandle) const returns
         * @cpp false @ce for all data associated with @p handle.
         *
         * Animators with @ref AnimatorFeature::DataAttachment that were
         * associated with this layers are kept, but are excluded from any
         * further processing in @ref clean() or @ref advanceAnimations().
         * Calling @ref removeAnimator() on these is left to the user.
         *
         * Calling this function causes
         * @ref UserInterfaceState::NeedsDataAttachmentUpdate to be set.
         * @see @ref clean()
         */
        void removeLayer(LayerHandle handle);

        /**
         * @brief Attach data to a node
         *
         * A shorthand for extracting a @ref LayerHandle from @p data using
         * @ref dataHandleLayer(), retrieving the particular layer instance
         * using @ref layer() and then calling
         * @ref AbstractLayer::attach(LayerDataHandle, NodeHandle) with a
         * @ref LayerDataHandle extracted with @ref dataHandleData(). See these
         * functions for more information. In addition to
         * @ref AbstractLayer::attach(LayerDataHandle, NodeHandle), this
         * function checks that @p node is either valid or
         * @ref NodeHandle::Null.
         *
         * Calling this function transitively causes
         * @ref UserInterfaceState::NeedsDataAttachmentUpdate to be set, which
         * is a consequence of @ref LayerState::NeedsAttachmentUpdate being set
         * by @ref AbstractLayer::attach().
         * @see @ref update()
         */
        void attachData(NodeHandle node, DataHandle data);

        /**
         * @}
         */

        /** @{
         * @name Node layouter management
         */

        /**
         * @brief Capacity of the layouter storage
         *
         * Can be at most 256. If @ref createLayouter() is called and there's
         * no free slots left, the internal storage gets grown.
         * @see @ref layouterUsedCount()
         */
        std::size_t layouterCapacity() const;

        /**
         * @brief Count of used items in the layouter storage
         *
         * Always at most @ref layouterCapacity(). Expired handles are counted
         * among used as well. The operation is done with a
         * @f$ \mathcal{O}(n) @f$ complexity where @f$ n @f$ is
         * @ref layouterCapacity().
         */
        std::size_t layouterUsedCount() const;

        /**
         * @brief Whether a layouter handle is valid
         *
         * A handle is valid if it has been returned from @ref createLayouter()
         * before and @ref removeLayouter() wasn't called on it yet. Note that
         * a handle is valid even if the layouter instance wasn't set with
         * @ref setLayouterInstance() yet. For @ref LayouterHandle::Null always
         * returns @cpp false @ce.
         */
        bool isHandleValid(LayouterHandle handle) const;

        /**
         * @brief Whether a layout handle is valid
         *
         * A shorthand for extracting a @ref LayouterHandle from @p handle
         * using @ref layoutHandleLayouter(), calling
         * @ref isHandleValid(LayouterHandle) const on it, if it's valid and
         * set then retrieving the particular layouter instance using
         * @ref layouter() and then calling
         * @ref AbstractLayouter::isHandleValid(LayouterDataHandle) const with
         * a @ref LayouterDataHandle extracted from @p handle using
         * @ref layoutHandleData(). See these functions for more information.
         * For @ref LayoutHandle::Null, @ref LayouterHandle::Null or
         * @ref LayouterDataHandle::Null always returns @cpp false @ce.
         */
        bool isHandleValid(LayoutHandle handle) const;

        /**
         * @brief First layouter in the layout calculation order
         *
         * This layouter gets executed before all others. Returns
         * @ref LayouterHandle::Null if there's no layouters. The returned
         * handle is always either valid or null.
         * @see @ref layouterNext(), @ref layouterLast(),
         *      @ref layouterPrevious()
         */
        LayouterHandle layouterFirst() const;

        /**
         * @brief Last layouter in the layout calculation order
         *
         * This layouter gets executed after all others. Returns
         * @ref LayouterHandle::Null if there's no layouters. The returned
         * handle is always either valid or null.
         * @see @ref layouterPrevious(), @ref layouterFirst(),
         *      @ref layouterNext()
         */
        LayouterHandle layouterLast() const;

        /**
         * @brief Previous layouter in the layout calculation order
         *
         * The previous layouter gets executed earlier. Expects that @p handle
         * is valid. Returns @ref LayouterHandle::Null if the layouter is
         * first. The returned handle is always either valid or null.
         * @see @ref isHandleValid(LayouterHandle) const,
         *      @ref layouterNext(), @ref layouterFirst(), @ref layouterLast()
         */
        LayouterHandle layouterPrevious(LayouterHandle handle) const;

        /**
         * @brief Next layouter in the layout calculation order
         *
         * The next layouter gets executed later. Expects that @p handle is
         * valid. Returns @ref LayouterHandle::Null if the layouter is last.
         * The returned handle is always either valid or null.
         * @see @ref isHandleValid(LayouterHandle) const,
         *      @ref layouterPrevious(), @ref layouterLast(),
         *      @ref layouterFirst()
         */
        LayouterHandle layouterNext(LayouterHandle handle) const;

        /**
         * @brief Create a layouter
         * @param before    A layouter to order before for layout calculation
         *      or @ref LayouterHandle::Null if ordered as last. Expected to be
         *      valid if not null.
         * @return New layouter handle
         *
         * Allocates a new handle in a free slot in the internal storage or
         * grows the storage if there's no free slots left. Expects that
         * there's at most 256 layouters. The returned handle is meant to be
         * used to construct an @ref AbstractLayouter subclass and the instance
         * then passed to @ref setLayouterInstance(). A layouter can be removed
         * again with @ref removeLayer().
         * @see @ref isHandleValid(LayouterHandle) const,
         *      @ref layouterCapacity(), @ref layouterUsedCount()
         */
        LayouterHandle createLayouter(LayouterHandle before =
            #ifdef DOXYGEN_GENERATING_OUTPUT
            LayouterHandle::Null
            #else
            LayouterHandle{} /* To not have to include Handle.h */
            #endif
        );

        /**
         * @brief Set a layouter instance
         * @return Reference to @p instance
         *
         * Expects that @p instance was created with a @ref LayouterHandle
         * returned from @ref createLayouter() earlier, the handle is valid and
         * @ref setLayouterInstance() wasn't called for the same handle yet.
         *
         * Calls @ref AbstractLayouter::setSize() on the layouter, unless
         * neither @ref setSize() nor @ref AbstractUserInterface(const Vector2&, const Vector2&, const Vector2i&)
         * was called yet.
         * @see @ref AbstractLayouter::handle(),
         *      @ref isHandleValid(LayouterHandle) const
         */
        AbstractLayouter& setLayouterInstance(Containers::Pointer<AbstractLayouter>&& instance);
        /** @overload */
        template<class T> T& setLayouterInstance(Containers::Pointer<T>&& instance) {
            return static_cast<T&>(setLayouterInstance(Containers::Pointer<AbstractLayouter>{Utility::move(instance)}));
        }

        /**
         * @brief Layouter instance
         *
         * Expects that @p handle is valid and that @ref setLayouterInstance()
         * was called for it.
         * @see @ref isHandleValid(LayouterHandle) const
         */
        AbstractLayouter& layouter(LayouterHandle handle);
        const AbstractLayouter& layouter(LayouterHandle handle) const; /**< @overload */

        /**
         * @brief Layouter instance in a concrete type
         *
         * Expects that @p handle is valid and that @ref setLayouterInstance()
         * was called for it. It's the user responsibility to ensure that @p T
         * matches the actual instance type.
         * @see @ref isHandleValid(LayouterHandle) const
         */
        template<class T> T& layouter(LayouterHandle handle) {
            return static_cast<T&>(layouter(handle));
        }
        /** @overload */
        template<class T> const T& layouter(LayouterHandle handle) const {
            return static_cast<const T&>(layouter(handle));
        }

        /**
         * @brief Remove a layouter
         *
         * Expects that @p handle is valid. After this call,
         * @ref isHandleValid(LayouterHandle) const returns @cpp false @ce for
         * @p handle and @ref isHandleValid(LayoutHandle) const returns
         * @cpp false @ce for all layouts associated with @p handle.
         *
         * Calling this function causes
         * @ref UserInterfaceState::NeedsLayoutAssignmentUpdate to be set.
         * @see @ref clean()
         */
        void removeLayouter(LayouterHandle handle);

        /**
         * @}
         */

        /** @{
         * @name Animator management
         */

        /**
         * @brief Capacity of the animator storage
         *
         * Can be at most 256. If @ref createAnimator() is called and there's
         * no free slots left, the internal storage gets grown.
         * @see @ref animatorUsedCount()
         */
        std::size_t animatorCapacity() const;

        /**
         * @brief Count of used items in the animator storage
         *
         * Always at most @ref animatorCapacity(). Expired handles are counted
         * among used as well. The operation is done with a
         * @f$ \mathcal{O}(n) @f$ complexity where @f$ n @f$ is
         * @ref animatorCapacity().
         */
        std::size_t animatorUsedCount() const;

        /**
         * @brief Whether an animator handle is valid
         *
         * A handle is valid if it has been returned from @ref createAnimator()
         * before and @ref removeAnimator() wasn't called on it yet. Note that
         * a handle is valid even if the animator instance wasn't set with
         * @ref setGenericAnimatorInstance(), @ref setNodeAnimatorInstance(),
         * @ref setDataAnimatorInstance() or @ref setStyleAnimatorInstance()
         * yet. For @ref AnimatorHandle::Null always returns @cpp false @ce.
         */
        bool isHandleValid(AnimatorHandle handle) const;

        /**
         * @brief Whether an animation handle is valid
         *
         * A shorthand for extracting an @ref AnimatorHandle from @p handle
         * using @ref animationHandleAnimator(), calling
         * @ref isHandleValid(AnimatorHandle) const on it, if it's valid and
         * set then retrieving the particular animator instance using
         * @ref animator() and then calling
         * @ref AbstractAnimator::isHandleValid(AnimatorDataHandle) const with
         * an @ref AnimatorDataHandle extracted from @p handle using
         * @ref animationHandleData(). See these functions for more
         * information. For @ref AnimationHandle::Null,
         * @ref AnimatorHandle::Null or @ref AnimatorDataHandle::Null always
         * returns @cpp false @ce.
         */
        bool isHandleValid(AnimationHandle handle) const;

        /**
         * @brief Create an animator
         * @return New animator handle
         *
         * Allocates a new handle in a free slot in the internal storage or
         * grows the storage if there's no free slots left. Expects that
         * there's at most 256 animators. The returned handle is meant to be
         * used to construct an @ref AbstractAnimator subclass and the instance
         * then passed to @ref setGenericAnimatorInstance(),
         * @ref setNodeAnimatorInstance(), @ref setDataAnimatorInstance() or
         * @ref setStyleAnimatorInstance(). An animator can be removed again
         * with @ref removeAnimator().
         * @see @ref isHandleValid(AnimatorHandle) const,
         *      @ref animatorCapacity(), @ref animatorUsedCount()
         */
        AnimatorHandle createAnimator();

        /**
         * @brief Set a generic animator instance
         * @return Reference to @p instance
         *
         * Expects that @p instance was created with an @ref AnimatorHandle
         * returned from @ref createAnimator() earlier, the handle is valid and
         * none of @ref setGenericAnimatorInstance(),
         * @ref setNodeAnimatorInstance(), @ref setDataAnimatorInstance() or
         * @ref setStyleAnimatorInstance() was called for the same handle yet.
         * Additionally, if @ref AnimatorFeature::DataAttachment is supported
         * by @p instance, expects that
         * @ref AbstractGenericAnimator::setLayer() has already been called on
         * it.
         *
         * Internally, the instance is inserted into a list partitioned by
         * animator type, which is done with a @f$ \mathcal{O}(n) @f$
         * complexity where @f$ n @f$ is @ref animatorCapacity().
         * @see @ref AbstractAnimator::handle(),
         *      @ref isHandleValid(AnimatorHandle) const
         */
        AbstractGenericAnimator& setGenericAnimatorInstance(Containers::Pointer<AbstractGenericAnimator>&& instance);
        /** @overload */
        template<class T> T& setGenericAnimatorInstance(Containers::Pointer<T>&& instance) {
            return static_cast<T&>(setGenericAnimatorInstance(Containers::Pointer<AbstractGenericAnimator>{Utility::move(instance)}));
        }

        /**
         * @brief Set a node animator instance
         * @return Reference to @p instance
         *
         * Expects that @p instance was created with an @ref AnimatorHandle
         * returned from @ref createAnimator() earlier, the handle is valid and
         * none of @ref setGenericAnimatorInstance(),
         * @ref setNodeAnimatorInstance(), @ref setDataAnimatorInstance() or
         * @ref setStyleAnimatorInstance() was called for the same handle yet.
         * The @ref AbstractNodeAnimator is expected to advertise
         * @ref AnimatorFeature::NodeAttachment.
         *
         * Internally, the instance is inserted into a list partitioned by
         * animator type, which is done with a @f$ \mathcal{O}(n) @f$
         * complexity where @f$ n @f$ is @ref animatorCapacity().
         * @see @ref AbstractAnimator::handle(),
         *      @ref isHandleValid(AnimatorHandle) const
         */
        AbstractNodeAnimator& setNodeAnimatorInstance(Containers::Pointer<AbstractNodeAnimator>&& instance);
        /** @overload */
        template<class T> T& setNodeAnimatorInstance(Containers::Pointer<T>&& instance) {
            return static_cast<T&>(setNodeAnimatorInstance(Containers::Pointer<AbstractNodeAnimator>{Utility::move(instance)}));
        }

        /**
         * @brief Set a data animator instance
         * @return Reference to @p instance
         *
         * Expects that @p instance was created with an @ref AnimatorHandle
         * returned from @ref createAnimator() earlier, the handle is valid and
         * none of @ref setGenericAnimatorInstance(),
         * @ref setNodeAnimatorInstance(), @ref setDataAnimatorInstance() or
         * @ref setStyleAnimatorInstance() was called for the same handle yet.
         * The @ref AbstractDataAnimator is expected to advertise
         * @ref AnimatorFeature::DataAttachment and it's expected that
         * @ref AbstractLayer::assignAnimator(AbstractDataAnimator&) const has
         * already been called for it.
         *
         * Internally, the instance is inserted into a list partitioned by
         * animator type, which is done with a @f$ \mathcal{O}(n) @f$
         * complexity where @f$ n @f$ is @ref animatorCapacity().
         * @see @ref AbstractAnimator::handle(),
         *      @ref isHandleValid(AnimatorHandle) const
         */
        AbstractDataAnimator& setDataAnimatorInstance(Containers::Pointer<AbstractDataAnimator>&& instance);
        /** @overload */
        template<class T> T& setDataAnimatorInstance(Containers::Pointer<T>&& instance) {
            return static_cast<T&>(setDataAnimatorInstance(Containers::Pointer<AbstractDataAnimator>{Utility::move(instance)}));
        }

        /**
         * @brief Set a style animator instance
         * @return Reference to @p instance
         *
         * Expects that @p instance was created with an @ref AnimatorHandle
         * returned from @ref createAnimator() earlier, the handle is valid and
         * none of @ref setGenericAnimatorInstance(),
         * @ref setNodeAnimatorInstance(), @ref setDataAnimatorInstance() or
         * @ref setStyleAnimatorInstance() was called for the same handle yet.
         * The @ref AbstractStyleAnimator is expected to advertise
         * @ref AnimatorFeature::DataAttachment and it's expected that
         * @ref AbstractLayer::assignAnimator(AbstractStyleAnimator&) const has
         * already been called for it.
         *
         * Internally, the instance is inserted into a list partitioned by
         * animator type, which is done with a @f$ \mathcal{O}(n) @f$
         * complexity where @f$ n @f$ is @ref animatorCapacity().
         * @see @ref AbstractAnimator::handle(),
         *      @ref isHandleValid(AnimatorHandle) const
         */
        AbstractStyleAnimator& setStyleAnimatorInstance(Containers::Pointer<AbstractStyleAnimator>&& instance);
        /** @overload */
        template<class T> T& setStyleAnimatorInstance(Containers::Pointer<T>&& instance) {
            return static_cast<T&>(setStyleAnimatorInstance(Containers::Pointer<AbstractStyleAnimator>{Utility::move(instance)}));
        }

        /**
         * @brief Animator instance
         *
         * Expects that @p handle is valid and that one of
         * @ref setGenericAnimatorInstance(), @ref setNodeAnimatorInstance(),
         * @ref setDataAnimatorInstance() or @ref setStyleAnimatorInstance()
         * was called for it.
         * @see @ref isHandleValid(AnimatorHandle) const
         */
        AbstractAnimator& animator(AnimatorHandle handle);
        const AbstractAnimator& animator(AnimatorHandle handle) const; /**< @overload */

        /**
         * @brief Animator instance in a concrete type
         *
         * Expects that @p handle is valid and that one of
         * @ref setGenericAnimatorInstance(), @ref setNodeAnimatorInstance(),
         * @ref setDataAnimatorInstance() or @ref setStyleAnimatorInstance()
         * was called for it. It's the user responsibility to ensure that @p T
         * matches the actual instance type.
         * @see @ref isHandleValid(AnimatorHandle) const
         */
        template<class T> T& animator(AnimatorHandle handle) {
            return static_cast<T&>(animator(handle));
        }
        /** @overload */
        template<class T> const T& animator(AnimatorHandle handle) const {
            return static_cast<const T&>(animator(handle));
        }

        /**
         * @brief Remove an animator
         *
         * Expects that @p handle is valid. After this call,
         * @ref isHandleValid(AnimatorHandle) const returns @cpp false @ce for
         * @p handle and @ref isHandleValid(AnimationHandle) const returns
         * @cpp false @ce for all animations associated with @p handle.
         *
         * Internally, if the removed animator had an instance set, the
         * instance is removed from a list partitioned by animator type, which
         * is done with a @f$ \mathcal{O}(n) @f$ complexity where @f$ n @f$ is
         * @ref animatorCapacity().
         * @see @ref clean()
         */
        void removeAnimator(AnimatorHandle handle);

        /**
         * @brief Attach an animation to a node
         *
         * A shorthand for extracting a @ref AnimatorHandle from @p animation
         * using @ref animationHandleAnimator(), retrieving the particular
         * animator instance using @ref animator() and then calling
         * @ref AbstractAnimator::attach(AnimatorDataHandle, NodeHandle) with a
         * @ref AnimatorDataHandle extracted with @ref animationHandleData().
         * See these functions for more information. In addition to
         * @ref AbstractAnimator::attach(AnimatorDataHandle, NodeHandle), this
         * function checks that @p node is either valid or
         * @ref NodeHandle::Null.
         */
        void attachAnimation(NodeHandle node, AnimationHandle animation);

        /**
         * @brief Attach an animation to a data
         *
         * A shorthand for extracting a @ref AnimatorHandle from @p animation
         * using @ref animationHandleAnimator(), retrieving the particular
         * animator instance using @ref animator() and then calling
         * @ref AbstractAnimator::attach(AnimatorDataHandle, DataHandle) with a
         * @ref AnimatorDataHandle extracted with @ref animationHandleData().
         * See these functions for more information. In addition to
         * @ref AbstractAnimator::attach(AnimatorDataHandle, DataHandle), this
         * function checks that @p data is either valid with the layer portion
         * matching @ref AbstractAnimator::layer() of given animator or
         * @ref DataHandle::Null.
         *
         * Note that unlike @ref AbstractAnimator::attach(AnimationHandle, LayerDataHandle),
         * here's no convenience function that would take a
         * @ref LayerDataHandle as it wouldn't be able to provide any extra
         * checks over calling the @ref AbstractAnimator API directly.
         */
        void attachAnimation(DataHandle data, AnimationHandle animation);

        /**
         * @}
         */

        /** @{
         * @name Node management
         */

        /**
         * @brief Current capacity of the node storage
         *
         * Can be at most 1048576. If @ref createNode() is called and there's
         * no free slots left, the internal storage gets grown.
         * @see @ref nodeUsedCount()
         */
        std::size_t nodeCapacity() const;

        /**
         * @brief Count of used items in the node storage
         *
         * Always at most @ref nodeCapacity(). Expired handles are counted
         * among used as well. The operation is done with a
         * @f$ \mathcal{O}(n) @f$ complexity where @f$ n @f$ is
         * @ref nodeCapacity().
         */
        std::size_t nodeUsedCount() const;

        /**
         * @brief Whether a node handle is valid
         *
         * A handle is valid if it has been returned from @ref createNode()
         * before, @ref removeNode() wasn't called on it yet and it wasn't
         * removed inside @ref update() due to a parent node being removed
         * earlier. For @ref NodeHandle::Null always returns @cpp false @ce.
         */
        bool isHandleValid(NodeHandle handle) const;

        /**
         * @brief Create a node
         * @param parent    Parent node to attach to or
         *      @ref NodeHandle::Null for a new root node. Expected to be valid
         *      if not null.
         * @param offset    Offset relative to the parent node
         * @param size      Size of the node contents. Used for layouting,
         *      clipping and event handling.
         * @param flags     Initial node flags
         * @return New node handle
         *
         * Allocates a new handle in a free slot in the internal storage or
         * grows the storage if there's no free slots left. Expects that
         * there's at most 1048576 nodes. The returned handle can be then
         * removed again with @ref removeNode().
         *
         * If @p parent is @ref NodeHandle::Null, the node is added at the
         * back of the draw and event processing list, i.e. drawn last (thus
         * at the front) and reacting to events before all others. Use
         * @ref setNodeOrder() and @ref clearNodeOrder() to adjust the draw
         * and event processing order or remove it from the list of visible
         * top-level nodes.
         *
         * Calling this function causes @ref UserInterfaceState::NeedsNodeUpdate
         * to be set.
         * @see @ref isHandleValid(NodeHandle) const, @ref nodeCapacity(),
         *      @ref nodeUsedCount(), @ref update()
         */
        NodeHandle createNode(NodeHandle parent, const Vector2& offset, const Vector2& size, NodeFlags flags = {});

        /**
         * @brief Create a root node
         *
         * Equivalent to calling @ref createNode(NodeHandle, const Vector2&, const Vector2&, NodeFlags)
         * with @ref NodeHandle::Null as the parent.
         */
        NodeHandle createNode(const Vector2& offset, const Vector2& size, NodeFlags flags = {});

        /**
         * @brief Node parent
         *
         * Expects that @p handle is valid. Returns @ref NodeHandle::Null if
         * it's a root node. Note that the returned handle may be invalid if
         * @ref removeNode() was called on one of the parent nodes and
         * @ref update() hasn't been called since.
         *
         * Unlike other node properties, the parent cannot be changed after
         * creation.
         * @see @ref isHandleValid(NodeHandle) const
         */
        NodeHandle nodeParent(NodeHandle handle) const;

        /**
         * @brief Node offset relative to its parent
         *
         * The returned value is before any layout calculation is done. Expects
         * that @p handle is valid.
         * @see @ref isHandleValid(NodeHandle) const, @ref nodeParent()
         */
        Vector2 nodeOffset(NodeHandle handle) const;

        /**
         * @brief Set node offset relative to its parent
         *
         * If the node has a layout assigned, the value is subsequently used
         * for layout calculation and it's up to the particular layouter
         * implementation how the initial offset value is used. If the node
         * doesn't have a layout assigned, the offset is used as-is. Expects
         * that @p handle is valid. Initially, a node has the offset that was
         * passed to @ref createNode().
         *
         * Calling this function causes
         * @ref UserInterfaceState::NeedsLayoutUpdate to be set.
         * @see @ref isHandleValid(NodeHandle) const, @ref nodeParent()
         */
        void setNodeOffset(NodeHandle handle, const Vector2& offset);

        /**
         * @brief Node size
         *
         * The returned value is before any layout calculation is done. Expects
         * that @p handle is valid.
         * @see @ref isHandleValid(NodeHandle) const
         */
        Vector2 nodeSize(NodeHandle handle) const;

        /**
         * @brief Set node size
         *
         * If the node has a layout assigned, the value is subsequently used
         * for layout calculation and it's up to the particular layouter
         * implementation how the initial size value is used. If the node
         * doesn't have a layout assigned, the size is used as-is. Expects that
         * @p handle is valid. Initially, a node has the size that was passed
         * to @ref createNode().
         *
         * Calling this function causes
         * @ref UserInterfaceState::NeedsLayoutUpdate to be set.
         * @see @ref isHandleValid(NodeHandle) const
         */
        void setNodeSize(NodeHandle handle, const Vector2& size);

        /**
         * @brief Node opacity
         *
         * The returned value is only the opacity set on the node itself,
         * without considering opacity inherited from parents. Expects that
         * @p handle is valid.
         * @see @ref isHandleValid(NodeHandle) const
         */
        Float nodeOpacity(NodeHandle handle) const;

        /**
         * @brief Set node opacity
         *
         * The value is subsequently multiplied with opacity of all parents and
         * passed to layers to affect rendering. Opacity of @cpp 1.0f @ce makes
         * a node opaque and @cpp 0.0f @ce fully transparent, although values
         * outside of this range are allowed as well. Note that it's up to the
         * particular layer implementation how the opacity value is actually
         * used, and if at all --- the layer may for example have additional
         * options that affect the opacity either way. Expects that @p handle
         * is valid. Initially, a node has the opacity set to @cpp 1.0f @ce.
         *
         * Calling this function causes
         * @ref UserInterfaceState::NeedsNodeOpacityUpdate to be set.
         * @see @ref isHandleValid(NodeHandle) const
         */
        void setNodeOpacity(NodeHandle handle, Float opacity);

        /**
         * @brief Node flags
         *
         * The returned value is only flags set directly on the node itself,
         * without considering flags such as @ref NodeFlag::Disabled that would
         * be inherited from parents. Expects that @p handle is valid.
         * @see @ref isHandleValid(NodeHandle) const, @ref setNodeFlags(),
         *      @ref addNodeFlags(), @ref clearNodeFlags()
         */
        NodeFlags nodeFlags(NodeHandle handle) const;

        /**
         * @brief Set node flags
         *
         * Expects that @p handle is valid. Initially, a node has the flags
         * that were passed to @ref createNode(), which are by default none.
         *
         * If @ref NodeFlag::Hidden was added or cleared by calling this
         * function, it causes @ref UserInterfaceState::NeedsNodeUpdate to be
         * set. If @ref NodeFlag::Clip was added or cleared by calling this
         * function, it causes @ref UserInterfaceState::NeedsNodeClipUpdate to
         * be set. If @ref NodeFlag::NoEvents, @ref NodeFlag::Disabled or
         * @ref NodeFlag::Focusable was added or cleared by calling this
         * function, it causes @ref UserInterfaceState::NeedsNodeEnabledUpdate
         * to be set.
         * @see @ref isHandleValid(NodeHandle) const, @ref addNodeFlags(),
         *      @ref clearNodeFlags()
         */
        void setNodeFlags(NodeHandle handle, NodeFlags flags);

        /**
         * @brief Add node flags
         *
         * Calls @ref setNodeFlags() with the existing flags ORed with
         * @p flags. Useful for preserving previously set flags.
         * @see @ref clearNodeFlags()
         */
        void addNodeFlags(NodeHandle handle, NodeFlags flags);

        /**
         * @brief Clear node flags
         *
         * Calls @ref setNodeFlags() with the existing flags ANDed with the
         * inverse of @p flags. Useful for removing a subset of previously set
         * flags.
         * @see @ref addNodeFlags()
         */
        void clearNodeFlags(NodeHandle handle, NodeFlags flags);

        /**
         * @brief Remove a node
         *
         * Expects that @p handle is valid. Nested nodes and data attached to
         * any of the nodes are then removed during the next call to
         * @ref update(). After this call, @ref isHandleValid(NodeHandle) const
         * returns @cpp false @ce for @p handle.
         *
         * If @p handle is a top-level node, the operation is done with an
         * @f$ \mathcal{O}(n) @f$ complexity, where @f$ n @f$ is the count of
         * nested top-level hierarchies.
         *
         * Calling this function causes @ref UserInterfaceState::NeedsNodeClean
         * to be set.
         * @see @ref clean(), @ref nodeOrderLastNested()
         */
        void removeNode(NodeHandle handle);

        /**
         * @}
         */

        /** @{
         * @name Top-level node draw and event processing order management
         */

        /**
         * @brief Capacity of the top-level node order storage
         *
         * @ref nodeOrderUsedCount()
         */
        std::size_t nodeOrderCapacity() const;

        /**
         * @brief Count of used items in the top-level node order storage
         *
         * Always at most @ref nodeOrderCapacity(). The operation is done
         * with a @f$ \mathcal{O}(n) @f$ complexity where @f$ n @f$ is
         * @ref nodeOrderCapacity(). When a root node is created or
         * @ref setNodeOrder() is called for the first time on a non-root node,
         * a slot in the node order storage is used for it, and gets recycled
         * only when the node is removed again or when @ref flattenNodeOrder()
         * is called on a non-root node.
         * @see @ref isNodeTopLevel()
         */
        std::size_t nodeOrderUsedCount() const;

        /**
         * @brief First top-level node in draw and event processing order
         *
         * The first node gets drawn first (thus is at the back) and reacts
         * to events after all others. Returns @ref NodeHandle::Null if
         * there's no nodes included in the draw and event processing order.
         * The returned handle is always either valid or null.
         * @see @ref nodeOrderNext(), @ref nodeOrderLast(),
         *      @ref nodeOrderPrevious(), @ref nodeOrderLastNested(),
         *      @ref isNodeTopLevel(), @ref isNodeOrdered()
         */
        NodeHandle nodeOrderFirst() const;

        /**
         * @brief Last top-level node in draw and event processing order
         *
         * The last node gets drawn last (thus is at the front) and reacts to
         * events before all others. Returns @ref NodeHandle::Null if there's
         * no nodes included in the draw and event processing order. The
         * returned handle is always either valid or null.
         * @see @ref nodeOrderPrevious(), @ref nodeOrderFirst(),
         *      @ref nodeOrderNext(), @ref nodeOrderLastNested(),
         *      @ref isNodeOrdered()
         */
        NodeHandle nodeOrderLast() const;

        /**
         * @brief Whether a node is top-level for draw and event processing
         *
         * If not top-level, it's in the usual order defined by the node
         * hierarchy. If top-level, it's ordered respective to other top-level
         * nodes. Expects that @p handle is valid.
         *
         * Always returns @cpp true @ce for root nodes. If @ref isNodeOrdered()
         * returns @cpp true @ce, it implies the node is top-level. If this
         * function returns @cpp true @ce, it doesn't necessarily mean the node
         * is visible --- the node or any of its parents could be excluded from
         * the order or it could be hidden.
         * @see @ref isHandleValid(NodeHandle) const, @ref nodeOrderNext(),
         *      @ref setNodeOrder(), @ref clearNodeOrder(),
         *      @ref flattenNodeOrder()
         */
        bool isNodeTopLevel(NodeHandle handle) const;

        /**
         * @brief Whether a node is top-level and is included in a draw and event processing order
         *
         * If not included, the node and all its children are not drawn and
         * don't react to events. Expects that @p handle is valid.
         *
         * If this function returns @cpp true @ce, @ref isNodeTopLevel() is
         * @cpp true @ce as well. For non-root nodes the function returning
         * @cpp true @ce means the node is included in the order respective to
         * its parent top-level node. It doesn't necessarily mean the node is
         * visible --- the parents can themselves be excluded from the order or
         * they could be hidden.
         * @see @ref isHandleValid(NodeHandle) const, @ref nodeOrderNext(),
         *      @ref setNodeOrder(), @ref clearNodeOrder(),
         *      @ref flattenNodeOrder()
         */
        bool isNodeOrdered(NodeHandle handle) const;

        /**
         * @brief Previous node in draw and event processing order
         *
         * The previous node gets drawn earlier (thus is behind) and reacts
         * to events later. Expects that @p handle is valid. Returns
         * @ref NodeHandle::Null if the node is first in the draw and
         * processing order, is a root node that's not included in the draw and
         * event processing order, or is a non-root node that's drawn in a
         * regular hierarchy-defined order. The returned handle is always
         * either valid or null.
         * @see @ref isHandleValid(NodeHandle) const, @ref nodeOrderNext(),
         *      @ref nodeOrderFirst(), @ref nodeOrderLast(),
         *      @ref isNodeOrdered()
         */
        NodeHandle nodeOrderPrevious(NodeHandle handle) const;

        /**
         * @brief Next node in draw and event processing order
         *
         * The next node gets drawn later (thus is in front) and reacts to
         * events earlier. Expects that @p handle is valid. Returns
         * @ref NodeHandle::Null if the node is last in the draw and processing
         * order, is a root node that's not included in the draw and event
         * processing order, or is a non-root node that's drawn in a regular
         * hierarchy-defined order. The returned handle is always either valid
         * or null.
         * @see @ref isHandleValid(NodeHandle) const, @ref nodeOrderPrevious(),
         *      @ref nodeOrderLast(), @ref nodeOrderLastNested(),
         *      @ref nodeOrderFirst(), @ref isNodeOrdered()
         */
        NodeHandle nodeOrderNext(NodeHandle handle) const;

        /**
         * @brief Last node in draw and event processing order nested under this node
         *
         * Expects that @p handle is valid. If any children are top-level nodes
         * as well, points to the last of them, including any nested
         * top-level hierarchies. If there are no child top-level hierarchies,
         * returns @p handle itself.
         *
         * The order of child top-level hierarchies relative to the
         * @p handle gets preserved when modifying the top-level order of
         * @p handle using @ref setNodeOrder() or @ref clearNodeOrder().
         * @see @ref isHandleValid(NodeHandle) const
         */
        NodeHandle nodeOrderLastNested(NodeHandle handle) const;

        /**
         * @brief Order a top-level node for draw and event processing
         *
         * The @p handle gets ordered to be drawn earlier (thus behind) and
         * react to event later than @p before. Expects that @p handle is
         * valid and @p before is either @ref NodeHandle::Null or a valid node
         * that's included in the draw and event processing order.
         *
         * If @p handle is a root node, expects that @p before is also a root
         * node, if not null. If null, the @p handle is ordered as drawn last.
         * The operation is done with an @f$ \mathcal{O}(1) @f$ complexity in
         * this case.
         *
         * If @p handle is not a root node, expects that both
         * @p handle and @p before have a common parent that's the closest
         * top-level or root node for both, if @p before is not null. If null,
         * the @p handle is ordered after all other top-level nodes under its
         * closest top-level or root parent. @ref NodeFlag::Hidden set on any
         * parent node affects the top-level node, @ref NodeFlag::Clip,
         * @relativeref{NodeFlag,NoEvents} or @relativeref{NodeFlag,Disabled}
         * doesn't. The operation is done with an @f$ \mathcal{O}(n) @f$
         * complexity in this case, where @f$ n @f$ is the depth at which
         * @p handle is in the node hierarchy.
         *
         * If the node was previously in a different position in the draw and
         * event processing order, it's moved, if it wasn't previously a
         * top-level node or if it wasn't included in the draw and event
         * processing order, it's inserted. Use @ref clearNodeOrder() to
         * exclude the node from the order afterwards and
         * @ref flattenNodeOrder() to integrate a non-root node back into the
         * usual order defined by the node hierarchy.
         *
         * @m_class{m-note m-warning}
         *
         * @par
         *      At the moment, if a non-root node isn't top-level yet, it can
         *      be made only if it doesn't already contain a nested top-level
         *      order. If it does, call @ref clearNodeOrder() on the nested
         *      top-level nodes first (or @ref flattenNodeOrder() them), after
         *      that you can order them back.
         *
         * Calling this function causes @ref UserInterfaceState::NeedsNodeUpdate
         * to be set.
         * @see @ref isHandleValid(NodeHandle) const, @ref isNodeTopLevel(),
         *      @ref nodeOrderNext(), @ref nodeOrderLastNested(), @ref update()
         */
        void setNodeOrder(NodeHandle handle, NodeHandle before);

        /**
         * @brief Clear a node from the draw and event processing order
         *
         * Expects that @p handle is valid. If the node isn't top-level or
         * wasn't previously in the draw and processing order, the function is
         * a no-op. After calling this function, @ref isNodeOrdered() returns
         * @cpp false @ce for @p handle.
         *
         * If @p handle is a root node, the operation is done with an
         * @f$ \mathcal{O}(1) @f$ complexity. If @p handle is not a root node,
         * the operation is done with an @f$ \mathcal{O}(n) @f$ complexity,
         * where @f$ n @f$ is the depth at which @p handle is in the node
         * hierarchy.
         *
         * If the node contains any child top-level hierarchies, their order
         * relative to the @p handle gets preserved, i.e. they get inserted
         * back alongside it next time @ref setNodeOrder() is called. Use
         * @ref flattenNodeOrder() to integrate a non-root node back into the
         * usual order defined by the node hierarchy.
         *
         * If not a no-op, calling this function causes
         * @ref UserInterfaceState::NeedsNodeUpdate to be set.
         * @see @ref isHandleValid(NodeHandle) const,
         *      @ref nodeOrderLastNested(), @ref update()
         */
        void clearNodeOrder(NodeHandle handle);

        /**
         * @brief Flatten a non-root top-level node back to the usual order defined by the node hierarchy
         *
         * Expects that @p handle is valid and isn't a root node. Undoes the
         * operation done by calling @ref setNodeOrder() on a non-root node,
         * i.e. the node is again drawn alongside its neighbors. If the node
         * isn't top-level, the function is a no-op. After calling this
         * function, @ref isNodeTopLevel() returns @cpp false @ce for
         * @p handle.
         *
         * The operation is done with an @f$ \mathcal{O}(n) @f$ complexity,
         * where @f$ n @f$ is the depth at which @p handle is in the node
         * hierarchy.
         *
         * If the node contains nested top-level nodes, they stay top-level.
         * Use @ref clearNodeOrder() if you want to exclude a top-level node
         * including all its children from the draw and event processing order.
         *
         * If not a no-op, calling this function causes
         * @ref UserInterfaceState::NeedsNodeUpdate to be set.
         * @see @ref isHandleValid(NodeHandle) const, @ref update()
         */
        void flattenNodeOrder(NodeHandle handle);

        /**
         * @}
         */

        /**
         * @brief Clean orphaned nodes, data and no longer valid data attachments
         * @return Reference to self (for method chaining)
         *
         * Called implicitly from @ref update() and subsequently also from
         * @ref draw() and all event processing functions. If @ref state()
         * contains neither @ref UserInterfaceState::NeedsNodeClean nor
         * @ref UserInterfaceState::NeedsDataClean, this function is a no-op,
         * otherwise it performs a subset of the following depending on the
         * state:
         *
         * -    Removes nodes with an invalid (removed) parent node
         * -    Calls @ref AbstractLayer::cleanNodes() with updated node
         *      generations, causing removal of data attached to invalid nodes
         * -    Calls @ref AbstractLayouter::cleanNodes() with updated node
         *      generations, causing removal of layouts assigned to invalid
         *      nodes
         * -    Calls @ref AbstractAnimator::cleanNodes() on all animators
         *      supporting @ref AnimatorFeature::NodeAttachment with updated
         *      node generations, causing removal of animations attached to
         *      invalid nodes
         * -    For all layers marked with @ref LayerState::NeedsDataClean
         *      calls @ref AbstractLayer::cleanData() with all animators
         *      supporting @ref AnimatorFeature::DataAttachment associated with
         *      given layer, causing removal of animations attached to invalid
         *      data
         *
         * After calling this function, @ref state() doesn't contain
         * @ref UserInterfaceState::NeedsNodeClean anymore;
         * @ref nodeUsedCount() and @ref AbstractLayer::usedCount() may get
         * smaller.
         */
        AbstractUserInterface& clean();

        /**
         * @brief Advance active animations
         * @return Reference to self (for method chaining)
         *
         * Implicitly calls @ref clean(), should be called before any
         * @ref update() or @ref draw() for given frame. Expects that @p time
         * is greater or equal to @ref animationTime(). If @ref state()
         * contains @ref UserInterfaceState::NeedsAnimationAdvance, this
         * function delegates to @ref AbstractAnimator::update() and then a
         * corresponding animator-specific advance function such as
         * @ref AbstractGenericAnimator::advance(),
         * @ref AbstractNodeAnimator::advance() or layer-specific
         * @ref AbstractLayer::advanceAnimations() on all animator instances
         * that have @ref AnimatorState::NeedsAdvance set.
         *
         * Calling this function updates @ref animationTime(). Afterwards,
         * @ref state() may still contain
         * @ref UserInterfaceState::NeedsAnimationAdvance, signalling that
         * animation advance is still needed the next fram. It may also have
         * other states added depending on what all the animators touched, and
         * a subsequent call to @ref draw() (or directly to @ref clean() and
         * @ref update() before that) for given frame will take care of
         * correctly updating the internal state.
         */
        AbstractUserInterface& advanceAnimations(Nanoseconds time);

        /**
         * @brief Update node hierarchy, data order and data contents for drawing and event processing
         * @return Reference to self (for method chaining)
         *
         * Expects that either @ref setSize() was called or the
         * @ref AbstractUserInterface(const Vector2&, const Vector2&, const Vector2i&)
         * constructor was used.
         *
         * Implicitly calls @ref clean(); called implicitly from @ref draw()
         * and all event processing functions. If @ref state() contains none of
         * @ref UserInterfaceState::NeedsDataUpdate,
         * @ref UserInterfaceState::NeedsDataAttachmentUpdate,
         * @ref UserInterfaceState::NeedsNodeEnabledUpdate,
         * @ref UserInterfaceState::NeedsNodeClipUpdate,
         * @ref UserInterfaceState::NeedsLayoutUpdate,
         * @ref UserInterfaceState::NeedsLayoutAssignmentUpdate or
         * @ref UserInterfaceState::NeedsNodeUpdate, this function is a no-op,
         * otherwise it performs a subset of the following depending on the
         * state, in order:
         *
         * -    Orders visible nodes back-to-front for drawing and
         *      front-to-back for event processing
         * -    Orders layouts assigned to nodes by their dependency
         * -    Performs layout calculation
         * -    Calculates absolute offsets for visible nodes
         * -    Culls invisible nodes, calculates clip rectangles
         * -    Propagates @ref NodeFlag::Disabled and @ref NodeFlag::NoEvents
         *      to child nodes
         * -    Orders data attachments in each layer by draw order
         * -    Resets @ref currentPressedNode(), @ref currentCapturedNode(),
         *      @ref currentHoveredNode() or @ref currentFocusedNode() if they
         *      no longer exist
         * -    Calls @ref AbstractLayer::visibilityLostEvent() and resets
         *      @ref currentPressedNode(), @ref currentCapturedNode(),
         *      @ref currentHoveredNode() or @ref currentFocusedNode() if they
         *      are not visible or have @ref NodeFlag::NoEvents or
         *      @ref NodeFlag::Disabled set on them or their parents, or if the
         *      currently focused node is no longer @ref NodeFlag::Focusable.
         * -    Goes in a back to front order through layers that have
         *      instances set and calls @ref AbstractLayer::update() with the
         *      ordered data
         *
         * After calling this function, @ref state() is empty apart from
         * @ref UserInterfaceState::NeedsAnimationAdvance, which may be present
         * if there are any animators for which @ref advanceAnimations() should
         * be called.
         */
        AbstractUserInterface& update();

        /**
         * @brief Draw the user interface
         * @return Reference to self (for method chaining)
         *
         * Implicitly calls @ref update(), which in turn implicitly calls
         * @ref clean(). Performs the following:
         *
         * -    Calls @ref AbstractRenderer::transition() with
         *      @ref RendererTargetState::Initial
         * -    Peforms draw calls by going through draws collected by
         *      @ref update() for each top level node and all its visible
         *      children in a back to front order, and then for each layer that
         *      supports @ref LayerFeature::Draw in a back to front order:
         *      -   If a layer advertises @ref LayerFeature::Composite, calls
         *          @ref AbstractRenderer::transition() with
         *          @ref RendererTargetState::Composite, and then
         *          @ref AbstractLayer::composite()
         *      -   Calls @ref AbstractRenderer::transition() with
         *          @ref RendererTargetState::Draw and appropriate
         *          @ref RendererDrawStates based on whether given layer
         *          advertises @ref LayerFeature::DrawUsesBlending or
         *          @relativeref{LayerFeature,DrawUsesScissor}
         *      -   Calls @ref AbstractLayer::draw()
         * -    Calls @ref AbstractRenderer::transition() with
         *      @ref RendererTargetState::Final
         */
        AbstractUserInterface& draw();

        /**
         * @brief Handle a pointer press event
         *
         * Implicitly calls @ref update(), which in turn implicitly calls
         * @ref clean(). The @p globalPosition is assumed to be in respect to
         * @ref windowSize(), and is internally scaled to match @ref size()
         * before being set to @ref PointerEvent.
         *
         * If the event is not primary and a node was captured by a previous
         * @ref pointerPressEvent(), @ref pointerMoveEvent() or a non-primary
         * @ref pointerReleaseEvent(), calls @ref pointerPressEvent() on all
         * data attached to that node even if the event happens outside of its
         * area, with the event position made relative to the node.
         *
         * Otherwise, if either the event is primary or no node is captured,
         * finds the front-most node under (scaled) @p globalPosition and
         * calls @ref AbstractLayer::pointerPressEvent() on all data attached
         * to it belonging to layers that support @ref LayerFeature::Event. If
         * no data accept the event, continues to other nodes under the
         * position in a front-to-back order and then to parent nodes. For each
         * such node, the event is always called on all attached data,
         * regardless of the accept status. For each call the event position is
         * made relative to the node to which given data is attached.
         *
         * If the press happened with @ref Pointer::MouseLeft, primary
         * @ref Pointer::Finger or @ref Pointer::Pen, the currently focused
         * node is affected as well. If the node the event was accepted on is
         * different from currently focused node (if there's any),
         * @ref AbstractLayer::blurEvent() is called on all data attached to
         * that node. Then, if the node has @ref NodeFlag::Focusable set,
         * @ref AbstractLayer::focusEvent() is called on all data attached to
         * it. If any data accept the focus event, the node is treated as
         * focused and receives all following key events until a primary
         * @ref pointerPressEvent() outside of this node happens or until a
         * different node is made focused using @ref focusEvent(). If the node
         * is not @ref NodeFlag::Focusable or the focus event was not accepted
         * by any data, the currently focused node is reset. If a primary press
         * event happens on a @ref NodeFlag::Focusable node that's already
         * focused, @ref AbstractLayer::focusEvent() gets called on it again,
         * without any preceding @ref AbstractLayer::blurEvent(). If that focus
         * event is not accepted however, the node subsequently receives an
         * @ref AbstractLayer::blurEvent(). If the event isn't primary, the
         * current focused node isn't affected in any way.
         *
         * Returns @cpp true @ce if the press event was accepted by at least
         * one data, @cpp false @ce if it wasn't or there wasn't any visible
         * event handling node at given position and thus the event should be
         * propagated further. Accept status of the focus and blur events
         * doesn't have any effect on the return value.
         *
         * If the event is primary, the node that accepted the event is
         * remembered and is subsequently used by @ref pointerReleaseEvent() to
         * emit a @ref AbstractLayer::pointerTapOrClickEvent(), if a primary
         * release happens on it as well. The node that accepted the primary
         * event also implicitly captures all further pointer events until and
         * including a primary @ref pointerReleaseEvent() even if they happen
         * outside of its area, unless @ref PointerEvent::setCaptured() is
         * called by the implementation to disable this behavior. The capture
         * can be also removed from a later @ref pointerMoveEvent() or a
         * non-primary @ref pointerReleaseEvent(). Any node that was already
         * captured when calling this function with a primary event is ignored.
         * If the event isn't primary, the node isn't rememebered for any
         * subsequent tap or click event and neither it captures any further
         * events.
         *
         * If no node accepted the event or there wasn't any visible event
         * handling node at given position, the previously remembered pressed
         * and captured nodes are reset if and only if the event is primary.
         *
         * Expects that the event is not accepted yet.
         * @see @ref PointerEvent::isPrimary(), @ref PointerEvent::isAccepted(),
         *      @ref PointerEvent::setAccepted(), @ref currentPressedNode(),
         *      @ref currentCapturedNode(), @ref currentFocusedNode(),
         *      @ref currentGlobalPointerPosition()
         */
        bool pointerPressEvent(const Vector2& globalPosition, PointerEvent& event);

        /**
         * @brief Handle an external pointer press event
         *
         * Converts the @p event to a @ref PointerEvent and delegates to
         * @ref pointerPressEvent(), see its documentation and
         * @ref Ui-AbstractUserInterface-application for more information.
         * The @p args allow passing optional extra arguments to a particular
         * event converter.
         */
        template<class Event, class ...Args, class = decltype(Implementation::PointerEventConverter<Event>::press(std::declval<AbstractUserInterface&>(), std::declval<Event&>(), std::declval<Args>()...))> bool pointerPressEvent(Event& event, Args&&... args) {
            return Implementation::PointerEventConverter<Event>::press(*this, event, Utility::forward<Args>(args)...);
        }

        /**
         * @brief Handle a pointer release event
         *
         * Implicitly calls @ref update(), which in turn implicitly calls
         * @ref clean(). The @p globalPosition is assumed to be in respect to
         * @ref windowSize(), and is internally scaled to match @ref size()
         * before being set to @ref PointerEvent.
         *
         * If a node was captured by a previous @ref pointerPressEvent(),
         * @ref pointerMoveEvent() or a non-primary @ref pointerReleaseEvent(),
         * calls @ref AbstractLayer::pointerReleaseEvent() on all data attached
         * to that node even if the event happens outside of its area, with the
         * event position made relative to the node.
         *
         * Otherwise, if a node wasn't captured, finds the front-most node
         * under (scaled) @p globalPosition and calls
         * @ref AbstractLayer::pointerReleaseEvent() on all data attached to it
         * belonging to layers that support @ref LayerFeature::Event. If no
         * data accept the event, continues to other nodes under the position
         * in a front-to-back order and then to parent nodes. For each such
         * node, the event is always called on all attached data, regardless of
         * the accept status. For each call the event position is made relative
         * to the node to which given data is attached.
         *
         * If the event is primary and a node is captured, the capture is
         * implicitly released after calling this function independently of
         * whether @ref PointerEvent::setCaptured() was set by the
         * implementation. If the event isn't primary, the current captured
         * node isn't affected by default, but the implementation can both
         * release the existing capture or make the containing node capture
         * future events by setting @ref PointerEvent::setCaptured().
         *
         * Returns @cpp true @ce if the event was accepted by at least one
         * data, @cpp false @ce if it wasn't or there wasn't any visible event
         * handling node at given position and thus the event should be
         * propagated further.
         *
         * If the node that accepted the event is the same as the node on which
         * a previous @ref pointerPressEvent() happened, the event is primary
         * and the pointer was either captured or didn't leave the node area
         * since, calls also @ref AbstractLayer::pointerTapOrClickEvent() on
         * all data attached to it.
         *
         * Expects that the event is not accepted yet.
         * @see @ref PointerEvent::isPrimary(), @ref PointerEvent::isAccepted(),
         *      @ref PointerEvent::setAccepted(), @ref currentPressedNode(),
         *      @ref currentCapturedNode(), @ref currentGlobalPointerPosition()
         */
        bool pointerReleaseEvent(const Vector2& globalPosition, PointerEvent& event);

        /**
         * @brief Handle an external pointer release event
         *
         * Converts the @p event to a @ref PointerEvent and delegates to
         * @ref pointerReleaseEvent(), see its documentation and
         * @ref Ui-AbstractUserInterface-application for more information.
         * The @p args allow passing optional extra arguments to a particular
         * event converter.
         */
        template<class Event, class ...Args, class = decltype(Implementation::PointerEventConverter<Event>::release(std::declval<AbstractUserInterface&>(), std::declval<Event&>(), std::declval<Args>()...))> bool pointerReleaseEvent(Event& event, Args&&... args) {
            return Implementation::PointerEventConverter<Event>::release(*this, event, Utility::forward<Args>(args)...);
        }

        /**
         * @brief Handle a pointer move event
         *
         * Implicitly calls @ref update(), which in turn implicitly calls
         * @ref clean(). The @p globalPosition is assumed to be in respect to
         * @ref windowSize(), and is internally scaled to match @ref size()
         * before being set to @ref PointerEvent.
         *
         * If a node was captured by a previous @ref pointerPressEvent() or
         * @ref pointerMoveEvent(), @ref pointerReleaseEvent() wasn't called
         * yet and the node wasn't removed since, calls
         * @ref AbstractLayer::pointerMoveEvent() on all data attached to that
         * node even if it happens outside of its area, with the event position
         * made relative to the node. If the move event is primary, happened
         * inside the node area and is accepted by at least one data, the node
         * is treated as hovered, otherwise as not hovered. An
         * @ref AbstractLayer::pointerEnterEvent() or
         * @relativeref{AbstractLayer,pointerLeaveEvent()} is then called for
         * all data attached to the captured node if the node hover status
         * changed with the same @p event except for
         * @ref PointerMoveEvent::relativePosition() which is reset to a zero
         * vector. No corresponding leave / enter event is called for any other
         * node in this case. If the move event isn't primary, the current
         * hovered node isn't affected in any way. Returns @cpp true @ce if the
         * move event was accepted by at least one data attached to the
         * captured node, @cpp false @ce if it wasn't and thus the event should
         * be propagated further; accept status of the enter and leave events
         * is ignored. If the move, enter or leave event implementations called
         * @ref PointerMoveEvent::setCaptured() resulting in it being
         * @cpp false @ce, the capture is released after this function,
         * otherwise it stays unchanged. For primary events the capture reset
         * is performed regardless of the event accept status, for non-primary
         * events only if they're accepted.
         *
         * Otherwise, if a node wasn't captured, finds the front-most node
         * under (scaled) @p globalPosition and calls
         * @ref AbstractLayer::pointerMoveEvent() on all data attached to it
         * belonging to layers that support @ref LayerFeature::Event. If
         * no data accept the event, continues to other nodes under the
         * position in a front-to-back order and then to parent nodes. For each
         * such node, the event is always called on all attached data,
         * regardless of the accept status. If the move event is primary, the
         * node on which the data accepted the event is then treated as
         * hovered; if no data accepted the event, there's no hovered node. For
         * each call the event position is made relative to the node to which
         * given data is attached. If the currently hovered node changed, an
         * @ref AbstractLayer::pointerLeaveEvent() is then called for all data
         * attached to a previously hovered node if it exists, and then a
         * corresponding @ref AbstractLayer::pointerEnterEvent() is called for
         * all data attached to the currently hovered node if it exists, in
         * both cases with the same @p event except for
         * @ref PointerMoveEvent::position() which is made relative to the
         * particular node it's called on and
         * @ref PointerMoveEvent::relativePosition() which is reset to a zero
         * vector. If the move event isn't primary, the current hovered node
         * isn't affected in any way. Returns @cpp true @ce if the event was
         * accepted by at least one data, @cpp false @ce if it wasn't or there
         * wasn't any visible event handling node at given position and thus
         * the event should be propagated further; accept status of the enter
         * and leave events is ignored. If any accepted move event or any enter
         * event called @ref PointerMoveEvent::setCaptured() resulting in it
         * being @cpp true @ce, the containing node implicitly captures all
         * further pointer events until and including a
         * @ref pointerReleaseEvent() even if they happen outside of its area,
         * or until the capture is released in a @ref pointerMoveEvent() again.
         * Calling @ref PointerMoveEvent::setCaptured() in the leave event has
         * no effect in this case.
         *
         * Expects that the event is not accepted yet.
         * @see @ref PointerEvent::isAccepted(),
         *      @ref PointerEvent::setAccepted(), @ref currentCapturedNode(),
         *      @ref currentHoveredNode(), @ref currentGlobalPointerPosition()
         */
        bool pointerMoveEvent(const Vector2& globalPosition, PointerMoveEvent& event);

        /**
         * @brief Handle an external pointer move event
         *
         * Converts the @p event to a @ref PointerMoveEvent and delegates to
         * @ref pointerMoveEvent(), see its documentation and
         * @ref Ui-AbstractUserInterface-application for more information.
         * The @p args allow passing optional extra arguments to a particular
         * event converter.
         */
        template<class Event, class ...Args, class = decltype(Implementation::PointerMoveEventConverter<Event>::move(std::declval<AbstractUserInterface&>(), std::declval<Event&>(), std::declval<Args>()...))> bool pointerMoveEvent(Event& event, Args&&... args) {
            return Implementation::PointerMoveEventConverter<Event>::move(*this, event, Utility::forward<Args>(args)...);
        }

        /**
         * @brief Handle a focus event
         *
         * Implicitly calls @ref update(), which in turn implicitly calls
         * @ref clean(). The @p node is expected to be either
         * @ref NodeHandle::Null or valid with @ref NodeFlag::Focusable set.
         *
         * If @p node is non-null and is or any of its parents are not visible
         * either due to @ref NodeFlag::Hidden set or due to the node hierarchy
         * not being in the top-level node order, or have
         * @ref NodeFlag::Disabled or @ref NodeFlag::NoEvents set, the function
         * is a no-op and returns @cpp false @ce. Note that this *does not*
         * apply to nodes that are clipped or otherwise out of view.
         *
         * If the @p node is visible and can be focused, calls
         * @ref AbstractLayer::focusEvent() on all attached data, regardless of
         * the accept status, and even if the current focused node is the same
         * as @p node. Then, if any of them accept the event, the node is set
         * as currently focused, and @ref AbstractLayer::blurEvent() is called
         * on the previously focused node if different from @p node. If none of
         * them accept the event and the previously focused node is different
         * from @p node, the previously focused node stays. If none of them
         * accept the event and the previously focused node is the same as
         * @p node, @ref AbstractLayer::blurEvent() is called on @p node and
         * the current focused node is set to @ref NodeHandle::Null.
         *
         * If @p node is @ref NodeHandle::Null and current focused node is
         * non-null, calls @ref AbstractLayer::blurEvent() on it and sets the
         * current focused node to @ref NodeHandle::Null.
         *
         * Returns @cpp true @ce if the @ref AbstractLayer::focusEvent() was
         * called at all and accepted by at least one data, @cpp false @ce
         * otherwise.
         *
         * Compared to @ref pointerPressEvent(), where pressing on a
         * non-focusable node blurs the previously focused node, this function
         * preserves the previously focused node if @p node is cannot be
         * focused for any of the above reasons. Given this function is meant
         * to be called by the application itself, this gives it more control
         * --- for example to try to focus the next active control, if there's
         * any, and stay on the previous one if not. To achieve the same
         * behavior as @ref pointerPressEvent(), call the function with
         * @ref NodeHandle::Null if a call with a non-null handle fails:
         *
         * @snippet Ui.cpp AbstractUserInterface-focusEvent-blur-if-not-focusable
         *
         * @see @ref currentFocusedNode()
         */
        bool focusEvent(NodeHandle node, FocusEvent& event);

        /**
         * @brief Handle a key press event
         *
         * Implicitly calls @ref update(), which in turn implicitly calls
         * @ref clean().
         *
         * If @ref currentFocusedNode() is not @ref NodeHandle::Null, calls
         * @ref AbstractLayer::keyPressEvent() on all data attached to it
         * belonging to layers that support @ref LayerFeature::Event.
         *
         * Otherwise, if @ref currentGlobalPointerPosition() is not
         * @relativeref{Corrade,Containers::NullOpt}, finds the front-most node
         * under it and calls @ref AbstractLayer::keyPressEvent() on all data
         * attached to it belonging to layers that support
         * @ref LayerFeature::Event. If no data accept the event, continues to
         * other nodes under the position in a front-to-back order and then to
         * parent nodes. For each such node, the event is always called on all
         * attached data, regardless of the accept status. For each call the
         * event contains the @ref currentGlobalPointerPosition() made relative
         * to the node to which given data is attached.
         *
         * Returns @cpp true @ce if the event was accepted by at least one
         * data; @cpp false @ce if it wasn't, if @ref currentFocusedNode() is
         * @ref NodeHandle::Null and there either wasn't any visible event
         * handling node at given position or @ref currentGlobalPointerPosition()
         * is @relativeref{Corrade,Containers::NullOpt}, and thus the event
         * should be propagated further.
         *
         * Expects that the event is not accepted yet.
         * @see @ref KeyEvent::isAccepted(), @ref KeyEvent::setAccepted()
         */
        bool keyPressEvent(KeyEvent& event);

        /**
         * @brief Handle an external key press event
         *
         * Converts the @p event to a @ref KeyEvent and delegates to
         * @ref keyPressEvent(), see its documentation and
         * @ref Ui-AbstractUserInterface-application for more information.
         * The @p args allow passing optional extra arguments to a particular
         * event converter.
         */
        template<class Event, class ...Args, class = decltype(Implementation::KeyEventConverter<Event>::press(std::declval<AbstractUserInterface&>(), std::declval<Event&>(), std::declval<Args>()...))> bool keyPressEvent(Event& event, Args&&... args) {
            return Implementation::KeyEventConverter<Event>::press(*this, event, Utility::forward<Args>(args)...);
        }

        /**
         * @brief Handle a key release event
         *
         * Implicitly calls @ref update(), which in turn implicitly calls
         * @ref clean().
         *
         * If @ref currentFocusedNode() is not @ref NodeHandle::Null, calls
         * @ref AbstractLayer::keyReleaseEvent() on all data attached to it
         * belonging to layers that support @ref LayerFeature::Event.
         *
         * Otherwise, if @ref currentGlobalPointerPosition() is not
         * @relativeref{Corrade,Containers::NullOpt}, finds the front-most node
         * under it and calls @ref AbstractLayer::keyReleaseEvent() on all data
         * attached to it belonging to layers that support
         * @ref LayerFeature::Event. If no data accept the event, continues to
         * other nodes under the position in a front-to-back order and then to
         * parent nodes. For each such node, the event is always called on all
         * attached data, regardless of the accept status. For each call the
         * event contains the @ref currentGlobalPointerPosition() made relative
         * to the node to which given data is attached.
         *
         * Returns @cpp true @ce if the event was accepted by at least one
         * data; @cpp false @ce if it wasn't, if @ref currentFocusedNode() is
         * @ref NodeHandle::Null and there either wasn't any visible event
         * handling node at given position or @ref currentGlobalPointerPosition()
         * is @relativeref{Corrade,Containers::NullOpt}, and thus the event
         * should be propagated further.
         *
         * Expects that the event is not accepted yet.
         * @see @ref KeyEvent::isAccepted(), @ref KeyEvent::setAccepted()
         */
        bool keyReleaseEvent(KeyEvent& event);

        /**
         * @brief Handle an external key release event
         *
         * Converts the @p event to a @ref KeyEvent and delegates to
         * @ref keyReleaseEvent(), see its documentation and
         * @ref Ui-AbstractUserInterface-application for more information.
         * The @p args allow passing optional extra arguments to a particular
         * event converter.
         */
        template<class Event, class ...Args, class = decltype(Implementation::KeyEventConverter<Event>::release(std::declval<AbstractUserInterface&>(), std::declval<Event&>(), std::declval<Args>()...))> bool keyReleaseEvent(Event& event, Args&&... args) {
            return Implementation::KeyEventConverter<Event>::release(*this, event, Utility::forward<Args>(args)...);
        }

        /**
         * @brief Handle a text input event
         *
         * Implicitly calls @ref update(), which in turn implicitly calls
         * @ref clean().
         *
         * If @ref currentFocusedNode() is not @ref NodeHandle::Null, calls
         * @ref AbstractLayer::textInputEvent() on all data attached to it
         * belonging to layers that support @ref LayerFeature::Event.
         *
         * Returns @cpp true @ce if the event was accepted by at least one
         * data; @cpp false @ce if it wasn't or if @ref currentFocusedNode() is
         * @ref NodeHandle::Null, and thus the event should be propagated
         * further.
         *
         * Expects that the event is not accepted yet.
         * @see @ref TextInputEvent::isAccepted(),
         *      @ref TextInputEvent::setAccepted()
         */
        bool textInputEvent(TextInputEvent& event);

        /**
         * @brief Handle an external text input event
         *
         * Converts the @p event to a @ref TextInputEvent and delegates to
         * @ref textInputEvent(), see its documentation and
         * @ref Ui-AbstractUserInterface-application for more information.
         * The @p args allow passing optional extra arguments to a particular
         * event converter.
         */
        template<class Event, class ...Args, class = decltype(Implementation::TextInputEventConverter<Event>::trigger(std::declval<AbstractUserInterface&>(), std::declval<Event&>(), std::declval<Args>()...))> bool textInputEvent(Event& event, Args&&... args) {
            return Implementation::TextInputEventConverter<Event>::trigger(*this, event, Utility::forward<Args>(args)...);
        }

        /**
         * @brief Node pressed by last pointer event
         *
         * Returns handle of a node that was under the pointer for the last
         * @ref pointerPressEvent(), the pointer wasn't released since and the
         * pointer is either captured on that node or didn't leave its area
         * since. If a @ref pointerReleaseEvent() then happens on the node
         * area, all data attached to it will receive a
         * @ref AbstractLayer::pointerTapOrClickEvent().
         *
         * If no pointer press event was called yet, if the event wasn't
         * accepted by any data, if @ref pointerReleaseEvent() was called since
         * or the pointer was uncaptured and left the node area, returns
         * @ref NodeHandle::Null. It also becomes @ref NodeHandle::Null if the
         * node or any of its parents were removed, hidden or have
         * @ref NodeFlag::NoEvents or @ref NodeFlag::Disabled set and
         * @ref update() was called since.
         *
         * The returned handle may be invalid if the node or any of its parents
         * were removed and @ref clean() wasn't called since.
         * @see @ref currentCapturedNode(), @ref currentFocusedNode()
         */
        NodeHandle currentPressedNode() const;

        /**
         * @brief Node captured by last pointer event
         *
         * Returns handle of a node that captured the last
         * @ref pointerPressEvent() or @ref pointerMoveEvent(). All data
         * attached to the captured node then receive all following pointer
         * events until and including a @ref pointerReleaseEvent() even if they
         * happen outside of its area, or until the capture is released in a
         * @ref pointerMoveEvent() again.
         *
         * If no pointer press event was called yet, if the event wasn't
         * accepted by any data, if the capture was disabled or subsequently
         * released with @ref PointerEvent::setCaptured() or if a
         * @ref pointerReleaseEvent() was called since, returns
         * @ref NodeHandle::Null. It also becomes @ref NodeHandle::Null if the
         * node or any of its parents were removed, hidden or have
         * @ref NodeFlag::NoEvents or @ref NodeFlag::Disabled set and
         * @ref update() was called since.
         *
         * The returned handle may be invalid if the node or any of its parents
         * were removed and @ref clean() wasn't called since.
         */
        NodeHandle currentCapturedNode() const;

        /**
         * @brief Node hovered by last pointer event
         *
         * Returns handle of a node that was under the pointer for the last
         * @ref pointerMoveEvent(). All data attached to such node already
         * received a @ref AbstractLayer::pointerEnterEvent(). Once a
         * @ref pointerMoveEvent() leaves its area, all data attached to that
         * node will receive a @ref AbstractLayer::pointerLeaveEvent(), and
         * this either becomes @ref NodeHandle::Null, or another node becomes
         * hovered, receiving an enter event. It's also @ref NodeHandle::Null
         * if no pointer move event was called yet or if the node or any of its
         * parents were removed, hidden or have @ref NodeFlag::NoEvents or
         * @ref NodeFlag::Disabled set and @ref update() was called since.
         *
         * The returned handle may be invalid if the node or any of its parents
         * were removed and @ref clean() wasn't called since.
         */
        NodeHandle currentHoveredNode() const;

        /**
         * @brief Node focused by last pointer or focus event
         *
         * Returns handle of a @ref NodeFlag::Focusable node that was under the
         * pointer for the last @ref pointerPressEvent() or for which
         * @ref focusEvent() was called and at least one data attached to such
         * node accepted the @ref AbstractLayer::focusEvent(). Once the node
         * becomes hidden (either due to @ref NodeFlag::Hidden set or due to
         * the node hierarchy not being in the top-level node order),
         * @ref NodeFlag::NoEvents or @ref NodeFlag::Disabled is set on it or
         * it loses @ref NodeFlag::Focusable, the data receive
         * @ref AbstractLayer::blurEvent() and this becomes
         * @ref NodeHandle::Null. It's also @ref NodeHandle::Null if no node
         * was focused yet or if the node or any of its
         * parents were removed or hidden and @ref update() was called since.
         *
         * The returned handle may be invalid if the node or any of its parents
         * were removed and @ref clean() wasn't called since.
         * @see @ref currentPressedNode()
         */
        NodeHandle currentFocusedNode() const;

        /**
         * @brief Position of last pointer event
         *
         * Returns a position passed to the last primary
         * @ref pointerPressEvent(), @ref pointerReleaseEvent() or
         * @ref pointerMoveEvent(), scaled to match @ref size() instead of
         * @ref windowSize(). If no primary pointer event happened yet, returns
         * @relativeref{Corrade,Containers::NullOpt}.
         * @see @ref PointerEvent::isPrimary(),
         *      @ref PointerMoveEvent::isPrimary()
         */
        Containers::Optional<Vector2> currentGlobalPointerPosition() const;

    private:
        /* Used by set*AnimatorInstance() */
        MAGNUM_UI_LOCAL AbstractAnimator& setAnimatorInstanceInternal(
            #ifndef CORRADE_NO_ASSERT
            const char* messagePrefix,
            #endif
            Containers::Pointer<AbstractAnimator>&& instance, Int type);
        /* Used by removeNode(), advanceAnimations() and clean() */
        MAGNUM_UI_LOCAL void removeNodeInternal(UnsignedInt id);
        /* Used by setNodeFlags(), addNodeFlags() and clearNodeFlags() */
        MAGNUM_UI_LOCAL void setNodeFlagsInternal(UnsignedInt id, NodeFlags flags);
        /* Used by removeNodeInternal(), setNodeOrder() and clearNodeOrder() */
        MAGNUM_UI_LOCAL bool clearNodeOrderInternal(NodeHandle handle);
        /* Used by *Event() functions */
        MAGNUM_UI_LOCAL void callVisibilityLostEventOnNode(UnsignedInt nodeId, VisibilityLostEvent& event, bool canBePressedOrHovering);
        template<void(AbstractLayer::*function)(UnsignedInt, FocusEvent&)> MAGNUM_UI_LOCAL bool callFocusEventOnNode(UnsignedInt nodeId, FocusEvent& event);
        template<void(AbstractLayer::*function)(UnsignedInt, KeyEvent&)> MAGNUM_UI_LOCAL bool callKeyEventOnNode(UnsignedInt nodeId, KeyEvent& even);
        MAGNUM_UI_LOCAL bool callTextInputEventOnNode(UnsignedInt nodeId, TextInputEvent& even);
        template<class Event, void(AbstractLayer::*function)(UnsignedInt, Event&)> MAGNUM_UI_LOCAL bool callEventOnNode(const Vector2& globalPositionScaled, UnsignedInt nodeId, Event& event, bool rememberCaptureOnUnaccepted = false);
        template<class Event, void(AbstractLayer::*function)(UnsignedInt, Event&)> MAGNUM_UI_LOCAL NodeHandle callEvent(const Vector2& globalPositionScaled, UnsignedInt visibleNodeIndex, Event& event);
        template<class Event, void(AbstractLayer::*function)(UnsignedInt, Event&)> MAGNUM_UI_LOCAL NodeHandle callEvent(const Vector2& globalPositionScaled, Event& event);
        template<void(AbstractLayer::*function)(UnsignedInt, KeyEvent&)> MAGNUM_UI_LOCAL bool keyPressOrReleaseEvent(KeyEvent& event);

        struct State;
        Containers::Pointer<State> _state;
};

}}

#endif
