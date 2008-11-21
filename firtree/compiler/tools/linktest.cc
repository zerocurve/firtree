/* ------------------------------------------------------------------------ */
/*                                                                          */
/* [firtree.c]                      PL0 Interpreter                             */
/*                                                                          */
/* Copyright (c) 2000 by Doelle, Manns                                      */
/* ------------------------------------------------------------------------ */

#include <firtree/main.h>
#include <firtree/value.h>
#include <firtree/compiler/llvm_compiled_kernel.h>
#include <firtree/linker/sampler_provider.h>
#include "../targets/glsl/glsl-target.h"

#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/PassManager.h"
#include "llvm/Support/Streams.h"
#include "llvm/Assembly/PrintModulePass.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

bool g_OptimizeLLVM = true;
bool g_OptimizeGLSL = true;

enum PrintOpt {
  PrintGLSL,
  PrintLLVM,
};
PrintOpt g_WhatToPrint = PrintGLSL;

void printtype(std::ostream& output, Firtree::KernelTypeSpecifier type)
{
  switch(type) {
    case Firtree::TySpecFloat:
      output << "float";
      break;
    case Firtree::TySpecInt:
      output << "int";
      break;
    case Firtree::TySpecBool:
      output << "bool";
      break;
    case Firtree::TySpecVec2:
      output << "vec2";
      break;
    case Firtree::TySpecVec3:
      output << "vec3";
      break;
    case Firtree::TySpecVec4:
      output << "vec4";
      break;
    case Firtree::TySpecSampler:
      output << "sampler";
      break;
    case Firtree::TySpecColor:
      output << "color";
      break;
    case Firtree::TySpecVoid:
      output << "void";
      break;
    default:
      output << "???";
      break;
  }
}

/* Main Program ------------------------------------------------------------ */

int compile_kernel( const char* fileid )
/* initialize and get source */
{
  int fd = open(fileid, O_RDONLY);
  if(fd == -1)
  {
    fprintf(stderr, "Could not open file '%s' for reading.\n",
        fileid);
    return 0;
  }
  off_t file_size = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);
  char* file_contents = new char[file_size];
  read(fd, file_contents, file_size);
  close(fd);

#if 0
  Firtree::LLVM::CompiledKernel* time_compiler = Firtree::LLVM::CompiledKernel::Create();
  Firtree::GLSLTarget* time_glsl_target = Firtree::GLSLTarget::Create();
  for(int i=0; i<1000; i++)
  {
    bool status = time_compiler->Compile(&file_contents, 1);
    if(status)
    {
      time_glsl_target->ProcessModule( time_compiler->GetCompiledModule() );
    }
  }
  FIRTREE_SAFE_RELEASE(time_compiler);
  FIRTREE_SAFE_RELEASE(time_glsl_target);
#endif

  Firtree::LLVM::CompiledKernel* compiler = Firtree::LLVM::CompiledKernel::Create();
  compiler->SetDoOptimization(g_OptimizeLLVM);

  bool status = compiler->Compile(&file_contents, 1);

  unsigned int log_length;
  const char* const * log = compiler->GetCompileLog(&log_length);
  if(log_length > 0)
  {
    std::cerr << "Log messages:\n";
    while((log != NULL) && (*log != NULL))
    {
      std::cerr << *log << '\n';
      log++;
    }
  }

  if(status)
  {
    Firtree::LLVM::SamplerProvider* sp = 
      Firtree::LLVM::SamplerProvider::CreateFromCompiledKernel(compiler);

    Firtree::LLVM::SamplerProvider::const_iterator it;
    for(it = sp->begin(); it != sp->end(); ++it)
    {
      std::cerr << "Param: " << it->Name << "\n";
    }

    Firtree::Value* v = Firtree::Value::CreateIntValue(5);
    sp->SetParameterValue("src", v);
    FIRTREE_SAFE_RELEASE(v);

    v = Firtree::Value::CreateIntValue(4);
    sp->SetParameterValue("src", v);
    FIRTREE_SAFE_RELEASE(v);

    if(!sp->IsValid()) {
      std::cerr << "Invalid sampler.\n";
    } else {
      Firtree::GLSLTarget* glsl_target = Firtree::GLSLTarget::Create();
      glsl_target->ProcessModule( sp->GetSampleFunction()->getParent(), g_OptimizeGLSL );
      std::cout << glsl_target->GetCompiledGLSL();
      FIRTREE_SAFE_RELEASE(glsl_target);
    }

    FIRTREE_SAFE_RELEASE(sp);

#if 0
    Firtree::LLVM::CompiledKernel::const_iterator it =
      compiler->begin();
    for( ; it != compiler->end(); ++it)
    {
      std::cerr << "Kernel '" << it->first << "' - ";
      std::cerr << it->second.Parameters.size() << " parameter(s)\n";

      std::vector<Firtree::LLVM::KernelParameter>::const_iterator pit =
        it->second.Parameters.begin();
      for( ; pit != it->second.Parameters.end(); ++pit)
      {
        std::cerr << "  - Parameter '" << pit->Name << "' type: ";
        printtype(std::cerr, pit->Type);
        if(pit->IsStatic) { std::cerr << " (static)"; }
        std::cerr << "\n";
      }
    }

    if(g_WhatToPrint == PrintLLVM) {
      llvm::PassManager Passes;
      Passes.add( new llvm::PrintModulePass(&llvm::cout) );
      Passes.run( *(compiler->GetCompiledModule()) );
    }

    if(g_WhatToPrint == PrintGLSL) {
      Firtree::GLSLTarget* glsl_target = Firtree::GLSLTarget::Create();

      glsl_target->ProcessModule( compiler->GetCompiledModule(), g_OptimizeGLSL );

      std::cout << glsl_target->GetCompiledGLSL();

      FIRTREE_SAFE_RELEASE(glsl_target);
    }
#endif
  }

  FIRTREE_SAFE_RELEASE(compiler);

  delete file_contents;

