#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include <string>
#include <unordered_map>
#include <map>
#include <algorithm>
#include<fstream>
#include<iostream>

using namespace llvm;
using namespace std;

#define DEBUG_TYPE "ValueNumbering"

using namespace llvm;

namespace {
struct ValueNumbering : public FunctionPass {
  string func_name = "test";
  static char ID;
  ValueNumbering() : FunctionPass(ID) {}

  bool findInMap(map<pair<Value*, int>, int> valueMap, Value * v){
        for(auto it = valueMap.begin(); it != valueMap.end();++ it){
            if(it->first.first == v) return true;
        }
        return false;
    }

  bool runOnFunction(Function &F) override {
        string file_name = F.getParent()->getSourceFileName();
        int index = file_name.find(".");
        file_name = file_name.substr(0,index);
        string c = ".out";
        //create *.out file
        file_name.append(c);
        ofstream location_out;
        location_out.open(file_name, std::ios::out | std::ios::app);
        if (!location_out.is_open()) return 0;
        errs() << "ValueNumbering: ";
        errs() << F.getName() << "\n";
    
        if (F.getName() != func_name) return false;

        for (auto& basic_block : F)//iterate over basic blocks of the given function
        {
            map<Value*, int> varToVersion;              // map the variable to its latest version number
            map<pair<Value*, int>, int> valueMap;       // map the variable to version number and value number
            map<int, pair<Value*,int>> valueMap_re;     // map the value number to variable and its version number
            map<string, int> exprMap;
            int counter = 1;
            //create 
            for (auto& inst : basic_block)//iterate over the instructions in a BB
            {
                if (inst.isBinaryOp())//is instruction a binary operation?
                {
                  Value* v1 = inst.getOperand(0);//get the left operand
                  Value* v2 = inst.getOperand(1);//get the right operand
                  int vn1;//value numbering of left operand
                  int vn2;//value numbering of right operand
                  errs() << "Value one is:" << *v1<<"\n";
                  errs() << "Value two is:" << *v2<<"\n";
                  errs() << "Op Code:" << inst.getOpcodeName()<<"\n"; // print opcode name
                  if(!findInMap(valueMap, v1))
                  {   // if not found in hash table
                    valueMap.insert(make_pair(make_pair(v1, 0),counter));
                    valueMap_re[counter] = make_pair(v1, 0);   // assign a new VN to the variable
                    varToVersion[v1] = 0;
                    vn1 = counter;
                    counter ++;
                  }
                  else{                            // if found in hash table
                    vn1 = valueMap.at(make_pair(v1, varToVersion[v1]));
                  }
                  if(!findInMap(valueMap, v2))
                  {   // if not found in hash table
                    valueMap.insert(make_pair(make_pair(v2, 0),counter));
                    valueMap_re[counter] = make_pair(v2, 0);   // assign a new VN to the variable
                    varToVersion[v2] = 0;
                    vn2 = counter;
                    counter ++;
                  }
                  else{                            // if found in hash table
                    vn2 = valueMap.at(make_pair(v2, varToVersion[v2]));
                  }
                  
                  string op;
                  if(inst.getOpcode() == Instruction::Add) op = "+";
                  if(inst.getOpcode() == Instruction::Sub) op = "-";
                  if(inst.getOpcode() == Instruction::Mul) op = "*";
                  if(inst.getOpcode() == Instruction::SDiv) op = "/";
                              
                  int vn1o=vn1;
                  int vn2o=vn2;

                  if(inst.isCommutative()) {
                    if(vn1 > vn2) swap(vn1, vn2);
                  }

                  string expr = to_string(vn1) + op + to_string(vn2);
                  string exprOld = to_string(vn1o)+op+to_string(vn2o);
                  Value* ptr = dyn_cast<Value>(&inst);
                  if(varToVersion.find(ptr) == varToVersion.end()){
                    varToVersion.insert(make_pair(ptr, 0));
                  }

                  if(exprMap.find(expr) == exprMap.end()) {
                    exprMap[expr] = counter;
                    if(!findInMap(valueMap, ptr)){
                      valueMap.insert(make_pair(make_pair(ptr, 0),counter));
                      valueMap_re[counter] = make_pair(ptr, 0);
                    }
                    else{
                      varToVersion[ptr] += 1;
                      valueMap.insert(make_pair(make_pair(ptr, varToVersion[ptr]),counter));
                      valueMap_re[counter] = make_pair(ptr, varToVersion[ptr]);
                    }
                      ++ counter;
                  }
                  else{
                    int tmp_counter = exprMap[expr];

                    errs() << "Redunant Computation: " << *ptr << "\n";
                    if(!findInMap(valueMap, ptr)){
                      valueMap.insert(make_pair(make_pair(ptr, 0),tmp_counter));
                      valueMap_re[tmp_counter] = make_pair(ptr, 0);
                    }
                    else{
                      varToVersion[ptr] += 1;
                      valueMap.insert(make_pair(make_pair(ptr, varToVersion[ptr]),tmp_counter));
                      valueMap_re[tmp_counter] = make_pair(ptr, varToVersion[ptr]);
                    }
                  }
                 
                    location_out<<exprMap[expr]<< "=" <<exprOld<<"\n"; 
                  
                   
                }
            }

        }
        location_out.close();
    return false;
  }
};
}
 // end of struct ValueNumbering
  // end of anonymous namespace

char ValueNumbering::ID = 0;
static RegisterPass<ValueNumbering> X("ValueNumbering", "ValueNumbering Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);