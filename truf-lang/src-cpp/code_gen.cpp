#include "json.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <map>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>

// Forward declarations
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

void processReturn(
    const nlohmann::json& returnStmt, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues, 
    llvm::LLVMContext &Context, 
    llvm::Module *Module
);

void processFunction(
    const nlohmann::json& funcAst,
    llvm::IRBuilder<> &BuilderObj,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &ContextObj,
    llvm::Module *ModuleObj
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

void declareExternalFunction(
    const std::string& functionName,
    llvm::FunctionType* funcType,
    llvm::Module* Module
);


void createPrintFunctions(llvm::LLVMContext& Context, llvm::Module* Module) {
    llvm::IRBuilder<> Builder(Context);

    // Declare printf
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::IntegerType::getInt32Ty(Context),
        llvm::PointerType::get(llvm::Type::getInt8Ty(Context), 0),
        true
    );
    llvm::Function* printfFunc = llvm::Function::Create(
        printfType,
        llvm::Function::ExternalLinkage,
        "printf",
        Module
    );

    // Create the `__compiler_reserved_print_int` function
    llvm::FunctionType* printIntType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(Context),
        { llvm::Type::getInt64Ty(Context) },
        false
    );
    llvm::Function* printIntFunc = llvm::Function::Create(
        printIntType,
        llvm::Function::ExternalLinkage,
        "__compiler_reserved_print_int",
        Module
    );
    {
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(Context, "entry", printIntFunc);
        Builder.SetInsertPoint(entry);

        // Create format string for `printf` ("%ld\n")
        llvm::Value* formatStr = Builder.CreateGlobalStringPtr("%ld\n");

        // Call `printf` with the format string and the argument from `printInt`
        llvm::Value* intArg = printIntFunc->arg_begin();
        Builder.CreateCall(printfFunc, { formatStr, intArg });
        Builder.CreateRetVoid();
    }

    // Create the `__compiler_reserved_print_bool` function
    llvm::FunctionType* printBoolType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(Context),
        { llvm::Type::getInt1Ty(Context) },
        false
    );
    llvm::Function* printBoolFunc = llvm::Function::Create(
        printBoolType,
        llvm::Function::ExternalLinkage,
        "__compiler_reserved_print_bool",
        Module
    );
    {
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(Context, "entry", printBoolFunc);
        Builder.SetInsertPoint(entry);

        llvm::Value* boolArg = printBoolFunc->arg_begin();

        // Convert the boolean value to an integer (0 or 1)
        llvm::Value* boolAsInt = Builder.CreateZExt(boolArg, llvm::Type::getInt32Ty(Context));

        // Create strings for "true" and "false"
        llvm::Value* trueStr = Builder.CreateGlobalStringPtr("true\n");
        llvm::Value* falseStr = Builder.CreateGlobalStringPtr("false\n");

        // Select the appropriate string based on the boolean value
        llvm::Value* cmp = Builder.CreateICmpEQ(boolArg, llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context), 1));
        llvm::Value* selectedStr = Builder.CreateSelect(cmp, trueStr, falseStr);

        // Call printf with the selected string
        Builder.CreateCall(printfFunc, { selectedStr });

        Builder.CreateRetVoid();
    }

    // Create the `__compiler_reserved_print_float` function
    llvm::FunctionType* printFloatType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(Context),
        { llvm::Type::getDoubleTy(Context) }, // Use double for both float and double types
        false
    );
    llvm::Function* printFloatFunc = llvm::Function::Create(
        printFloatType,
        llvm::Function::ExternalLinkage,
        "__compiler_reserved_print_float",
        Module
    );
    {
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(Context, "entry", printFloatFunc);
        Builder.SetInsertPoint(entry);

        // Create format string for `printf` ("%f\n")
        llvm::Value* formatStr = Builder.CreateGlobalStringPtr("%f\n");

        // Get the argument (double type)
        llvm::Value* floatArg = printFloatFunc->arg_begin();

        // Call `printf` with the format string and the argument from `printFloat`
        Builder.CreateCall(printfFunc, { formatStr, floatArg });
        Builder.CreateRetVoid();
    }
}


void declareExternalFunction(
    const std::string& functionName,
    llvm::FunctionType* funcType,
    llvm::Module* Module
) {
    llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        functionName,
        Module
    );
}


