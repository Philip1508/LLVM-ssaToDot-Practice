
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
#include "llvm/Support/raw_ostream.h"

#include "filesystem"
#include "vector"
#include <map>

#include <iostream>
#include <fstream>

using namespace llvm;

void writeGraph(std::string sourceName, std::string functionName, const std::vector<std::string>& clusterVector);
void readSourceCode(std::string sourceName);
struct GraphMetadata mapInstructionToNodeIdentifier(Function &F);



struct GraphMetadata
{
  std::map<BasicBlock*, int> basicBlockToNumber;
  std::map<Instruction*, std::string> instructionToNodeIdentifier;
  std::map<BasicBlock*, Instruction*> basicBlockToFirstInstruction;
};





PreservedAnalyses SSAtoDot::run(Function &F,
                                      FunctionAnalysisManager &AM) {

    DISubprogram *functionInfo = F.getSubprogram();
    if (functionInfo == nullptr)
    {
      errs() << "LLVM Program has not been compiled with -g flag or this particular function is not debuggable. "
                "Aborting all procedure. Function in question: " << F.getName()  <<  "\n";
      return PreservedAnalyses::all();
    }

    std::string LLVM_Cluster = "LLVM";
    std::string C_Cluster = "C";

    GraphMetadata metaData = mapInstructionToNodeIdentifier(F);






    StringRef functionName = F.getName();
    unsigned int lineNum = functionInfo->getLine();


    std::vector<std::string> LLVM_ClusterArray;




    std::string argumentList;
    llvm::raw_string_ostream argumentStream(argumentList);

    std::vector<const llvm::Argument*> argumentArray;
    for (const llvm::Argument &arg : F.args())
    {
      argumentArray.push_back(&arg);
    }

    for (const llvm::Argument* arg : argumentArray)
    {
      argumentStream << *arg << "\n";
    }



    std::string llvmClusterStr;
    llvm::raw_string_ostream llvmMainClusterStream(llvmClusterStr);

    llvmMainClusterStream << "subgraph cluster_" << LLVM_Cluster << "_1" << "{";
    llvmMainClusterStream << "label = \"" << LLVM_Cluster << "\";";
    llvmMainClusterStream << "FNAME [label = \"" << functionName << "\"]";




    if (!argumentArray.empty())
    {
      llvmMainClusterStream << "ARGUMENTS [label = \"" << argumentStream.str() << "\" , shape=ellipse, color=red]";
      llvmMainClusterStream << " FNAME -> ARGUMENTS; " << "ARGUMENTS -> " << metaData.instructionToNodeIdentifier[&F.front().front()] << "; ";
    }
    else
    {
      llvmMainClusterStream << " FNAME -> " << metaData.instructionToNodeIdentifier[&F.front().front()] << "; ";
    }


    std::string branchEdges;
    llvm::raw_string_ostream branchEdgesStream(branchEdges);



    // Basic Block Iteration Loop
    for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b)
    {
      std::string basicBlockDOTCluster;
      llvm::raw_string_ostream basicBlockClusterStream(basicBlockDOTCluster);


      BasicBlock* BB = &*b;

      basicBlockClusterStream << "\n subgraph cluster_" << metaData.basicBlockToNumber[BB] << " {"; // Begin Cluster
      basicBlockClusterStream << "label = \"" << BB->getName() << "\";"; // Label the Cluster with the Basic Block Name






      std::vector<std::string> toLink;


      // Instruction Iteration Loop
      for (llvm::BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i)
      {

        Instruction* inst = &*i;


        std::string nodeIdentifier;
        llvm::raw_string_ostream nodeIdentifierStream(nodeIdentifier);

        nodeIdentifierStream << metaData.instructionToNodeIdentifier[inst];


        std::string clearedString;
        llvm::raw_string_ostream stringCleaner(clearedString);


        ReturnInst *returnInst = llvm::dyn_cast<llvm::ReturnInst>(inst);

        std::string retColor = "";
        if (returnInst)
        {
          retColor = "shape=ellipse, color=lightgreen";
        }

        if (DILocation *diLocation = inst->getDebugLoc())
        {



          unsigned int cLine = diLocation->getLine();
          inst->setDebugLoc(DebugLoc()); // DI Removal for Label

          stringCleaner << *inst;
          clearedString = stringCleaner.str();
          clearedString.erase(std::remove(clearedString.begin(), clearedString.end(), '\"'), clearedString.end());


          basicBlockClusterStream << nodeIdentifierStream.str() << " ";
          basicBlockClusterStream << "[label = \"" << clearedString << "\"" << retColor <<"];";
          inst->setDebugLoc(diLocation); // DI ADD FOR INTEGRITY


        }
        else
        {
          stringCleaner << *inst;
          clearedString = stringCleaner.str();
          clearedString.erase(std::remove(clearedString.begin(), clearedString.end(), '\"'), clearedString.end());


          basicBlockClusterStream << nodeIdentifierStream.str() << " ";
          basicBlockClusterStream << "[label = \"" << clearedString << "\"" << retColor <<"];";
        }

        if (BranchInst *branchInstruction = llvm::dyn_cast<llvm::BranchInst>(inst))
        {
          int num = branchInstruction->getNumSuccessors();

          for (int i = 0; i < num; ++i)
          {
            if (llvm::BasicBlock* successorBlock = branchInstruction->getSuccessor(i))
            {
              std::string str;
              llvm::raw_string_ostream stream(str);

              Instruction* instPrime = &successorBlock->front();


              stream << " " << metaData.instructionToNodeIdentifier[inst] << "->" << metaData.instructionToNodeIdentifier[instPrime] << " [color = red]" << "; ";

              branchEdgesStream << stream.str();

            }
          }
        }

        if (SwitchInst *switchInstruction = llvm::dyn_cast<llvm::SwitchInst>(inst))
        {
          int num = switchInstruction->getNumSuccessors();

          for (int i = 0; i < num; ++i)
          {
            if (llvm::BasicBlock* successorBlock = switchInstruction->getSuccessor(i))
            {
              std::string str;
              llvm::raw_string_ostream stream(str);

              Instruction* instPrime = &successorBlock->front();

              std::string G;
              if (i == 0)
              {
                G = " [label=\"default\", color=red] ";
              }

              stream << " " << metaData.instructionToNodeIdentifier[inst] << "->" << metaData.instructionToNodeIdentifier[instPrime] << G << "; ";

              branchEdgesStream << stream.str();

            }
          }

        }

        if (InvokeInst *invokeInst = llvm::dyn_cast<llvm::InvokeInst>(inst))
        {
          if (llvm::BasicBlock* BB = invokeInst->getNormalDest())
          {
            std::string str;
            llvm::raw_string_ostream stream(str);
            Instruction* instPrime = &BB->front();
            stream << " " << metaData.instructionToNodeIdentifier[inst] << "->" << metaData.instructionToNodeIdentifier[instPrime] << " [color = blue]" << "; ";
            branchEdgesStream << stream.str();

          }
          if (llvm::BasicBlock* BB = invokeInst->getNormalDest())
          {
            if (BB != invokeInst -> getNormalDest())
            {
              std::string str;
              llvm::raw_string_ostream stream(str);
              Instruction* instPrime = &BB->front();
              stream << " " << metaData.instructionToNodeIdentifier[inst] << "->" << metaData.instructionToNodeIdentifier[instPrime] << " [color = blue]" << "; ";
              branchEdgesStream << stream.str();
            }
          }



        }

        /*
        if (CallInst *branchInstruction = llvm::dyn_cast<llvm::CallInst>(inst))
        {
          if (Instruction* postCall = branchInstruction->getNextNode())
          {
            std::string str;
            llvm::raw_string_ostream stream(str);
            stream << " " << metaData.instructionToNodeIdentifier[inst] << "->" << metaData.instructionToNodeIdentifier[postCall] << "; ";
            branchEdgesStream << stream.str();

          }
        }*/


        toLink.push_back(nodeIdentifierStream.str());
      }

      for (size_t i = 0; i < toLink.size(); ++i)
      {
        if (i+1 < toLink.size())
        {
          basicBlockClusterStream << " " << toLink[i] << "->" << toLink[i+1] << " ";
        }

      }


      basicBlockClusterStream << "} \n"; // Cluster End


      llvmMainClusterStream << basicBlockClusterStream.str();



    }







    Module *M = F.getParent();
    std::string sourceName = M->getSourceFileName();


    llvmMainClusterStream << "\n " << branchEdgesStream.str() << "\n";



    llvmMainClusterStream << "}";


    LLVM_ClusterArray.push_back(llvmMainClusterStream.str());


    writeGraph(sourceName, functionName.str(), LLVM_ClusterArray);


    return PreservedAnalyses::all();




}