//	int return_value = 0;
//
//	if ( PT_errorCnt() == 0 ) {
//		Firtree::LLVMFrontend llvm_frontend(( firtree )srcterm );
//
//		if ( llvm_frontend.GetCompilationSucceeded() ) {
//			// Output the bitcode file to stdout
//			WriteBitcodeToFile( llvm_frontend.GetCompiledModule(), std::cout );
//		} else {
//			return_value = 1;
//		}
//
//		// Write the log (if any).
//		const std::vector<std::string>& log = llvm_frontend.GetLog();
//
//		std::vector<std::string>::const_iterator i = log.begin();
//
//		for ( ; i != log.end(); i++ ) {
//			fprintf( stderr, "%s\n", i->c_str() );
//		}
//	}
//
//	if ( PT_errorCnt() > 0 ) {
//		fprintf( stderr,"Total %d errors.\n",PT_errorCnt() );
//		STD_ERREXIT;
//	}
//
	return 0;
}

void print_usage(const char* progname, FILE* f)
{
  fprintf(f, "Usage: %s [options] filename\n\n", progname);
  fprintf(f, "Where [options] are zero or more of:\n\n");
  fprintf(f, "\t-help \t\tPrint brief usage information.\n");
  fprintf(f, "\t-[no-]opt-llvm \tActivate [deactivate] LLVM optimisation.\n");
  fprintf(f, "\t-[no-]opt-glsl \tActivate [deactivate] GLSL optimisation.\n");
  fprintf(f, "\t-print \t\tSelect what output to print (Default = GLSL).\n");
  fprintf(f, "\t\t=glsl \tPrint GLSL.\n");
  fprintf(f, "\t\t=llvm \tPrint LLVM.\n");
}

const char *get_arg_string(const char* opt_orig)
{
  const char* opt = opt_orig;
  while(*opt != '\0')
  {
    if(*opt == '=') { return opt + 1 ; }
    ++opt;
  }

  fprintf(stderr, "Option %s requires an argument.\n", opt_orig);
  exit(1);
}

int main( int argc, const char* argv[] )
{
	int rv = 1;

  int filenameidx = 1;
  while((filenameidx < argc) && (argv[filenameidx][0] == '-'))
  {
    const char* opt = argv[filenameidx];
    if(strcmp(opt, "-help") == 0) {
      print_usage(argv[0], stdout);
      return 0;
    } else if(strcmp(opt, "-opt-llvm") == 0) {
      g_OptimizeLLVM = true;
    } else if(strcmp(opt, "-no-opt-llvm") == 0) {
      g_OptimizeLLVM = false;
    } else if(strcmp(opt, "-opt-glsl") == 0) {
      g_OptimizeGLSL = true;
    } else if(strcmp(opt, "-no-opt-glsl") == 0) {
      g_OptimizeGLSL = false;
    } else if(strncmp(opt, "-print", 6) == 0) {
      const char* arg_str = get_arg_string(opt);
      if(strcmp(arg_str, "glsl") == 0) {
        g_WhatToPrint = PrintGLSL;
      } else if(strcmp(arg_str, "llvm") == 0) {
        g_WhatToPrint = PrintLLVM;
      } else {
        fprintf(stderr, "Unknown output type: %s.\n", arg_str);
        print_usage(argv[0], stderr);
        return 1;
      }
    } else {
      fprintf(stderr, "Unknown option: %s\n", opt);
      print_usage(argv[0], stderr);
      return 1;
    }

    ++filenameidx;
  }

  if(filenameidx == argc - 1) {
    rv = compile_kernel( argv[filenameidx] );
  }	else {
    print_usage(argv[0], stderr);
		fprintf( stderr,"missing source\n" );
	}

	return rv;
}

/* vim:sw=2:ts=2:cindent:et
 */
