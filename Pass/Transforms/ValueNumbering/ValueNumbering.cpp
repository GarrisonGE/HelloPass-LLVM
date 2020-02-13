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
        //get the name of input file
        string file_name = F.getParent()->getSourceFileName();
        //create *.out file
        int index = file_name.find(".");
        file_name = file_name.substr(0,index);
        string c = ".out";
        file_name.append(c);
        ofstream location_out;
        location_out.open(file_name, std::ios::out|std::ios::app);
        if (!location_out.is_open()) return 0;


        errs() << "ValueNumbering: ";
        errs() << F.getName() << "\n";
    
        if (F.getName() != func_name) return false;

        for (auto& basic_block : F)//iterate over basic blocks of the given function
        {
            map<Value*, int> versionMap;              // map the variable to its latest version number
            map<pair<Value*, int>, int> valueMap;       // map the variable to version number and value number
            map<string, int> exprMap;
            int counter = 1; //represent the value numbering
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
                  //search left operand in value map
                  if(!findInMap(valueMap, v1))
                  {   // if not found in hash table
                    valueMap.insert(make_pair(make_pair(v1, 0),counter));//insert new value into valueMap
                    versionMap[v1] = 0;
                    vn1 = counter;
                    counter ++;
                  }
                  else{                            // if found in hash table
                    vn1 = valueMap.at(make_pair(v1, versionMap[v1]));
                  }
                  //search left operand in value map
                  if(!findInMap(valueMap, v2))
                  {   // if not found in hash table
                    valueMap.insert(make_pair(make_pair(v2, 0),counter));
                    versionMap[v2] = 0;
                    vn2 = counter;
                    counter ++;
                  }
                  else{                            // if found in hash table
                    vn2 = valueMap.at(make_pair(v2, versionMap[v2]));
                  }
                  // get the operation of instruction.
                  string op;
                  if(inst.getOpcode() == Instruction::Add) op = "+";
                  else if(inst.getOpcode() == Instruction::Sub) op = "-";
                  else if(inst.getOpcode() == Instruction::Mul) op = "*";
                  else if(inst.getOpcode() == Instruction::SDiv) op = "/";
                              
                  int vn1o=vn1;
                  int vn2o=vn2;

                  if(inst.isCommutative()) {
                    if(vn1 > vn2) swap(vn1, vn2);
                  }

                  string expr = to_string(vn1) + op + to_string(vn2);
                  string exprOld = to_string(vn1o)+op+to_string(vn2o);//For output
                  Value* ptr = dyn_cast<Value>(&inst);//get the left variable of definititon
                  if(versionMap.find(ptr) == versionMap.end()){
                    versionMap.insert(make_pair(ptr, 0));
                  }
                  else{
                    versionMap[ptr] += 1;
                  }
                  //find the expression is in expression map or not
                  if(exprMap.find(expr) == exprMap.end()) {
                    exprMap[expr] = counter;
                    valueMap.insert(make_pair(make_pair(ptr, versionMap[ptr]),counter));
                    ++ counter;
                  }
                  else{
                    int tmp_counter = exprMap[expr];

                    errs() << "This expression is a Redundancy: " << *ptr << "\n";
                    versionMap[ptr] += 1;
                    valueMap.insert(make_pair(make_pair(ptr, versionMap[ptr]),tmp_counter));

                  }
                    int defi = valueMap.at(make_pair(ptr, versionMap[ptr]));
                    location_out<<defi<< "=" <<exprOld<<"\n"; 
                  
                   
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