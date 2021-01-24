cmake_minimum_required(VERSION 3.5)
project (kng)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W0" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-everything")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")
endif()

if(NOT LLVM_PATH)
    message(FATAL_ERROR "LLVM_PATH is required")
endif()


set(EXEC Release)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

message("llvm dir:")
message("${LLVM_PATH}/include")

include_directories(
        "${LLVM_PATH}/include"
        src/vendor/cxxopts/include
        src/vendor/Catch2-2.13.3/include
        src/vendor/spdlog-1.8.1/include
        src/
)

add_definitions(${LLVM_DEFINITIONS})

set(ALL_FILES
        src/ast.cpp
        src/common.cpp
        src/compiler.cpp
        src/error.cpp
        src/exporter.cpp
        src/llvmgenerator.cpp
        src/x86generator.cpp
        src/lexer.cpp
        src/main.cpp
        src/parser.cpp
        src/token.cpp
        src/typechecking.cpp
        src/types.cpp
        )

#if(DCMAKE_BUILD_TYPE STREQUAL "Release")
#    link_directories("${LLVM_PATH}/Release/lib")
#elseif(DCMAKE_BUILD_TYPE STREQUAL "Debug")
#    link_directories("${LLVM_PATH}/Debug/lib")
#endif()


link_directories("${LLVM_PATH}/${CMAKE_BUILD_TYPE}/lib")

# Now build our tools
add_executable(${EXEC} ${ALL_FILES})

