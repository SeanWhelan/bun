#pragma once

namespace Zig {
class GlobalObject;
}

#include "root.h"
#include "JavaScriptCore/JSFunction.h"
#include "JavaScriptCore/VM.h"

#include "headers-handwritten.h"
#include "BunClientData.h"
#include "WebCoreJSBuiltinInternals.h"
#include "JavaScriptCore/CallFrame.h"

namespace JSC {
class JSGlobalObject;
}

namespace Zig {

using namespace JSC;

using FFIFunction = JSC::EncodedJSValue (*)(JSC::JSGlobalObject* globalObject, JSC::CallFrame* callFrame);

class JSFFIFunction final : public JSC::JSFunction {
public:
    using Base = JSFunction;

    static constexpr unsigned StructureFlags = Base::StructureFlags;
    static constexpr bool needsDestruction = false;
    static void destroy(JSCell* cell)
    {
        static_cast<JSFFIFunction*>(cell)->JSFFIFunction::~JSFFIFunction();
    }

    template<typename, SubspaceAccess mode> static JSC::GCClient::IsoSubspace* subspaceFor(JSC::VM& vm)
    {
        if constexpr (mode == JSC::SubspaceAccess::Concurrently)
            return nullptr;
        return WebCore::subspaceForImpl<JSFFIFunction, WebCore::UseCustomHeapCellType::No>(
            vm,
            [](auto& spaces) { return spaces.m_clientSubspaceForFFIFunction.get(); },
            [](auto& spaces, auto&& space) { spaces.m_clientSubspaceForFFIFunction = WTFMove(space); },
            [](auto& spaces) { return spaces.m_subspaceForFFIFunction.get(); },
            [](auto& spaces, auto&& space) { spaces.m_subspaceForFFIFunction = WTFMove(space); });
    }

    DECLARE_EXPORT_INFO;

    JS_EXPORT_PRIVATE static JSFFIFunction* create(VM&, Zig::GlobalObject*, unsigned length, const String& name, FFIFunction, Intrinsic = NoIntrinsic, NativeFunction nativeConstructor = callHostFunctionAsConstructor);

    static Structure* createStructure(VM& vm, JSGlobalObject* globalObject, JSValue prototype)
    {
        ASSERT(globalObject);
        return Structure::create(vm, globalObject, prototype, TypeInfo(JSFunctionType, StructureFlags), info());
    }

    const FFIFunction function() { return m_function; }

private:
    JSFFIFunction(VM&, NativeExecutable*, JSGlobalObject*, Structure*, FFIFunction&&);
    void finishCreation(VM&, NativeExecutable*, unsigned length, const String& name);
    DECLARE_VISIT_CHILDREN;

    FFIFunction m_function;
};

} // namespace JSC

extern "C" Zig::JSFFIFunction* Bun__CreateFFIFunction(Zig::GlobalObject* globalObject, const ZigString* symbolName, unsigned argCount, Zig::FFIFunction functionPointer);