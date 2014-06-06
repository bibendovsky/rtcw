function(rtcw_set_common_options)
    # ===========
    # Definitions
    # ===========

    if (MSVC)
        add_compile_options("$<$<CONFIG:DEBUG>:-D_CRT_SECURE_NO_WARNINGS>")
        add_compile_options("$<$<CONFIG:DEBUG>:-D_ITERATOR_DEBUG_LEVEL=0>")
    endif ()

    # ==============
    # Compiler flags
    # ==============

    # Common compiler flags

    if (MSVC)
        # Warning Level
        add_compile_options("-W4")

        # Multi-processor Compilation
        add_compile_options("-MP")

        # Eliminate Duplicate Strings
        add_compile_options("-GF")

        # Disable Minimal Rebuild
        add_compile_options("-Gm-")

        # Disable RTTI
        add_compile_options("-GR-")

        # Disable Precompiled Headers
        add_compile_options("-Y-")

        # No Enhanced Instructions
        add_compile_options("-arch:IA32")

        # Suppress "unreferenced formal parameter" warning
        #add_compile_options(-wd4100)

        # Suppress "The POSIX name for this item is deprecated" warning
        #add_compile_options(-wd4996)
    endif ()

    # ---------------------
    # Debug compile options
    # ---------------------

    if (MSVC)
        # Runtime Library (Multi-threaded Debug)
        add_compile_options("$<$<CONFIG:DEBUG>:-MTd>")
    endif ()


    # -----------------------
    # Release compile options
    # -----------------------

    if (MSVC)
        # Runtime Library (Multi-threaded)
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-MT>")

        # Enable Intrinsic Function
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-Oi>")

        # In-line Function Expansion (Only __inline)
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-Ob1>")

        # Favour size or speed (speed)
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-Ot>")

        # Omit Frame Pointers
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-Oy>")

        # Disable Security Check
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-GS->")

        # Disable Whole Program Optimization
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-GL->")

        # Disable Additional SDL checks
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-sdl->")

        # Enable Function-Level Linking
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-Gy->")
    endif ()
endfunction (rtcw_set_common_options)

function(rtcw_add_linker_option TARGET OPTION)
    get_target_property(LINKER_OPTIONS "${TARGET}" "LINK_FLAGS")

    if (NOT LINKER_OPTIONS)
        set(LINKER_OPTIONS "")
    endif ()

    set(LINKER_OPTIONS "${LINKER_OPTIONS} ${OPTION}")

    set_target_properties("${TARGET}" PROPERTIES LINK_FLAGS "${LINKER_OPTIONS}")
endfunction(rtcw_add_linker_option)

function(rtcw_add_linker_common_options TARGET)
    if (MINGW)
        rtcw_add_linker_option("${TARGET}" "-static")
    elseif (MSVC)
        rtcw_add_linker_option("${TARGET}" "-OPT:REF")
        rtcw_add_linker_option("${TARGET}" "-OPT:ICF")
    endif ()
endfunction(rtcw_add_linker_common_options)