void gen_llvm_ir(std::string filepath, nlohmann::json ast) {
    // Initialize the LLVM context and module
    llvm::LLVMContext ContextObj;
    llvm::IRBuilder<> BuilderObj(ContextObj);
    std::unique_ptr<llvm::Module> ModuleObj = std::make_unique<llvm::Module>("truffle_main", ContextObj);

    // Create all intrinsic functions
    createPrintFunctions(ContextObj, ModuleObj.get());

    // Create a symbol table
    std::map<std::string, llvm::Value*> NamedValues;

    // Process the AST
    if (ast["type"] == "Module") {
        for (const auto& stmt : ast["statements"]) {
            if (stmt["type"] == "Function") {
                processFunction(stmt, BuilderObj, NamedValues, ContextObj, ModuleObj.get());
            } else {
                std::cout << "Unhandled top-level statement type: " << stmt["type"] << "\n";
            }
        }
    } else if (ast["type"] == "Function") {
        processFunction(ast, BuilderObj, NamedValues, ContextObj, ModuleObj.get());
    } else {
        std::cout << "Unhandled AST root type: " << ast["type"] << "\n";
    }

    // Output the generated LLVM IR to the specified file
    std::error_code ECObj;
    llvm::raw_fd_ostream DestObj(filepath, ECObj, llvm::sys::fs::OF_None);

    if (ECObj) {
        llvm::errs() << "Could not open file: " << ECObj.message() << "\n";
        return;
    }

    // Print the module to the file
    ModuleObj->print(DestObj, nullptr);

    // Close the file
    DestObj.flush();  // Ensure the file is written to disk
}

// New helper function to process functions
void processFunction(
    const nlohmann::json& funcAst,
    llvm::IRBuilder<> &BuilderObj,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &ContextObj,
    llvm::Module *ModuleObj
) {
    // Create a new symbol table
    // std::map<std::string, llvm::Value*> NamedValues;


    // Extract function name, parameters, return type, and code block
    std::string funcName = funcAst["name"];
    std::vector<std::string> parameters = funcAst["parameters"];
    std::string retTypeStr = funcAst["ret-type"];
    nlohmann::json codeBlock = funcAst["code-block"];

    // Create the function type
    llvm::Type* retType = getLLVMType(retTypeStr, ContextObj);

    std::vector<llvm::Type*> paramTypes;
    for (auto& param : parameters) {
        llvm::Type* paramType = getLLVMType(param, ContextObj);
        paramTypes.push_back(paramType);
    }
    llvm::FunctionType *funcType = llvm::FunctionType::get(retType, paramTypes, false);
    llvm::Function *function = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, funcName, ModuleObj);

    // Set names for all arguments (if any).
    unsigned idx = 0;
    for (llvm::Function::arg_iterator AI = function->arg_begin(); idx != parameters.size(); ++AI, ++idx) {
        AI->setName("arg" + std::to_string(idx));
    }

    // Create a new basic block to start insertion into
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(ContextObj, "entry", function);
    BuilderObj.SetInsertPoint(BB);

    // Process the code block
    processCodeBlock(codeBlock, BuilderObj, NamedValues, ContextObj, ModuleObj);

    // For now, create a default return instruction
    if (retType->isVoidTy()) {
        BuilderObj.CreateRetVoid();
    } else {
        // You might want to modify this to return the actual return value
        llvm::Value *retValue = llvm::ConstantInt::get(retType, 0);
        BuilderObj.CreateRet(retValue);
    }

    // Verify the function
    llvm::verifyFunction(*function);
}


llvm::Type* getLLVMType(const std::string& dtype, llvm::LLVMContext &Context) {
    if (dtype == "I64") {
        return llvm::Type::getInt64Ty(Context);
    } else if (dtype == "I32") {
        return llvm::Type::getInt32Ty(Context);
    } else if (dtype == "F64") {
        return llvm::Type::getDoubleTy(Context);
    } else if (dtype == "F32") {
        return llvm::Type::getFloatTy(Context);
    } else if (dtype == "Bool") {
        return llvm::Type::getInt1Ty(Context);
    } else if (dtype == "Null") {
        return llvm::Type::getVoidTy(Context);
    } else {
        llvm::errs() << "Error: Unsupported data type '" << dtype << "'.\n";
        return nullptr;
    }
}


