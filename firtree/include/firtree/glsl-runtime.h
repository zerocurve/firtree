// FIRTREE - A generic image processing library
// Copyright (C) 2007, 2008 Rich Wareham <richwareham@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License verstion as published
// by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

//=============================================================================
/// \file glsl-runtime.h OpenGL rendering methods for Images.
//=============================================================================

//=============================================================================
#ifndef FIRTREE_GLSL_RUNTIME_H
#define FIRTREE_GLSL_RUNTIME_H
//=============================================================================

#include <firtree/kernel.h>
#include <firtree/image.h>

namespace Firtree { 

namespace Internal { 
    template<typename Key> class LRUCache;
}

//=============================================================================
/// An OpenGLContext is an object which knows how to start up, tear down and
/// make active a particular OpenGL context. The GLRenderer object
/// makes use of an OpenGLContext to ensure that the current OpenGL context is
/// the correct one.
///
/// The default implementation does nothing moving the entire burden for
/// OpenGL context management to the controlling app.
class OpenGLContext : public ReferenceCounted
{
    protected:
        OpenGLContext();

    public:
        virtual ~OpenGLContext();

        // ====================================================================
        // CONSTRUCTION METHODS
        
        /// Create a simple 'null' context which does nothing when
        /// MakeCurrent() is called.
        static OpenGLContext* CreateNullContext();

        // ====================================================================
        // MUTATING METHODS
        
        /// Ensure this context is current. Note that this might well be called
        /// an awful lot so if it is an expensive operation, it might well
        /// be worth ensuring it only swaps the context when necessary.
        /// The default implementation throws an error if this is called outside
        /// of a Begin()/End() pair, i.e. when m_BeginDepth == 0.
        ///
        /// Sub classes sould be careful to call this implementation in their
        /// overridden methods.
        virtual void EnsureCurrent();

        /// Begin rendering into this context.
        ///
        /// Sub classes sould be careful to call this implementation in their
        /// overridden methods.
        virtual void Begin();

        /// Finish rendering into this context.
        ///
        /// Sub classes sould be careful to call this implementation in their
        /// overridden methods.
        virtual void End();

        /// Return the number of Begin() calls which are waiting for a matching
        /// End().
        uint32_t GetBeginDepth() const { return m_BeginDepth; }

    protected:
        /// This is initially 0 on construction and is incremented once for
        /// every call to Begin() and decremented once for each call to End().
        uint32_t    m_BeginDepth;
};

//=============================================================================
/// A GLSL rendering context encapsulates all the information Firtree needs
/// to render images into OpenGL windows. The programmer must arrange
/// for a OpenGLContext object which knows how to make a desired OpenGL context
/// 'current'.
///
/// In addition to knowing how to render images, the rendering context
/// caches GLSL shader programs to mitigate the overhead of re-compilation
/// on each draw.
class GLRenderer : public ReferenceCounted {
    protected:
        /// Protected contrution. Use Create*() methods.
        ///@{
        GLRenderer(OpenGLContext* glContext);
        ///@}
        
    public:
        virtual ~GLRenderer();

        // ====================================================================
        // CONSTRUCTION METHODS

        /// Construct a new OpenGL rendering context. If glContext is NULL,
        /// a 'null' context is created via CreateNullContext().
        static GLRenderer* Create(OpenGLContext* glContext = NULL);

        // ====================================================================
        // CONST METHODS

        /// Purge any caches held by this context.
        void CollectGarbage();

        /// Clear the context with the specified color
        void Clear(float r, float g, float b, float a);

        /// Render the passed image into the current OpenGL context
        /// in the rectangle destRect, using the pixels from srcRect in
        /// the image as a source. 
        void RenderInRect(Image* image, const Rect2D& destRect,
                const Rect2D& srcRect);

        /// Convenience wrapper around RenderInRect() which sets the
        /// destination and source rectangles to have identical sizes.
        /// The result us to render the pixels of the image from srcRect 
        /// with the lower-left pixel being at location.
        void RenderAtPoint(Image* image, const Point2D& location,
                const Rect2D& srcRect);

        /// Render an image at 1:1 size with the logical origin of the
        /// image at origin.
        void RenderWithOrigin(Image* image, const Point2D& origin);

    private:

        /// The internal cache of sampler objects.
        Internal::LRUCache<Image*>*     m_SamplerCache;

        /// The context associated with this renderer.
        OpenGLContext*                  m_OpenGLContext;
};

}

//=============================================================================
#endif // FIRTREE_GLSL_RUNTIME_H
//=============================================================================

//=============================================================================
// vim:sw=4:ts=4:cindent:et

