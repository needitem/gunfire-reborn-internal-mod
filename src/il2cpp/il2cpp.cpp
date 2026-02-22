#include "il2cpp.h"

// Global function pointers
il2cpp_domain_get_t il2cpp_domain_get = nullptr;
il2cpp_domain_get_assemblies_t il2cpp_domain_get_assemblies = nullptr;
il2cpp_assembly_get_image_t il2cpp_assembly_get_image = nullptr;
il2cpp_image_get_name_t il2cpp_image_get_name = nullptr;
il2cpp_class_from_name_t il2cpp_class_from_name = nullptr;
il2cpp_class_get_method_from_name_t il2cpp_class_get_method_from_name = nullptr;
il2cpp_runtime_invoke_t il2cpp_runtime_invoke = nullptr;
il2cpp_object_unbox_t il2cpp_object_unbox = nullptr;
il2cpp_class_get_field_from_name_t il2cpp_class_get_field_from_name = nullptr;
il2cpp_field_static_get_value_t il2cpp_field_static_get_value = nullptr;
il2cpp_thread_attach_t il2cpp_thread_attach = nullptr;
il2cpp_thread_detach_t il2cpp_thread_detach = nullptr;
il2cpp_method_get_pointer_t il2cpp_method_get_pointer = nullptr;
il2cpp_string_new_t il2cpp_string_new = nullptr;
il2cpp_object_new_t il2cpp_object_new = nullptr;
il2cpp_class_get_parent_t il2cpp_class_get_parent = nullptr;
il2cpp_field_static_set_value_t il2cpp_field_static_set_value = nullptr;
il2cpp_class_get_name_t il2cpp_class_get_name = nullptr;
il2cpp_class_get_namespace_t il2cpp_class_get_namespace = nullptr;
il2cpp_class_get_fields_t il2cpp_class_get_fields = nullptr;
il2cpp_field_get_name_t il2cpp_field_get_name = nullptr;
il2cpp_field_get_type_t il2cpp_field_get_type = nullptr;
il2cpp_type_get_name_t il2cpp_type_get_name = nullptr;
il2cpp_field_get_offset_t il2cpp_field_get_offset = nullptr;
il2cpp_class_get_methods_t il2cpp_class_get_methods = nullptr;
il2cpp_method_get_name_t il2cpp_method_get_name = nullptr;
il2cpp_method_get_return_type_t il2cpp_method_get_return_type = nullptr;
il2cpp_method_get_param_count_t il2cpp_method_get_param_count = nullptr;
il2cpp_method_get_param_t il2cpp_method_get_param = nullptr;
il2cpp_image_get_class_count_t il2cpp_image_get_class_count = nullptr;
il2cpp_image_get_class_t il2cpp_image_get_class = nullptr;

HMODULE g_GameAssembly = nullptr;

Il2CppImage* FindImage(const char* name) {
    auto domain = il2cpp_domain_get();
    if (!domain) return nullptr;
    
    size_t count = 0;
    auto assemblies = il2cpp_domain_get_assemblies(domain, &count);
    
    for (size_t i = 0; i < count; i++) {
        auto img = il2cpp_assembly_get_image(assemblies[i]);
        auto imgName = il2cpp_image_get_name(img);
        if (imgName && strstr(imgName, name)) return img;
    }
    return nullptr;
}

