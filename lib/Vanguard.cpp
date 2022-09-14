//
// Created by Jon Stephens on 8/19/22.
//

#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/AsmParser/Parser.h>
#include <llvm/CodeGen/AsmPrinter.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/PassRegistry.h>
#include <llvm/InitializePasses.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/IR/Module.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/Passes/StandardInstrumentations.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/CodeGen/CommandFlags.h>
#include <llvm/CodeGen/TargetPassConfig.h>
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Demangle/Demangle.h>
#include "program/Universe.h"
#include "program/LLVMFactory.h"
#include "detectors/DetectorRegistry.h"
#include "program/Universe.h"
#include "program/LLVMFactory.h"
#include "domain/libBlockchain/BlockchainFactory.h"
#include "domain/libBlockchain/Blockchain.h"
#include "program/Top.h"
#include "detectors/DetectorResult.h"
#include "detectors/DetectorReport.h"
#include "domain/libBlockchain/rapidjson/document.h"

static llvm::cl::list<std::string> detectors("detectors", llvm::cl::desc("Vanguard Detectors to Run"), llvm::cl::CommaSeparated, llvm::cl::OneOrMore, llvm::cl::Optional);
static llvm::cl::list<std::string> inputFiles(llvm::cl::Positional, llvm::cl::desc("<Input files>"), llvm::cl::OneOrMore);

template<typename Domain>
vanguard::DetectorReport *performDetection(llvm::ModuleAnalysisManager &mam, llvm::FunctionAnalysisManager &fam, vanguard::UniverseDetector<Domain> *detector, Domain &universe) {
    auto requirements = detector->registerAnalyses();
    for(auto req : requirements) {
        req->fetch(mam, fam);
    }

    detector->startDetection();
    detector->detect(universe);
    return detector->report();
}

template<typename Domain>
void runDetectors(llvm::ModuleAnalysisManager &mam, llvm::FunctionAnalysisManager &fam, vanguard::DetectorRegistry &registry, const std::vector<std::string>& detectorNames, Domain &universe) {
    auto humanReadableReport = new vanguard::HumanReadableFormat();
    auto jsonReport = new vanguard::JsonFormat();
    rapidjson::Document jsondoc;

    for(auto name : detectorNames) {
        auto detector = registry.get<Domain>(name);
        if(detector == nullptr) {
            llvm::errs() << "Unknown pass: " << name << "\n";
        }
    }
    std::string finalJsonString = "";
    finalJsonString += jsonReport->preamble();
    for(auto name : detectorNames) {
        auto detector = registry.get<Domain>(name);
        if(detector != nullptr) {
            vanguard::DetectorReport *detectorReport = performDetection(mam, fam, detector, universe);
            //std::cout << humanReadableReport->format(*detectorReport);
            finalJsonString += jsonReport->format(*detectorReport);
        }
    }
    finalJsonString += jsonReport->postamble();
    jsondoc.Parse(finalJsonString.c_str());
    rapidjson::StringBuffer strbuf;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
    jsondoc.Accept(writer);
    std::string jsonString = strbuf.GetString();
    std::cout << jsonString;
}

void initializeLLVM(int argc, char **argv) {
    llvm::InitLLVM init(argc, argv);
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmParsers();
    llvm::PassRegistry &registry = *llvm::PassRegistry::getPassRegistry();
    llvm::initializeCore(registry);
    llvm::initializeCoroutines(registry);
    llvm::initializeScalarOpts(registry);
    llvm::initializeObjCARCOpts(registry);
    llvm::initializeVectorization(registry);
    llvm::initializeIPO(registry);
    llvm::initializeAnalysis(registry);
    llvm::initializeTransformUtils(registry);
    llvm::initializeInstCombine(registry);
    llvm::initializeAggressiveInstCombine(registry);
    llvm::initializeInstrumentation(registry);
    llvm::initializeTarget(registry);
    // For codegen passes, only passes that do IR to IR transformation are
    // supported.
    llvm::initializeExpandMemCmpPassPass(registry);
    llvm::initializeScalarizeMaskedMemIntrinLegacyPassPass(registry);
    llvm::initializeCodeGenPreparePass(registry);
    llvm::initializeAtomicExpandPass(registry);
    llvm::initializeRewriteSymbolsLegacyPassPass(registry);
    llvm::initializeWinEHPreparePass(registry);
    llvm::initializeDwarfEHPrepareLegacyPassPass(registry);
    llvm::initializeSafeStackLegacyPassPass(registry);
    llvm::initializeSjLjEHPreparePass(registry);
    llvm::initializePreISelIntrinsicLoweringLegacyPassPass(registry);
    llvm::initializeGlobalMergePass(registry);
    llvm::initializeIndirectBrExpandPassPass(registry);
    llvm::initializeInterleavedLoadCombinePass(registry);
    llvm::initializeInterleavedAccessPass(registry);
    llvm::initializeEntryExitInstrumenterPass(registry);
    llvm::initializePostInlineEntryExitInstrumenterPass(registry);
    llvm::initializeUnreachableBlockElimLegacyPassPass(registry);
    llvm::initializeExpandReductionsPass(registry);
    llvm::initializeExpandVectorPredicationPass(registry);
    llvm::initializeWasmEHPreparePass(registry);
    llvm::initializeWriteBitcodePassPass(registry);
    llvm::initializeHardwareLoopsPass(registry);
    llvm::initializeTypePromotionPass(registry);
    llvm::initializeReplaceWithVeclibLegacyPass(registry);

    llvm::cl::ParseCommandLineOptions(argc, argv, "Vanguard Static Analyzer\n");
}

