// FIRTREE
// Copyright (C) 2007, 2008 Rich Wareham <richwareham@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License version 2 for more details.
//
// You should have received a copy of the GNU General Public License version 2
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

%module(directors=1) Firtree
%{
#include <firtree/main.h>
#include <firtree/image.h>
#include <firtree/blob.h>
#include <firtree/math.h>
#include <firtree/kernel.h>
#include <firtree/value.h>
#include <firtree/glsl-runtime.h>
#include <firtree/cpu-runtime.h>

using namespace Firtree;

%}

%include "cdata.i" 
%include "std_string.i" 
%include "std_vector.i" 

#ifdef SWIGPYTHON

namespace std {
  %template(stringList) vector<std::string>;
}

%inline %{
/* Build void pointer */
    namespace Firtree {
        const void* IntToVoidPtr(const unsigned long ival) {
            return reinterpret_cast<const void*>(ival);
        }
    }
%}

%apply unsigned int { uint32_t }
%apply int { int32_t }

// %feature("ref")   Firtree::ReferenceCounted "printf(\"Ref: %p\\n\", $this);"
// %feature("unref") Firtree::ReferenceCounted "printf(\"Unref: %p\\n\", $this); $this->Release();"

%feature("director") Firtree::ImageProvider;
%feature("director") Firtree::OpenGLContext;

// Suppress "Warning(473): Returning a pointer or reference 
//           in a director method is not recommended."
%warnfilter(473) Firtree::ImageProvider;

%feature("director:except") {
    if ($error != NULL) {
        throw Swig::DirectorMethodException();
    }
}

// Special typemap allowing us to return a BitmapImageRep from
// an ImageProvider. This doesn't try to claim ownership of the object.
%typemap(directorout) Firtree::BitmapImageRep* {
    void* _p;
    SWIG_ConvertPtr($1, &_p, SWIGTYPE_p_Firtree__BitmapImageRep, 0);
    $result = static_cast<Firtree::BitmapImageRep*>(_p);
}

// Let blobs be assigned from Python strings
%apply (char *STRING, int LENGTH) { (const void *buffer, size_t length) };

%feature("ref")   Firtree::ReferenceCounted ""
%feature("unref") Firtree::ReferenceCounted "$this->Release();"

%newobject Firtree::Blob::Create;
%newobject Firtree::Blob::CreateWithLength;
%newobject Firtree::Blob::CreateFromBuffer;
%newobject Firtree::Blob::CreateFromBlob;

%extend Firtree::Blob
{
    SWIGCDATA GetData() const
    {
        return cdata_void((void*)($self->GetBytes()), $self->GetLength());
    }
}

%newobject Firtree::SamplerParameter::CreateFromImage;
%newobject Firtree::SamplerParameter::GetAndOwnTransform;

%newobject Firtree::Kernel::CreateFromSource;

%newobject Firtree::Image::CreateFromImage;
%newobject Firtree::Image::CreateFromBitmapData;
%newobject Firtree::Image::CreateFromFile;
%newobject Firtree::Image::CreateFromImageWithTransform;
%newobject Firtree::Image::CreateFromImageCroppedTo;
%newobject Firtree::Image::CreateFromKernel;
%newobject Firtree::Image::CreateFromImageProvider;

%newobject Firtree::ImageAccumulator::Create;

%newobject Firtree::AccumulationImage::Create;

%newobject Firtree::BitmapImageRep::Create;
%newobject Firtree::BitmapImageRep::CreateFromBitmapImageRep;

%newobject Firtree::ImageProvider::CreateImageRep;

%newobject Firtree::Parameter::Create;

%newobject Firtree::AffineTransform::Identity;
%newobject Firtree::AffineTransform::FromTransformStruct;
%newobject Firtree::AffineTransform::Scaling;
%newobject Firtree::AffineTransform::RotationByDegrees;
%newobject Firtree::AffineTransform::RotationByRadians;
%newobject Firtree::AffineTransform::Translation;

