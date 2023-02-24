#include <wasmedge/wasmedge.h>
//#include "wasmedge.h"
#include "wasm_runner.h"
#include "Util.h"
#include "Node.h"
#include <cstdio>
//#include <host/wasi/wasimodule.h>
//#include <host/wasmedge_process/processmodule.h>

// gcc wasmedge_runner.cpp -lwasmedge -o test_wasmedge

// supports externref threads …
// https://wasmedge.org/book/en/sdk/c.html
// https://wasmedge.org/book/en/sdk/c/externref.html

#define VMCxt context

uint32_t SquareFunc(uint32_t A) { return A * A; }

uint32_t AddFunc(uint32_t A, uint32_t B) { return A + B; }

uint32_t MulFunc(uint32_t A, uint32_t B) { return A * B; }

// Host function to call `SquareFunc` by external reference
WasmEdge_Result ExternSquare(void *Data,
                             const WasmEdge_CallingFrameContext *CallFrameCxt,
                             const WasmEdge_Value *In, WasmEdge_Value *Out) {
    // Function type: {externref, i32} -> {i32}
//	uint32_t (*Func)(uint32_t) = WasmEdge_ValueGetExternRef(In[0]);
//	uint32_t C = Func(WasmEdge_ValueGetI32(In[1]));
//	Out[0] = WasmEdge_ValueGenI32(C);
    return WasmEdge_Result_Success;
}


// Host function to call `AddFunc` by external reference
WasmEdge_Result ExternAdd(void *Data,
                          const WasmEdge_CallingFrameContext *CallFrameCxt,
                          const WasmEdge_Value *In, WasmEdge_Value *Out) {
    // Function type: {externref, i32, i32} -> {i32}
//	uint32_t (*Func)(uint32_t, uint32_t) = WasmEdge_ValueGetExternRef(In[0]);
//	uint32_t C = Func(WasmEdge_ValueGetI32(In[1]), WasmEdge_ValueGetI32(In[2]));
//	Out[0] = WasmEdge_ValueGenI32(C);
    return WasmEdge_Result_Success;
}

// Helper function to create the "extern_module" module instance.
WasmEdge_ModuleInstanceContext *CreateExternModule() {
    WasmEdge_String HostName;
    WasmEdge_FunctionTypeContext *HostFType = NULL;
    WasmEdge_FunctionInstanceContext *HostFunc = NULL;
    enum WasmEdge_ValType P[3], R[1];

    HostName = WasmEdge_StringCreateByCString("extern_module");
    WasmEdge_ModuleInstanceContext *HostMod = WasmEdge_ModuleInstanceCreate(HostName);
    WasmEdge_StringDelete(HostName);

    // Add host function "functor_square": {externref, i32} -> {i32}
    P[0] = WasmEdge_ValType_ExternRef;
    P[1] = WasmEdge_ValType_I32;
    R[0] = WasmEdge_ValType_I32;
    HostFType = WasmEdge_FunctionTypeCreate(P, 2, R, 1);
    HostFunc = WasmEdge_FunctionInstanceCreate(HostFType, ExternSquare, NULL, 0);
    WasmEdge_FunctionTypeDelete(HostFType);
    HostName = WasmEdge_StringCreateByCString("functor_square");
    WasmEdge_ModuleInstanceAddFunction(HostMod, HostName, HostFunc);
    return HostMod;
}


extern "C" int64 run_wasm(bytes buffer, int buf_size) {
    // perfect except we can't access memory

    /* Create the configure context and add the WASI support. */
    /* This step is not necessary unless you need WASI support. */
    WasmEdge_ConfigureContext *conf = WasmEdge_ConfigureCreate();
    WasmEdge_ConfigureAddHostRegistration(conf, WasmEdge_HostRegistration_Wasi);

    //--enable-instruction-count
    //--enable-gas-measuring
    //--enable-time-measuring
    // wasmedge --enable-all
    // most are enabled by default and need explicit disabling --disable-multi-value …
    // ⚠️ "Though the user can specify enabling the proposal, the support for the proposal is not implemented yet." :(
    // Those implemented have a check mark: ✔️	https://wasmedge.org/book/en/features/proposals.html
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_BulkMemoryOperations);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_ReferenceTypes); // externref ≠ GC types! :(
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_Annotations);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_ImportExportMutGlobals);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_MultiValue); // ✓
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_MultiMemories); // ✓ --enable-multi-memory
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_ExtendedConst); // i32.add in global's init
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_ExceptionHandling);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_TailCall);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_Memory64);
    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_FunctionReferences);// function pointers!!
    // --enable-function-reference NOT YET https://github.com/WasmEdge/WasmEdge/pull/2122
    // ⚠️ "Though the user can specify enabling the proposal, the support for the proposal is not implemented yet." :(