void processCodeBlock(
    const nlohmann::json& codeBlock, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context, 
    llvm::Module *Module
) {
    for (const auto& stmtJson : codeBlock["statements"]) {
        processStatement(stmtJson, Builder, NamedValues, Context, Module);
    }
}

void processStatement(const nlohmann::json& stmt, llvm::IRBuilder<> &Builder,
                      std::map<std::string, llvm::Value*> &NamedValues,
                      llvm::LLVMContext &Context, llvm::Module *Module) {
    std::string stmtType = stmt["type"];

    if (stmtType == "DeclarationStatement") {
        processDeclarationStatement(stmt, Builder, NamedValues, Context, Module);
    } 
    else if (stmtType == "AssignmentStatement") {
        processAssignmentStatement(stmt, Builder, NamedValues, Context, Module);
    }
    else if (stmtType == "FunctionCall") {
        processFunctionCall(stmt, Builder, NamedValues, Context, Module);
    }
    else if (stmtType == "ReturnStatement") {
        processReturn(stmt, Builder, NamedValues, Context, Module);
    }
    else {
        std::cout << "Warning, unhandled statement type: " << stmtType << "\n\n";
    }
}

void processDeclarationStatement(
    const nlohmann::json& stmt, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context, 
    llvm::Module *Module
) {
    std::string varName = stmt["dst"];
    std::string dtypeStr = stmt["dtype"];
    nlohmann::json src = stmt["src"];

    llvm::Type *varType = getLLVMType(dtypeStr, Context);
    llvm::AllocaInst *alloca = Builder.CreateAlloca(varType, nullptr, varName);

    // Store the initial value
    llvm::Value *initValue = processExpression(src, Builder, NamedValues, Context, Module);
    Builder.CreateStore(initValue, alloca);

    // Add the variable to the symbol table
    NamedValues[varName] = alloca;
}

void processAssignmentStatement(
    const nlohmann::json& stmt, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context, 
    llvm::Module *Module
) {
    std::string varName = stmt["dst"];
    nlohmann::json src = stmt["src"];

    // Check that the variable has been declared
    if (NamedValues.find(varName) == NamedValues.end()) {
        // Variable not found
        // Handle error
        llvm::errs() << "Undefined variable: " << varName << "\n";
        return;
    }

    llvm::AllocaInst *alloca = static_cast<llvm::AllocaInst*>(NamedValues[varName]);

    // Compute the new value
    llvm::Value *newValue = processExpression(src, Builder, NamedValues, Context, Module);

    // Store the new value
    Builder.CreateStore(newValue, alloca);
}

void processFunctionCall(
    const nlohmann::json& functionCall, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*>& NamedValues, 
    llvm::LLVMContext& Context, 
    llvm::Module* Module
) {
    std::string functionName = functionCall["function-name"];

    // Process parameters
    std::vector<llvm::Value*> args;
    for (const auto& param : functionCall["parameters"]) {
        llvm::Value* argValue = processExpression(param, Builder, NamedValues, Context, Module);
        if (!argValue) {
            llvm::errs() << "Error processing argument in function call.\n";
            return;
        }
        args.push_back(argValue);
    }

    if (functionName == "print") {
        // Handle built-in 'print' function
        if (args.size() != 1) {
            llvm::errs() << "Error: 'print' function expects one argument.\n";
            return;
        }

        llvm::Value* arg = args[0];
        llvm::Function* printFunc = nullptr;

        if (arg->getType()->isIntegerTy(64)) {
            // Use the printInt function
            printFunc = Module->getFunction("__compiler_reserved_print_int");
            if (!printFunc) {
                llvm::errs() << "Error: '__compiler_reserved_print_int' function not found.\n";
                return;
            }
        } 
        else if (arg->getType()->isIntegerTy(1)) {
            // Use the printBool function
            printFunc = Module->getFunction("__compiler_reserved_print_bool");
            if (!printFunc) {
                llvm::errs() << "Error: '__compiler_reserved_print_bool' function not found.\n";
                return;
            }
        } 
        else if (arg->getType()->isDoubleTy()) {
            // Use the printFloat function
            printFunc = Module->getFunction("__compiler_reserved_print_float");
            if (!printFunc) {
                llvm::errs() << "Error: '__compiler_reserved_print_float' function not found.\n";
                return;
            }
        }
        else {
            llvm::errs() << "Error: Unsupported type for 'print' function.\n";
            return;
        }

        // Create the function call to the appropriate print function
        Builder.CreateCall(printFunc, { arg });
    } else {
        // Handle user-defined or external functions
        llvm::Function* calleeFunction = Module->getFunction(functionName);
        if (!calleeFunction) {
            // Function not found, declare it as external
            llvm::FunctionType* funcType = llvm::FunctionType::get(
                llvm::Type::getVoidTy(Context),
                {},  // Empty parameter list
                false
            );
            calleeFunction = llvm::Function::Create(
                funcType,
                llvm::Function::ExternalLinkage,
                functionName,
                Module
            );
        }

        // Create the function call
        Builder.CreateCall(calleeFunction, args);
    }
}