int main(int argc, char **argv) {
    initializeLLVM(argc, argv);

    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    llvm::PassInstrumentationCallbacks PIC;
    llvm::PrintPassOptions PrintPassOpts;
    PrintPassOpts.Verbose = true;
    PrintPassOpts.SkipAnalyses = false;
    llvm::StandardInstrumentations SI(true, true, PrintPassOpts);
    llvm::PipelineTuningOptions PTO;
    PTO.LoopUnrolling = false;

    /*llvm::Triple ModuleTriple(modules[0]->getTargetTriple());
    std::string CPUStr, FeaturesStr;
    llvm::TargetMachine *Machine = nullptr;
    const llvm::TargetOptions Options =
            llvm::codegen::InitTargetOptionsFromCodeGenFlags(ModuleTriple);

    if (ModuleTriple.getArch()) {
        CPUStr = llvm::codegen::getCPUStr();
        FeaturesStr = llvm::codegen::getFeaturesStr();
        Machine = GetTargetMachine(ModuleTriple, CPUStr, FeaturesStr, Options);
    } else if (ModuleTriple.getArchName() != "unknown" &&
               ModuleTriple.getArchName() != "") {
        llvm::errs() << argv[0] << ": unrecognized architecture '"
                     << ModuleTriple.getArchName() << "' provided.\n";
        return 1;
    }

    std::unique_ptr<llvm::TargetMachine> TM(Machine);*/
    llvm::PassBuilder PB(nullptr, PTO, llvm::None, &PIC);

    llvm::AAManager AA;
    if (auto Err = PB.parseAAPipeline(AA, "default")) {
        return EXIT_FAILURE;
    }

    FAM.registerPass([&] { return std::move(AA); });

    // Register all the basic analyses with the managers.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager MPM;

    llvm::cl::PrintOptionValues();

    llvm::LLVMContext ctxt;

    llvm::SMDiagnostic Err;

    ctxt.setDiscardValueNames(false);

    auto setDataLayout = [](llvm::StringRef) -> llvm::Optional<std::string> {
        return llvm::None;
    };

    std::unordered_map<llvm::Module *, std::string> filenames;
    std::vector<std::unique_ptr<llvm::Module>> modules;
    for (auto inFile: inputFiles) {
        auto module = parseIRFile(inputFiles[0], Err, ctxt, setDataLayout);
        if (!module) {
            Err.print(argv[0], llvm::errs());
            return 1;
        }

        filenames[module.get()] = inFile;
        modules.push_back(move(module));
        MPM.run(*module, MAM);
    }

    vanguard::DetectorRegistry &detectorRegistry = vanguard::DetectorRegistry::getInstance();

    auto detectorNames = detectors.empty() ? detectorRegistry.all() : detectors;
    auto domain = detectorRegistry.domain(detectorNames);

    if(domain == vanguard::Detector::BASIC) {
        vanguard::LLVMFactory factory;
        std::vector<vanguard::Universe::CompilationUnit *> units;
        units.reserve(modules.size());
for(auto &mod : modules) {
            units.push_back(factory.createUnit(modules.back().get()));
        }
        vanguard::Top<vanguard::Universe> universe(factory, units);
        runDetectors<vanguard::Top<vanguard::Universe>>(MAM, FAM, detectorRegistry, detectorNames, universe);
    }
    else if(domain == vanguard::Detector::BLOCKCHAIN) {
        vanguard::BlockchainFactory factory;
        std::vector<vanguard::Universe::CompilationUnit *> units;
        units.reserve(modules.size());
        for(auto &mod : modules) {
            std::string filename = filenames[mod.get()];
            std::string summary = filename.substr(0, filename.length() - 2) + "json";
            units.push_back(factory.createUnit(modules.back().get(), summary));
        }
        vanguard::Top<vanguard::Blockchain<vanguard::Universe>> universe(factory, units);
        runDetectors<vanguard::Top<vanguard::Blockchain<vanguard::Universe>>>(MAM, FAM, detectorRegistry, detectorNames, universe);
    }
    else {
        throw std::runtime_error("Unknown domain");
    }

    /* Example of how to fetch results from the opt passes
    for (auto &module: modules) {
        for(auto &fn : *module) {
            if(!fn.hasName() || fn.isDeclaration()) {
                continue;
            }

            auto &result = FAM.getResult<llvm::AAManager>(fn);
        }
    }*/

}