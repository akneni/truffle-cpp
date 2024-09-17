#include "json.hpp"

#include <string>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

llvm::Type* getLLVMType(const std::string& dtype, llvm::LLVMContext &Context);

void processCodeBlock(
    const nlohmann::json& codeBlock,
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context,
    llvm::Module *Module
);

void processStatement(
    const nlohmann::json& stmt,
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context,
    llvm::Module *Module
);

void processDeclarationStatement(
    const nlohmann::json& stmt,
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context,
    llvm::Module *Module
);

void processAssignmentStatement(
    const nlohmann::json& stmt,
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context,
    llvm::Module *Module
);

void processFunctionCall(
    const nlohmann::json& functionCall, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues, 
    llvm::LLVMContext &Context, 
    llvm::Module *Module
);

llvm::Value* processExpression(
    const nlohmann::json& expr,
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context,
    llvm::Module *Module = nullptr
);

llvm::Value* processLiteral(const nlohmann::json& literal, llvm::LLVMContext &Context);

llvm::Value* processVariable(
    const nlohmann::json& var, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues
);

llvm::Value* processBinaryExpression(
    const nlohmann::json& expr,
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context,
    llvm::Module *Module
);



void gen_llvm_ir(std::string filepath, nlohmann::json ast);
