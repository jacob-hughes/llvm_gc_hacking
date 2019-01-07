#include <iostream>
#include "codegen.h"
#include "node.h"

using namespace std;

extern int yyparse();
extern FILE* yyin;
extern NBlock* programBlock;

void createCoreFunctions(CodeGenContext& context);

int main(int argc, char **argv)
{
    if ( argc < 2 ) {
      std::cout << "No input given." << std::endl;
      return -1;
    }

    std::string filename = argv[1];

    yyin = fopen(filename.c_str(), "r+") ;
    if ( yyin == nullptr )
    {
       std::cout << "File "<< filename << "not found. Abort" << std::endl;
       return -1;
    }
	yyparse();

    if( yyin != nullptr )
      fclose(yyin);
	/* cout << programBlock << endl; */
    // see http://comments.gmane.org/gmane.comp.compilers.llvm.devel/33877
	InitializeNativeTarget();
	InitializeNativeTargetAsmPrinter();
	InitializeNativeTargetAsmParser();
	CodeGenContext context;
	createCoreFunctions(context);
    
	/* context.generateCode(*programBlock); */
	programBlock->codeGen(context); /* emit bytecode for the toplevel block */
    std::cout << "BEGIN_IR" << std::endl;
	context.module->print(llvm::errs(),nullptr);

	return 0;
}