void writeGraph(std::string sourceName, std::string functionName, const std::vector<std::string>& clusterVector)
{

  sourceName = sourceName.erase(sourceName.size()-2) + "_" + functionName + ".dot";

  std::ofstream outputFile;
  outputFile.open(sourceName);

  outputFile.clear();


  std::string graphString;
  llvm::raw_string_ostream stringOstream(graphString);

  stringOstream << "digraph G {     node [shape=box, style=filled, color=lightblue]; \n" ;




  for (size_t i = 0; i < clusterVector.size(); i++)
  {
    stringOstream << clusterVector[i];
  }

  stringOstream << "\n }";


  outputFile << stringOstream.str();


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




  for (std::size_t i = 0; i < sourceLines.size(); i++)
  {

    errs() << "L#" << i << ": " << sourceLines[i] << "\n";

  }




}



struct GraphMetadata mapInstructionToNodeIdentifier(Function &F)
{
  std::map<BasicBlock*, int> basicBlockToNumber;
  std::map<Instruction*, std::string> instructionToNodeIdentifier;
  std::map<BasicBlock*, Instruction*> basicBlockToFirstInstruction;


  int clusterCount = 100;
  // Basic Block Iteration Loop
  for (Function::iterator b = F.begin(), be = F.end(); b != be; ++b)
  {
    BasicBlock* BB = &*b;


    int clusterInstruction = 0;



    bool firstInstruction = true;

    // Instruction Iteration Loop
    for (llvm::BasicBlock::iterator i = BB->begin(), ie = BB->end(); i != ie; ++i)
    {
      Instruction* inst = &*i;

      if (firstInstruction)
      {
        basicBlockToFirstInstruction[BB] = inst;

        firstInstruction = false;
      }

      basicBlockToFirstInstruction[BB] = inst;

      std::string nodeIdentifier;
      llvm::raw_string_ostream nodeIdentifierStream(nodeIdentifier);
      nodeIdentifierStream << "Instruction" << clusterCount << "_" << clusterInstruction;


      instructionToNodeIdentifier[inst] = nodeIdentifierStream.str();

      clusterInstruction++;
    }

    basicBlockToNumber[BB] = clusterCount;
    clusterCount++; // Increment Cluster Count for uniqueness of Clusters
  }

  struct GraphMetadata metadata = {basicBlockToNumber, instructionToNodeIdentifier, basicBlockToFirstInstruction};



  return metadata;
}



