
//===-- HelloWorld.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/ssa2dot.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DebugInfo.h"

#include "filesystem"
#include "vector"

#include <iostream>
#include <fstream>

using namespace llvm;

void writeGraph(std::string sourceName, std::string functionName);
void readSourceCode(std::string sourceName);

PreservedAnalyses SSAtoDot::run(Function &F,
                                      FunctionAnalysisManager &AM) {



  Module *M = F.getParent();
  std::string sourceName = M->getSourceFileName();




  StringRef functionName = F.getName();


  errs() << "Analyzing Function: " << functionName <<  "\n";


    std::vector<const llvm::Argument*> argumentArray;

    for (const llvm::Argument &arg : F.args())
    {
      argumentArray.push_back(&arg);
    }

    errs() << "Num of Arg: " << argumentArray.size() <<  "\n";

    if (argumentArray.size() > 0)
    {
      errs() << "Arguments of " << functionName << "\n";

      for (const llvm::Argument* arg : argumentArray)
      {
        errs() << *arg << "\n";
      }
    }
    else
    {
      errs() << "No Arguments for Function" << functionName << "\n";
    }



    for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b)
    {


      BasicBlock* BB = &*b;

      errs() << BB->getName() << " (Block) \n";

      int count  = 1;
      for (llvm::BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i)
      {
        Instruction* inst = &*i;





        if (DILocation *diLocation = inst->getDebugLoc())
        {

          auto cLine = diLocation->getLine();

          inst->setDebugLoc(DebugLoc());
          errs() << *inst << " InstructioS # " << count << "@: " << cLine << "\n";
          inst->setDebugLoc(diLocation);
        }
        else
        {
          errs() << *inst << " Instruction # " << count << "\n";
        }




        count += 1;
      }

    }

    writeGraph(sourceName, functionName.str());


    return PreservedAnalyses::all();
}



void writeGraph(std::string sourceName, std::string functionName)
{

  sourceName = sourceName.erase(sourceName.size()-2) + "_" + functionName + ".dot";

  std::ofstream outputFile;
  outputFile.open(sourceName);

  outputFile.clear();

  outputFile << "digraph G { A -> B; B -> C; C -> A; }";


  outputFile.close();

}


void readSourceCode(std::string sourceName)
{
  std::vector<std::string> sourceLines;

  sourceLines.push_back("NULL");

  std::ifstream inputFile;

  inputFile.open(sourceName);

  std::string individualLine;
  while (std::getline(inputFile, individualLine))
  {
    sourceLines.push_back(individualLine);
  }




  for (int i = 0; i < sourceLines.size(); i++)
  {

    errs() << "L#" << i << ": " << sourceLines[i] << "\n";

  }


}




