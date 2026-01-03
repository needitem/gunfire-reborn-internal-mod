#pragma once
#include <Windows.h>

// IL2CPP types
typedef void* Il2CppObject;
typedef void* Il2CppClass;
typedef void* Il2CppMethod;
typedef void* Il2CppDomain;
typedef void* Il2CppAssembly;
typedef void* Il2CppImage;

// IL2CPP function pointer types
typedef Il2CppDomain* (*il2cpp_domain_get_t)();
typedef Il2CppAssembly** (*il2cpp_domain_get_assemblies_t)(Il2CppDomain*, size_t*);
typedef Il2CppImage* (*il2cpp_assembly_get_image_t)(Il2CppAssembly*);
typedef const char* (*il2cpp_image_get_name_t)(Il2CppImage*);
typedef Il2CppClass* (*il2cpp_class_from_name_t)(Il2CppImage*, const char*, const char*);
typedef Il2CppMethod* (*il2cpp_class_get_method_from_name_t)(Il2CppClass*, const char*, int);
typedef Il2CppObject* (*il2cpp_runtime_invoke_t)(Il2CppMethod*, void*, void**, void**);
typedef void* (*il2cpp_object_unbox_t)(Il2CppObject*);
typedef void* (*il2cpp_class_get_field_from_name_t)(Il2CppClass*, const char*);
typedef void (*il2cpp_field_static_get_value_t)(void*, void*);
typedef void (*il2cpp_thread_attach_t)(Il2CppDomain*);
typedef void (*il2cpp_thread_detach_t)(void*);
typedef void* (*il2cpp_method_get_pointer_t)(Il2CppMethod*);
typedef Il2CppObject* (*il2cpp_string_new_t)(const char*);

// Global IL2CPP function pointers
extern il2cpp_domain_get_t il2cpp_domain_get;
extern il2cpp_domain_get_assemblies_t il2cpp_domain_get_assemblies;
extern il2cpp_assembly_get_image_t il2cpp_assembly_get_image;
extern il2cpp_image_get_name_t il2cpp_image_get_name;
extern il2cpp_class_from_name_t il2cpp_class_from_name;
extern il2cpp_class_get_method_from_name_t il2cpp_class_get_method_from_name;
extern il2cpp_runtime_invoke_t il2cpp_runtime_invoke;
extern il2cpp_object_unbox_t il2cpp_object_unbox;
extern il2cpp_class_get_field_from_name_t il2cpp_class_get_field_from_name;
extern il2cpp_field_static_get_value_t il2cpp_field_static_get_value;
extern il2cpp_thread_attach_t il2cpp_thread_attach;
extern il2cpp_thread_detach_t il2cpp_thread_detach;
extern il2cpp_method_get_pointer_t il2cpp_method_get_pointer;
extern il2cpp_string_new_t il2cpp_string_new;

extern HMODULE g_GameAssembly;

// Helper functions
Il2CppImage* FindImage(const char* name);
bool InitIL2CPP();