//    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_Threads);
//    WasmEdge_ConfigureAddProposal(conf, WasmEdge_Proposal_SIMD);


//    Proposal::ReferenceTypes
    /* The configure and store context to the VM creation can be NULL. */
    WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(conf, NULL);
//    WasmEdge_VMContext *VMCxt = WasmEdge_VMCreate(0, 0); // no wasi



    WasmEdge_Value Params[1];
    WasmEdge_Value Returns[1];
    WasmEdge_String FuncName = WasmEdge_StringCreateByCString("wasp_main");
    WasmEdge_Result Res = WasmEdge_VMRunWasmFromBuffer(VMCxt, buffer, buf_size, FuncName, Params, 0, Returns, 1);
    if (not WasmEdge_ResultOK(Res)) {
        print("Starting main() instead …");
        WasmEdge_String FuncName2 = WasmEdge_StringCreateByCString("main");
        Res = WasmEdge_VMRunWasmFromBuffer(VMCxt, buffer, buf_size, FuncName2, Params, 0, Returns, 1);
    }
    const WasmEdge_ModuleInstanceContext *module_ctx = WasmEdge_VMGetActiveModule(VMCxt);
    auto mem = WasmEdge_StringCreateByCString("memory");
    WasmEdge_MemoryInstanceContext *memory_ctx = WasmEdge_ModuleInstanceFindMemory(module_ctx, mem);
    uint8_t *memo = WasmEdge_MemoryInstanceGetPointer(memory_ctx, 0, 0);
    if (memo)
        wasm_memory = memo;
    else
        warn("⚠️Can't connect wasmedge memory");

    if (WasmEdge_ResultOK(Res)) {
//        int32_t value = WasmEdge_ValueGetI32(Returns[0]);
        int64_t value = WasmEdge_ValueGetI64(Returns[0]);
//        printf("Got result: 0x%llx\n", value);
        auto node1 = smartNode(value);
//        print(node1);
        auto smartPointer = node1->toSmartPointer();
        return smartPointer;
    } else printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
    return -1;
}

int64 run_wasm2(char *wasm_path) {
//extern "C" int64 run_wasm(char *wasm_path){
    /* Create the configure context and add the WASI support. */
    /* This step is not necessary unless you need WASI support. */
    WasmEdge_ConfigureContext *ConfCxt = WasmEdge_ConfigureCreate();
//	WasmEdge_ConfigureAddHostRegistration(ConfCxt, WasmEdge_HostRegistration_Wasi);
    /* The configure and store context to the VM creation can be NULL. */
    WasmEdge_VMContext *context = WasmEdge_VMCreate(ConfCxt, 0);

    // link other modules
    auto HostMod = CreateExternModule();
    WasmEdge_VMRegisterModuleFromImport(VMCxt, HostMod);
//	WasmEdge_ValueGenExternRef(AddFunc);

//	WasmEdge_VMGetFunctionList(context,)
//	context.
    /* The parameters and returns arrays. */
    int ParamCount = 0;
    WasmEdge_Value Params[ParamCount];// = { WasmEdge_ValueGenI32(32) };
    WasmEdge_Value Returns[1];
    /* Function name. */
    WasmEdge_String FuncName = WasmEdge_StringCreateByCString("test");
    /* Run the WASM function from file. */
    WasmEdge_Result Res = WasmEdge_VMRunWasmFromFile(context, wasm_path, FuncName, Params, ParamCount, Returns, 1);
//

    if (WasmEdge_ResultOK(Res)) {
        int32_t i = WasmEdge_ValueGetI32(Returns[0]);
        printf("Get result: %d\n", i);
        return (int) i;
    } else {
        printf("Error message: %s\n", WasmEdge_ResultGetMessage(Res));
        return -1;
    }

    /* Resources deallocations. */
    WasmEdge_VMDelete(context);
    WasmEdge_ConfigureDelete(ConfCxt);
    WasmEdge_StringDelete(FuncName);
}