bool InitIL2CPP() {
    g_GameAssembly = GetModuleHandleA("GameAssembly.dll");
    if (!g_GameAssembly) return false;
    
    il2cpp_domain_get = (il2cpp_domain_get_t)GetProcAddress(g_GameAssembly, "il2cpp_domain_get");
    il2cpp_domain_get_assemblies = (il2cpp_domain_get_assemblies_t)GetProcAddress(g_GameAssembly, "il2cpp_domain_get_assemblies");
    il2cpp_assembly_get_image = (il2cpp_assembly_get_image_t)GetProcAddress(g_GameAssembly, "il2cpp_assembly_get_image");
    il2cpp_image_get_name = (il2cpp_image_get_name_t)GetProcAddress(g_GameAssembly, "il2cpp_image_get_name");
    il2cpp_class_from_name = (il2cpp_class_from_name_t)GetProcAddress(g_GameAssembly, "il2cpp_class_from_name");
    il2cpp_class_get_method_from_name = (il2cpp_class_get_method_from_name_t)GetProcAddress(g_GameAssembly, "il2cpp_class_get_method_from_name");
    il2cpp_runtime_invoke = (il2cpp_runtime_invoke_t)GetProcAddress(g_GameAssembly, "il2cpp_runtime_invoke");
    il2cpp_object_unbox = (il2cpp_object_unbox_t)GetProcAddress(g_GameAssembly, "il2cpp_object_unbox");
    il2cpp_class_get_field_from_name = (il2cpp_class_get_field_from_name_t)GetProcAddress(g_GameAssembly, "il2cpp_class_get_field_from_name");
    il2cpp_field_static_get_value = (il2cpp_field_static_get_value_t)GetProcAddress(g_GameAssembly, "il2cpp_field_static_get_value");
    il2cpp_thread_attach = (il2cpp_thread_attach_t)GetProcAddress(g_GameAssembly, "il2cpp_thread_attach");
    il2cpp_thread_detach = (il2cpp_thread_detach_t)GetProcAddress(g_GameAssembly, "il2cpp_thread_detach");
    il2cpp_method_get_pointer = (il2cpp_method_get_pointer_t)GetProcAddress(g_GameAssembly, "il2cpp_method_get_pointer");
    il2cpp_string_new = (il2cpp_string_new_t)GetProcAddress(g_GameAssembly, "il2cpp_string_new");
    il2cpp_object_new = (il2cpp_object_new_t)GetProcAddress(g_GameAssembly, "il2cpp_object_new");
    il2cpp_class_get_parent = (il2cpp_class_get_parent_t)GetProcAddress(g_GameAssembly, "il2cpp_class_get_parent");
    il2cpp_field_static_set_value = (il2cpp_field_static_set_value_t)GetProcAddress(g_GameAssembly, "il2cpp_field_static_set_value");
    il2cpp_class_get_name = (il2cpp_class_get_name_t)GetProcAddress(g_GameAssembly, "il2cpp_class_get_name");
    il2cpp_class_get_namespace = (il2cpp_class_get_namespace_t)GetProcAddress(g_GameAssembly, "il2cpp_class_get_namespace");
    il2cpp_class_get_fields = (il2cpp_class_get_fields_t)GetProcAddress(g_GameAssembly, "il2cpp_class_get_fields");
    il2cpp_field_get_name = (il2cpp_field_get_name_t)GetProcAddress(g_GameAssembly, "il2cpp_field_get_name");
    il2cpp_field_get_type = (il2cpp_field_get_type_t)GetProcAddress(g_GameAssembly, "il2cpp_field_get_type");
    il2cpp_type_get_name = (il2cpp_type_get_name_t)GetProcAddress(g_GameAssembly, "il2cpp_type_get_name");
    il2cpp_field_get_offset = (il2cpp_field_get_offset_t)GetProcAddress(g_GameAssembly, "il2cpp_field_get_offset");
    il2cpp_class_get_methods = (il2cpp_class_get_methods_t)GetProcAddress(g_GameAssembly, "il2cpp_class_get_methods");
    il2cpp_method_get_name = (il2cpp_method_get_name_t)GetProcAddress(g_GameAssembly, "il2cpp_method_get_name");
    il2cpp_method_get_return_type = (il2cpp_method_get_return_type_t)GetProcAddress(g_GameAssembly, "il2cpp_method_get_return_type");
    il2cpp_method_get_param_count = (il2cpp_method_get_param_count_t)GetProcAddress(g_GameAssembly, "il2cpp_method_get_param_count");
    il2cpp_method_get_param = (il2cpp_method_get_param_t)GetProcAddress(g_GameAssembly, "il2cpp_method_get_param");
    il2cpp_image_get_class_count = (il2cpp_image_get_class_count_t)GetProcAddress(g_GameAssembly, "il2cpp_image_get_class_count");
    il2cpp_image_get_class = (il2cpp_image_get_class_t)GetProcAddress(g_GameAssembly, "il2cpp_image_get_class");

    return il2cpp_domain_get && il2cpp_runtime_invoke && il2cpp_thread_attach;
}
