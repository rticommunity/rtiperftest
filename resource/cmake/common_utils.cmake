
# This funtion returns the directory name where the Core library can be found
# based on the provided <full_arch_name>.
# The function produces <core_lib_dir_name> output containing the Core's
# directory name
function(get_core_lib_dir_name full_arch_name)
    string(REPLACE "-" ";" split_arch_name ${full_arch_name})
    list(GET split_arch_name 0 core_lib_dir_name)
    list(LENGTH split_arch_name split_arch_name_length)
    # Set function outputs
    set(core_lib_dir_name ${core_lib_dir_name} PARENT_SCOPE)
    # If there's text following the "-" in the arch name, we are working with a
    # Platform Independent architecture, hence a Platform Independent Cert
    if(split_arch_name_length GREATER 1)
        set(arch_is_pi ON PARENT_SCOPE)
    else()
        set(arch_is_pi OFF PARENT_SCOPE)
    endif()
endfunction()