void processReturn(
    const nlohmann::json& returnStmt, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues, 
    llvm::LLVMContext &Context, 
    llvm::Module *Module
) {
    // Check if there's a return value
    if (returnStmt.contains("value") && returnStmt.contains("dtype") && returnStmt["dtype"] != "Null") {
        // Process the return value expression
        llvm::Value* returnValue = processExpression(returnStmt["value"], Builder, NamedValues, Context, Module);
        
        if (!returnValue) {
            llvm::errs() << "Error processing return value.\n";
            return;
        }

        // Get the current function
        llvm::Function* currentFunction = Builder.GetInsertBlock()->getParent();
        llvm::Type* returnType = currentFunction->getReturnType();

        // Check if the return value type matches the function's return type
        if (returnValue->getType() != returnType) {
            llvm::errs() << "Return value type does not match function return type.\n";
            return;
        }

        // Create the return instruction
        Builder.CreateRet(returnValue);
    } else {
        // If there's no return value, create a void return
        Builder.CreateRetVoid();
    }
}
llvm::Value* processExpression(
    const nlohmann::json& expr, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context, 
    llvm::Module *Module
) {
    std::string exprType = expr["type"];

    if (exprType == "Literal") {
        return processLiteral(expr, Context);
    } else if (exprType == "Variable") {
        return processVariable(expr, Builder, NamedValues);
    } else if (exprType == "Expression") {
        return processBinaryExpression(expr, Builder, NamedValues, Context, Module);
    } else {
        // Handle other expression types
        return nullptr;
    }
}



llvm::Value* processLiteral(const nlohmann::json& literal, llvm::LLVMContext &Context) {
    std::string dtypeStr = literal["dtype"];
    std::string valueStr = literal["value"];

    if (dtypeStr == "I64") {
        llvm::Type *type = getLLVMType(dtypeStr, Context);
        int64_t value = std::stoll(valueStr);
        return llvm::ConstantInt::get(type, value, true);
    } 
    else if (dtypeStr == "I32") {
        llvm::Type *type = getLLVMType(dtypeStr, Context);
        int32_t value = std::stoi(valueStr);
        return llvm::ConstantInt::get(type, value, true);
    } 
    else if (dtypeStr == "F64") {
        llvm::Type *type = getLLVMType(dtypeStr, Context);
        double value = std::stod(valueStr);
        return llvm::ConstantFP::get(type, value);
    } 
    else if (dtypeStr == "F32") {
        llvm::Type *type = getLLVMType(dtypeStr, Context);
        float value = std::stof(valueStr);
        return llvm::ConstantFP::get(type, value);
    } 
    else if (dtypeStr == "Bool") {
        llvm::Type *type = getLLVMType(dtypeStr, Context);
        bool value = (valueStr == "true");
        return llvm::ConstantInt::get(type, value);
    } 
    else {
        llvm::errs() << "Error: Unsupported literal type '" << dtypeStr << "'.\n";
        return nullptr;
    }
}


llvm::Value* processVariable(
    const nlohmann::json& var, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues
) {
    std::string varName = var["name"];

    // Check if variable exists
    if (NamedValues.find(varName) == NamedValues.end()) {
        // Variable not found
        // Handle error
        llvm::errs() << "Undefined variable: " << varName << "\n";
        return nullptr;
    }

    llvm::AllocaInst *alloca = static_cast<llvm::AllocaInst*>(NamedValues[varName]);
    // Load the value
    return Builder.CreateLoad(alloca->getAllocatedType(), alloca, varName + "_load");
}

