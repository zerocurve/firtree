//===========================================================================
/// \file sampler_provider.h
///
/// This file defines the abstract base class all sampler providers should
/// derrive from.

#ifndef __SAMPLER_PROVIDER_H
#define __SAMPLER_PROVIDER_H

#include <firtree/main.h>
#include <firtree/compiler/llvm_compiled_kernel.h>

#include <vector>
#include <string>

namespace llvm { class Function; }

namespace Firtree { class Value; }

namespace Firtree { namespace LLVM {

//===========================================================================
/// \brief A Sampler Provider
///
/// A sampler provider represents a single sampler as a set of LLVM 
/// functions. They are:
///
///  Sample Function: A LLVM function which gets the output of the sampler.
///                   It is passed any non-static sampler parameters.
///  Transform Function: A LLVM function which transforms from pixel
///                      co-ordinates to those expected to be returned by
///                      the destCoord() function.
///  Extent Function: A LLVM function which returns the extent of the sampler
///                   as a vector of floating point values giving the
///                   origin x, origin y, width and height.
class SamplerProvider : public ReferenceCounted, private Uncopiable
{
	protected:
		SamplerProvider();
		~SamplerProvider();

	public:
		/// An iterator over the kernel parameters.
		typedef KernelParameterList::const_iterator const_iterator;

		/// Create a sampler provider from a compiled firtree kernel source.
		/// Note that the state of the compiled kernel is copied on creation
		/// so that the compiler may be re-used. This has the effect that
		/// subsequent changes to the CompiledKernel passed to this function
		/// have no effect on this provider.
		///
		/// If kernel_name is non-empty, the kernel with that name will
		/// be used, otherwise the first kernel defined in the source
		/// will be used.
		static SamplerProvider* CreateFromCompiledKernel(
				const CompiledKernel* kernel,
				const std::string& kernel_name = "");

		/// Get the sample function. The returned value may be 
		/// invalidated after a static parameter is set.
		virtual const llvm::Function* GetSampleFunction() const = 0;

		/// Get the transform function. The returned value may be 
		/// invalidated after a static parameter is set.
		virtual const llvm::Function* GetTransformFunction() const = 0;

		/// Get the extent function. The returned value may be 
		/// invalidated after a static parameter is set.
		virtual const llvm::Function* GetExtentFunction() const = 0;

		/// Return a const iterator pointing to the parameter named
		/// 'name'. If no such parameter exists, return the
		/// same iterator as end().
		virtual const_iterator find(const std::string& name) const = 0;

		/// Return an iterator pointing to the start of the parameter list.
		virtual const_iterator begin() const = 0;

		/// Return an iterator pointing to the end of the parameter list.
		virtual const_iterator end() const = 0;

		/// Return a flag indicating if the sampler is in a valid state.
		/// This usually means that all the static parameters have been
		/// set to some non-NULL value. The return values of 
		/// Get{.*}Function() are undefined if this flag is false.
		virtual bool IsValid() const = 0;

		/// Set the parameter pointed to by a given iterator to the passed
		/// value. Static parameters can cause the value of 
		/// Get{.*}Function() to change. As a convenience this should 
		/// return 'true' if a static parameter was changed.
		/// The Value can be NULL in which case the semantic is to 'unset'
		/// the parameter and release any references associated with it.
		/// Throws an error is param == end().
		virtual bool SetParameterValue(const const_iterator& param,
				const Value* value) = 0;

		/// Get the value of the referenced parameter. Returns NULL if the
		/// parameter doesn't exist or is unset. Note that, therefore,
		/// the return value *can* be NULL even if the parameter *does*
		/// exist. If the caller is interested in keeping the value long 
		/// term, it should be Retain()-ed or Clone()-ed.
		virtual const Value* GetParameterValue(
				const const_iterator& param) const = 0;

		/// Convenience wrapper around SetParameterValue().
		inline bool SetParameterValue(const std::string& param_name,
				const Value* value) 
		{
			return this->SetParameterValue(this->find(param_name), value);
		}

		/// Convenience wrapper around GetParameterValue().
		inline const Value* GetParameterValue(
				const std::string& param_name) const
		{
			return this->GetParameterValue(this->find(param_name));
		}

	private:
};

} } // namespace Firtree::LLVM

#endif // __SAMPLER_PROVIDER_H 

// vim:sw=4:ts=4:cindent:noet