set(llvm_libraries
        DynamicLibraryLib.lib
        ExampleIRTransforms.lib
        gtest.lib
        gtest_main.lib
        Kaleidoscope-Ch4.lib
        Kaleidoscope-Ch5.lib
        Kaleidoscope-Ch6.lib
        Kaleidoscope-Ch7.lib
        Kaleidoscope-Ch8.lib
        Kaleidoscope-Ch9.lib
        LLVMAArch64AsmParser.lib
        LLVMAArch64CodeGen.lib
        LLVMAArch64Desc.lib
        LLVMAArch64Disassembler.lib
        LLVMAArch64Info.lib
        LLVMAArch64Utils.lib
        LLVMAggressiveInstCombine.lib
        LLVMAMDGPUAsmParser.lib
        LLVMAMDGPUCodeGen.lib
        LLVMAMDGPUDesc.lib
        LLVMAMDGPUDisassembler.lib
        LLVMAMDGPUInfo.lib
        LLVMAMDGPUUtils.lib
        LLVMAnalysis.lib
        LLVMARMAsmParser.lib
        LLVMARMCodeGen.lib
        LLVMARMDesc.lib
        LLVMARMDisassembler.lib
        LLVMARMInfo.lib
        LLVMARMUtils.lib
        LLVMAsmParser.lib
        LLVMAsmPrinter.lib
        LLVMAVRAsmParser.lib
        LLVMAVRCodeGen.lib
        LLVMAVRDesc.lib
        LLVMAVRDisassembler.lib
        LLVMAVRInfo.lib
        LLVMBinaryFormat.lib
        LLVMBitReader.lib
        LLVMBitstreamReader.lib
        LLVMBitWriter.lib
        LLVMBPFAsmParser.lib
        LLVMBPFCodeGen.lib
        LLVMBPFDesc.lib
        LLVMBPFDisassembler.lib
        LLVMBPFInfo.lib
        LLVMCFGuard.lib
        LLVMCodeGen.lib
        LLVMCore.lib
        LLVMCoroutines.lib
        LLVMCoverage.lib
        LLVMDebugInfoCodeView.lib
        LLVMDebugInfoDWARF.lib
        LLVMDebugInfoGSYM.lib
        LLVMDebugInfoMSF.lib
        LLVMDebugInfoPDB.lib
        LLVMDemangle.lib
        LLVMDlltoolDriver.lib
        LLVMDWARFLinker.lib
        LLVMExecutionEngine.lib
        LLVMExtensions.lib
        LLVMFileCheck.lib
        LLVMFrontendOpenACC.lib
        LLVMFrontendOpenMP.lib
        LLVMFuzzMutate.lib
        LLVMGlobalISel.lib
        LLVMHelloNew.lib
        LLVMHexagonAsmParser.lib
        LLVMHexagonCodeGen.lib
        LLVMHexagonDesc.lib
        LLVMHexagonDisassembler.lib
        LLVMHexagonInfo.lib
        LLVMInstCombine.lib
        LLVMInstrumentation.lib
        LLVMInterfaceStub.lib
        LLVMInterpreter.lib
        LLVMipo.lib
        LLVMIRReader.lib
        LLVMJITLink.lib
        LLVMLanaiAsmParser.lib
        LLVMLanaiCodeGen.lib
        LLVMLanaiDesc.lib
        LLVMLanaiDisassembler.lib
        LLVMLanaiInfo.lib
        LLVMLibDriver.lib
        LLVMLineEditor.lib
        LLVMLinker.lib
        LLVMLTO.lib
        LLVMMC.lib
        LLVMMCA.lib
        LLVMMCDisassembler.lib
        LLVMMCJIT.lib
        LLVMMCParser.lib
        LLVMMipsAsmParser.lib
        LLVMMipsCodeGen.lib
        LLVMMipsDesc.lib
        LLVMMipsDisassembler.lib
        LLVMMipsInfo.lib
        LLVMMIRParser.lib
        LLVMMSP430AsmParser.lib
        LLVMMSP430CodeGen.lib
        LLVMMSP430Desc.lib
        LLVMMSP430Disassembler.lib
        LLVMMSP430Info.lib
        LLVMNVPTXCodeGen.lib
        LLVMNVPTXDesc.lib
        LLVMNVPTXInfo.lib
        LLVMObjCARCOpts.lib
        LLVMObject.lib
        LLVMObjectYAML.lib
        LLVMOption.lib
        LLVMOrcJIT.lib
        LLVMOrcShared.lib
        LLVMOrcTargetProcess.lib
        LLVMPasses.lib
        LLVMPowerPCAsmParser.lib
        LLVMPowerPCCodeGen.lib
        LLVMPowerPCDesc.lib
        LLVMPowerPCDisassembler.lib
        LLVMPowerPCInfo.lib
        LLVMProfileData.lib
        LLVMRemarks.lib
        LLVMRISCVAsmParser.lib
        LLVMRISCVCodeGen.lib
        LLVMRISCVDesc.lib
        LLVMRISCVDisassembler.lib
        LLVMRISCVInfo.lib
        LLVMRISCVUtils.lib
        LLVMRuntimeDyld.lib
        LLVMScalarOpts.lib
        LLVMSelectionDAG.lib
        LLVMSparcAsmParser.lib
        LLVMSparcCodeGen.lib
        LLVMSparcDesc.lib
        LLVMSparcDisassembler.lib
        LLVMSparcInfo.lib
        LLVMSupport.lib
        LLVMSymbolize.lib
        LLVMSystemZAsmParser.lib
        LLVMSystemZCodeGen.lib
        LLVMSystemZDesc.lib
        LLVMSystemZDisassembler.lib
        LLVMSystemZInfo.lib
        LLVMTableGen.lib
        LLVMTableGenGlobalISel.lib
        LLVMTarget.lib
        LLVMTestingSupport.lib
        LLVMTextAPI.lib
        LLVMTransformUtils.lib
        LLVMVectorize.lib
        LLVMWebAssemblyAsmParser.lib
        LLVMWebAssemblyCodeGen.lib
        LLVMWebAssemblyDesc.lib
        LLVMWebAssemblyDisassembler.lib
        LLVMWebAssemblyInfo.lib
        LLVMWindowsManifest.lib
        LLVMX86AsmParser.lib
        LLVMX86CodeGen.lib
        LLVMX86Desc.lib
        LLVMX86Disassembler.lib
        LLVMX86Info.lib
        LLVMXCoreCodeGen.lib
        LLVMXCoreDesc.lib
        LLVMXCoreDisassembler.lib
        LLVMXCoreInfo.lib
        LLVMXRay.lib
        LTO.lib
        Remarks.lib
        )

# Link against LLVM libraries
target_link_libraries(${EXEC} ${llvm_libraries})



#https://stackoverflow.com/questions/33253664/cmake-set-visualstudion2010-parameters-additional-library-directories
#https://stackoverflow.com/questions/28597351/how-do-i-add-a-library-path-in-cmake
#https://stackoverflow.com/questions/30867712/add-llvm-to-project-using-cmake