llvm::Value* processBinaryExpression(
    const nlohmann::json& expr, 
    llvm::IRBuilder<> &Builder,
    std::map<std::string, llvm::Value*> &NamedValues,
    llvm::LLVMContext &Context, 
    llvm::Module *Module
) {
    nlohmann::json lhsJson = expr["left-operand"];
    nlohmann::json rhsJson = expr["right-operand"];
    std::string op = expr["operator"];

    llvm::Value *L = processExpression(lhsJson, Builder, NamedValues, Context, Module);
    llvm::Value *R = processExpression(rhsJson, Builder, NamedValues, Context, Module);

    if (!L || !R) {
        // Handle error
        return nullptr;
    }

    // Handle Division Operation Separately
    if (op == "/") {
        // Convert operands to f64 (double) if they are not already
        if (!L->getType()->isDoubleTy()) {
            if (L->getType()->isIntegerTy()) {
                L = Builder.CreateSIToFP(L, llvm::Type::getDoubleTy(Context), "lhsDouble");
            } else if (L->getType()->isFloatingPointTy()) {
                L = Builder.CreateFPExt(L, llvm::Type::getDoubleTy(Context), "lhsDoubleExt");
            } else {
                llvm::errs() << "Unsupported type for division on LHS\n";
                return nullptr;
            }
        }

        if (!R->getType()->isDoubleTy()) {
            if (R->getType()->isIntegerTy()) {
                R = Builder.CreateSIToFP(R, llvm::Type::getDoubleTy(Context), "rhsDouble");
            } else if (R->getType()->isFloatingPointTy()) {
                R = Builder.CreateFPExt(R, llvm::Type::getDoubleTy(Context), "rhsDoubleExt");
            } else {
                llvm::errs() << "Unsupported type for division on RHS\n";
                return nullptr;
            }
        }

        // Perform floating-point division
        return Builder.CreateFDiv(L, R, "divtmp");
    }

    // Handle Other Arithmetic Operations
    // If both operands are integers
    if (L->getType()->isIntegerTy() && R->getType()->isIntegerTy()) {
        if (op == "+") {
            return Builder.CreateAdd(L, R, "addtmp");
        } else if (op == "-") {
            return Builder.CreateSub(L, R, "subtmp");
        } else if (op == "*") {
            return Builder.CreateMul(L, R, "multmp");
        } else {
            // Handle other operators
            llvm::errs() << "Unsupported integer operator: " << op << "\n";
            return nullptr;
        }
    }
    // If both operands are floating-point
    else if (L->getType()->isFloatingPointTy() && R->getType()->isFloatingPointTy()) {
        if (op == "+") {
            return Builder.CreateFAdd(L, R, "faddtmp");
        } else if (op == "-") {
            return Builder.CreateFSub(L, R, "fsubtmp");
        } else if (op == "*") {
            return Builder.CreateFMul(L, R, "fmultmp");
        } else {
            // '/' is already handled above
            llvm::errs() << "Unsupported floating-point operator: " << op << "\n";
            return nullptr;
        }
    }
    // If operands are mixed types
    else if ((L->getType()->isIntegerTy() && R->getType()->isFloatingPointTy()) ||
             (L->getType()->isFloatingPointTy() && R->getType()->isIntegerTy())) {

        // Convert integer operand to floating-point
        if (L->getType()->isIntegerTy()) {
            L = Builder.CreateSIToFP(L, R->getType(), "lhsToFP");
        } else if (R->getType()->isIntegerTy()) {
            R = Builder.CreateSIToFP(R, L->getType(), "rhsToFP");
        }

        // Perform floating-point arithmetic
        if (op == "+") {
            return Builder.CreateFAdd(L, R, "faddtmp");
        } else if (op == "-") {
            return Builder.CreateFSub(L, R, "fsubtmp");
        } else if (op == "*") {
            return Builder.CreateFMul(L, R, "fmultmp");
        } else {
            llvm::errs() << "Unsupported operator for mixed types: " << op << "\n";
            return nullptr;
        }
    }
    else {
        // Handle other types or errors
        llvm::errs() << "Type mismatch in binary expression\n";
        return nullptr;
    }
}
