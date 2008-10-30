//===========================================================================
/// \file llvm_binop_logic.cc

#include <firtree/main.h>

#include "llvm_backend.h"
#include "llvm_private.h"
#include "llvm_emit_decl.h"
#include "llvm_expression.h"
#include "llvm_type_cast.h"
#include "llvm_emit_constant.h"

#include <llvm/Instructions.h>

using namespace llvm;

namespace Firtree
{

//===========================================================================
/// \brief Class to emit an logic binary operator.
class BinaryOpLogicEmitter : ExpressionEmitter
{
	public:
		BinaryOpLogicEmitter()
				: ExpressionEmitter() {
		}

		virtual ~BinaryOpLogicEmitter() {
		}

		/// Create an instance of this emitter.
		static ExpressionEmitter* Create() {
			return new BinaryOpLogicEmitter();
		}

		/// Massage the values passed to be boolean values.
		bool MassageBinOpTypes( LLVMContext* context,
		                        firtreeExpression binopexpr,
		                        ExpressionValue* left_in,
		                        ExpressionValue* right_in,
		                        ExpressionValue** left_out,
		                        ExpressionValue** right_out ) {
			ExpressionValue* left_cast = NULL;
			ExpressionValue* right_cast = NULL;

			try {
				// Attempt to cast both sides.
				left_cast = TypeCaster::CastValue( context,
						binopexpr,
						left_in, FullType::TySpecBool );
				right_cast = TypeCaster::CastValue( context,
						binopexpr,
						right_in, FullType::TySpecBool );
			} catch ( CompileErrorException e ) {
				FIRTREE_SAFE_RELEASE( left_cast );
				FIRTREE_SAFE_RELEASE( right_cast );
				throw e;
			}

			*left_out = left_cast;
			FIRTREE_SAFE_RELEASE( left_in );
			*right_out = right_cast;
			FIRTREE_SAFE_RELEASE( right_in );

			return true;
		}

		/// Emit the passed expression term returning a pointer to
		/// a ExpressionValue containing it's value. It is the
		/// responsibility of the caller to Release() this value.
		virtual ExpressionValue* Emit( LLVMContext* context,
		                               firtreeExpression expression ) {
			firtreeExpression left, right;

			// Emit code for the left and right.
			ExpressionValue* left_val = NULL;
			ExpressionValue* right_val = NULL;
			ExpressionValue* return_val = NULL;

			Instruction::BinaryOps op = Instruction::BinaryOpsEnd;

			if ( firtreeExpression_logicaland( expression, &left, &right ) ) {
				op = Instruction::And;
			} else if ( firtreeExpression_logicalor( expression, &left, &right ) ) {
				op = Instruction::Or;
			} else if ( firtreeExpression_logicalxor( expression, &left, &right ) ) {
				op = Instruction::Xor;
			} else {
				FIRTREE_LLVM_ICE( context, expression,
				                  "Unknown logic operator." );
			}

			try {
				left_val = ExpressionEmitterRegistry::GetRegistry()->Emit(
				               context, left );
				right_val = ExpressionEmitterRegistry::GetRegistry()->Emit(
				                context, right );

				if ( ! MassageBinOpTypes( context, expression,
				                          left_val, right_val,
				                          &left_val, &right_val ) ) {
					FIRTREE_LLVM_ERROR( context, expression,
					                    "Incompatible types for binary "
					                    "operator." );
				}

				llvm::Value* result_val = BinaryOperator::create( op,
				                          left_val->GetLLVMValue(),
				                          right_val->GetLLVMValue(),
				                          "tmpbinop", context->BB );
				return_val =  ConstantExpressionValue::
				              Create( context, result_val );

				FIRTREE_SAFE_RELEASE( left_val );
				FIRTREE_SAFE_RELEASE( right_val );
			} catch ( CompileErrorException e ) {
				FIRTREE_SAFE_RELEASE( left_val );
				FIRTREE_SAFE_RELEASE( right_val );
				throw e;
			}

			return return_val;
		}
};

//===========================================================================
// Register the emitter.
RegisterEmitter<BinaryOpLogicEmitter> g_BinaryOpOrReg( "logicalor" );
RegisterEmitter<BinaryOpLogicEmitter> g_BinaryOpAndReg( "logicaland" );
RegisterEmitter<BinaryOpLogicEmitter> g_BinaryOpXorReg( "logicalxor" );
/*
RegisterEmitter<BinaryOpLogicEmitter> g_BinaryOpEqReg( "equal" );
RegisterEmitter<BinaryOpLogicEmitter> g_BinaryOpNeReg( "notequal" );
RegisterEmitter<BinaryOpLogicEmitter> g_BinaryOpGtReg( "greater" );
RegisterEmitter<BinaryOpLogicEmitter> g_BinaryOpGteReg( "greaterequal" );
RegisterEmitter<BinaryOpLogicEmitter> g_BinaryOpLtReg( "less" );
RegisterEmitter<BinaryOpLogicEmitter> g_BinaryOpLteReg( "lessequal" );
*/

}

// vim:sw=4:ts=4:cindent:noet
