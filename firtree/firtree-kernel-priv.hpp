/* firtree-kernel-priv.hpp */

/* Firtree - A generic image processing library
 * Copyright (C) 2009 Rich Wareham <richwareham@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License verstion as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef _FIRTREE_KERNEL_PRIV
#define _FIRTREE_KERNEL_PRIV

#include "firtree-kernel.h"

#include <llvm/Module.h>

G_BEGIN_DECLS

/**
 * firtree_kernel_get_llvm_module:
 * @self: A FirtreeKernel instance.
 *
 * Get a pointer to the LLVM module resulting from the last call to
 * firtree_kernel_compile_from_source(). If the kernel has not been
 * successfully compiled, NULL is returned.
 *
 * Returns: NULL or an LLVM module.
 */
llvm::Module*
firtree_kernel_get_llvm_module(FirtreeKernel* self);

G_END_DECLS

#endif /* _FIRTREE_KERNEL_PRIV */

/* vim:sw=4:ts=4:et:cindent
 */
