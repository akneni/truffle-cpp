#include "json.hpp"
#include <string>
#include <fstream>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>  

void gen_llvm_ir(std::string filepathObj, nlohmann::json astObj) {
    // Initialize the LLVM context and module
    llvm::LLVMContext ContextObj;
    llvm::IRBuilder<> BuilderObj(ContextObj);
    std::unique_ptr<llvm::Module> ModuleObj = std::make_unique<llvm::Module>("truffle_main", ContextObj);

    // Create the main function: 'int main()'
    llvm::FunctionType* FuncTypeObj = llvm::FunctionType::get(llvm::Type::getVoidTy(ContextObj), false);
    llvm::Function* MainFuncObj = llvm::Function::Create(FuncTypeObj, llvm::Function::ExternalLinkage, "main", ModuleObj.get());
    

    auto mainBlock = llvm::BasicBlock::Create(ContextObj, "entry", MainFuncObj);
    BuilderObj.SetInsertPoint(mainBlock);

    auto x_ptr = BuilderObj.CreateAlloca(
        llvm::Type::getInt64Ty(ContextObj),
        nullptr,
        "x"
    );

    BuilderObj.CreateStore(
        llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(ContextObj),
            10
        ),
        x_ptr
    );


    auto if_true_block = llvm::BasicBlock::Create(ContextObj, "if_true", MainFuncObj);
    BuilderObj.SetInsertPoint(if_true_block);
    llvm::Value* x_val = BuilderObj.CreateLoad(
        llvm::Type::getInt64Ty(ContextObj),
        x_ptr
    );
    llvm::Value* add_op = BuilderObj.CreateAdd(
        x_val,
        llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(ContextObj),
            10
        )
    );
    BuilderObj.CreateStore(
        add_op,
        x_ptr
    );

    auto else_block = llvm::BasicBlock::Create(ContextObj, "else", MainFuncObj);
    BuilderObj.SetInsertPoint(else_block);
    x_val = BuilderObj.CreateLoad(
        llvm::Type::getInt64Ty(ContextObj),
        x_ptr
    );
    add_op = BuilderObj.CreateAdd(
        x_val,
        llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(ContextObj),
            -10
        )
    );
    BuilderObj.CreateStore(
        add_op,
        x_ptr
    );

    BuilderObj.SetInsertPoint(mainBlock);


    x_val = BuilderObj.CreateLoad(
        llvm::Type::getInt64Ty(ContextObj),
        x_ptr
    );
    llvm::Value* x_greater_than_10 = BuilderObj.CreateICmpSGT(
        x_val,
        llvm::ConstantInt::get(
            llvm::Type::getInt64Ty(ContextObj),
            15
        )
    );
    BuilderObj.CreateCondBr(
        x_greater_than_10,
        if_true_block,
        else_block
    );

    BuilderObj.CreateRetVoid();

    llvm::verifyFunction(*MainFuncObj);


    // Output the generated LLVM IR to the specified file
    std::error_code ECObj;
    llvm::raw_fd_ostream DestObj(filepathObj, ECObj, llvm::sys::fs::OF_None);

    if (ECObj) {
        llvm::errs() << "Could not open file: " << ECObj.message() << "\n";
        return;
    }

    // Print the module to the file
    ModuleObj->print(DestObj, nullptr);

    // Close the file
    DestObj.flush();  // Ensure the file is written to disk
}