%newobject Firtree::OpenGLContext::CreateNullContext;
%newobject Firtree::OpenGLContext::CreateOffScreenContext;

%newobject Firtree::GLRenderer::Create;
%newobject Firtree::GLRenderer::CreateBitmapImageRepFromImage;

%newobject Firtree::CPURenderer::Create;
%newobject Firtree::CPURenderer::CreateImage;
%newobject Firtree::CPURenderer::CreateBitmapImageRepFromImage;

%newobject Firtree::ExtentProvider::CreateStandardExtentProvider;

%newobject Firtree::RectComputeTransform;

%newobject Firtree::Value::Create;
%newobject Firtree::Value::CreateFloatValue;
%newobject Firtree::Value::CreateIntValue;
%newobject Firtree::Value::CreateBoolValue;
%newobject Firtree::Value::CreateVectorValue;
%newobject Firtree::Value::Clone;

/* Some cleverness to allow us to wire up python callables
 * as an extent provider */

%{
    class PythonExtentProvider : public Firtree::ExtentProvider
    {
        public:
            PythonExtentProvider(PyObject* pyfunc)
                :   Firtree::ExtentProvider()
                ,   m_Function(pyfunc)
            {
                Py_INCREF(m_Function);
            }

            virtual ~PythonExtentProvider()
            {
                Py_DECREF(m_Function);
            }

            virtual Rect2D ComputeExtentForKernel(Kernel* kernel)
            {
                PyObject *arglist, *pyKernel;
                PyObject *result;

                pyKernel = 
                    SWIG_NewPointerObj(SWIG_as_voidptr(kernel), 
                            SWIGTYPE_p_Firtree__Kernel, 0);
                arglist = Py_BuildValue("(O)", pyKernel);

                result = PyEval_CallObject(m_Function, arglist);

                Py_XDECREF(arglist);

                Rect2D resultRect = Firtree::Rect2D();

                if(result)
                {
                    Rect2D *r;
                    int ok = SWIG_ConvertPtr(result, (void**)(&r), 
                            SWIGTYPE_p_Firtree__Rect2D, SWIG_POINTER_EXCEPTION);
                    if(ok == 0)
                    {
                        resultRect = *r;
                    }
                }

                Py_XDECREF(result);

                return resultRect;
            }

        private:
            PyObject*       m_Function;
    };
%}

/* Grab a Python function object as a Python object. */
%typemap(in) PyObject *pyfunc {
  if (!PyCallable_Check($input)) {
      PyErr_SetString(PyExc_TypeError, "Need a callable object!");
      return NULL;
  }
  $1 = $input;
}

/* Attach a new method to Image to use a Python callable
 * as an extent provider */
%extend Firtree::Image {
    static Image* CreateFromKernel(Firtree::Kernel* k, PyObject* pyfunc)
    {
        ExtentProvider* ep = new PythonExtentProvider(pyfunc);
        Image* retVal = Firtree::Image::CreateFromKernel(k, ep);
        FIRTREE_SAFE_RELEASE(ep);
        return retVal;
    }
}

/* Convert from Python --> C */
%typemap(in) uint32_t {
    $1 = PyInt_AsLong($input);
}

/* Convert from C --> Python */
%typemap(out) uint32_t {
    $result = PyInt_FromLong($1);
}

%exception {
    try {
        $action
    } catch (Swig::DirectorException &e) { 
        SWIG_fail; 
    } catch ( Firtree::Exception& e ) {
        std::string message = e.GetFunction();
        message += ": ";
        message += e.GetMessage();
        PyErr_SetString(PyExc_RuntimeError, const_cast<char*>(message.c_str()));
        return NULL;
    }
}
#endif

%include <firtree/main.h>
%include <firtree/blob.h>
%include <firtree/math.h>
%include <firtree/image.h>
%include <firtree/kernel.h>
%include <firtree/glsl-runtime.h>
%include <firtree/cpu-runtime.h>

/* So that VIM does the "right thing"
 * vim:cindent:sw=4:ts=4:et
 */
