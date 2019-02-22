#include <iostream>
#include "codegen.h"
#include "node.h"

using namespace std;

extern int yyparse();
extern NBlock* programBlock;


llvm::Function* createLogFunction(CodeGenContext& context)
{
    std::vector<llvm::Type*> log_arg_types;
    log_arg_types.push_back(llvm::Type::getInt64PtrTy(MyContext, 1)); //char*

    llvm::FunctionType* log_type =
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(MyContext), log_arg_types, false);

    llvm::Function *func = llvm::Function::Create(
                log_type, llvm::Function::ExternalLinkage,
                llvm::Twine("logger"),
                context.module
           );
    func->setCallingConv(llvm::CallingConv::C);
    return func;
}

llvm::Function* createForceGcFunction(CodeGenContext& context)
{
    std::vector<llvm::Type*> no_args;

    llvm::FunctionType* force_gc_ty =
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(MyContext), no_args, false);

    llvm::Function *func = llvm::Function::Create(
                force_gc_ty, llvm::Function::ExternalLinkage,
                llvm::Twine("force_gc"),
                context.module
           );
    func->setCallingConv(llvm::CallingConv::C);
    return func;
}


llvm::Function* createGcAllocFunction(CodeGenContext& context)
{
    std::vector<llvm::Type*> gc_alloc_arg_types;
    gc_alloc_arg_types.push_back(llvm::Type::getInt64Ty(MyContext)); //char*

    llvm::FunctionType* gc_alloc_ty =
        llvm::FunctionType::get(
            llvm::Type::getInt64PtrTy(MyContext, 1), gc_alloc_arg_types, false);

    llvm::Function *func = llvm::Function::Create(
                gc_alloc_ty, llvm::Function::ExternalLinkage,
                llvm::Twine("gc_alloc"),
                context.module
           );
    func->setCallingConv(llvm::CallingConv::C);
    return func;
}

void createEchoFunction(CodeGenContext& context, llvm::Function* printfFn)
{
    std::vector<llvm::Type*> echo_arg_types;
    echo_arg_types.push_back(llvm::Type::getInt64Ty(MyContext));

    llvm::FunctionType* echo_type =
        llvm::FunctionType::get(
            llvm::Type::getVoidTy(MyContext), echo_arg_types, false);

    llvm::Function *func = llvm::Function::Create(
                echo_type, llvm::Function::InternalLinkage,
                llvm::Twine("echo"),
                context.module
           );
    llvm::BasicBlock *bblock = llvm::BasicBlock::Create(MyContext, "entry", func, 0);
	context.pushBlock(bblock);
    
    const char *constValue = "%d\n";
    llvm::Constant *format_const = llvm::ConstantDataArray::getString(MyContext, constValue);
    llvm::GlobalVariable *var =
        new llvm::GlobalVariable(
            *context.module, llvm::ArrayType::get(llvm::IntegerType::get(MyContext, 8), strlen(constValue)+1),
            true, llvm::GlobalValue::PrivateLinkage, format_const, ".str");
    llvm::Constant *zero =
        llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(MyContext));

    std::vector<llvm::Constant*> indices;
    indices.push_back(zero);
    indices.push_back(zero);
    llvm::Constant *var_ref = llvm::ConstantExpr::getGetElementPtr(
	llvm::ArrayType::get(llvm::IntegerType::get(MyContext, 8), strlen(constValue)+1),
        var, indices);

    std::vector<Value*> args;
    args.push_back(var_ref);

    Function::arg_iterator argsValues = func->arg_begin();
    Value* toPrint = &*argsValues++;
    toPrint->setName("toPrint");
    args.push_back(toPrint);
    
	CallInst *call = CallInst::Create(printfFn, makeArrayRef(args), "", bblock);
	ReturnInst::Create(MyContext, bblock);
	context.popBlock();
}

void createCoreFunctions(CodeGenContext& context){
	createLogFunction(context);
    createGcAllocFunction(context);
    createForceGcFunction(context);
    /* createEchoFunction(context, printfFn); */
}
