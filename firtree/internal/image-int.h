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
/// \file image-int.h The internal interface to a FIRTREE image.
//=============================================================================

//=============================================================================
#ifndef FIRTREE_IMAGE_INT_H
#define FIRTREE_IMAGE_INT_H
//=============================================================================

#include <stdlib.h>

#include <firtree/include/main.h>
#include <firtree/include/math.h>
#include <firtree/include/blob.h>

namespace Firtree { namespace Internal {

//=============================================================================
/// The internal implementation of the public Image class.
class ImageImpl : public Image
{
    protected:
        ///@{
        /// Protected constructors/destructors. Use the various Image
        /// factory functions instead.
        ImageImpl();
        ImageImpl(const Image& im);
        ImageImpl(const BitmapImageRep& imageRep, bool copyData);
        virtual ~ImageImpl();
        ///@}

    public:
        // ====================================================================
        // CONSTRUCTION METHODS

        // ====================================================================
        // CONST METHODS

        /// Return the size of image that would be returned by 
        /// GetAsOpenGLTexture() or GetAsBitmapImageRep().
        Size2D GetUnderlyingPixelSize() const;

        // ====================================================================
        // MUTATING METHODS

        /// These functions can potentially mutate the class since they will
        /// convert the image to the requested type 'on the fly'.
        ///@{
        
        /// Return an OpenGL texture object handle containing this image.
        unsigned int GetAsOpenGLTexture();
        
        ///@}

    protected:
        /// The image representation as a binary blob or NULL if there is 
        /// none yet.
        BitmapImageRep* m_BitmapRep;

        /// The image representation as an OpenGL texture or 0 if there is 
        /// none yet.
        unsigned int    m_GLTexture;

        friend class Firtree::Image;
};

} }

//=============================================================================
#endif // FIRTREE_IMAGE_INT_H
//=============================================================================

//=============================================================================
// vim:sw=4:ts=4:cindent:et

