lj_arch_test_source = files('lj_arch_test.c')

if get_option('portable')
    jitlib_install_dir = get_option('bindir') / 'lua' / 'jit'
else
    jitlib_install_dir = 'share' / ( 'luajit-' + meson.project_version() ) / 'jit'
endif

subdir('host')

ljlib_sources = files(
    'lib_base.c', 'lib_math.c', 'lib_bit.c', 'lib_string.c', 'lib_table.c',
    'lib_io.c', 'lib_os.c', 'lib_package.c', 'lib_debug.c', 'lib_jit.c', 'lib_ffi.c',
    'lib_buffer.c')
ljcore_sources = files(
    'lj_assert.c', 'lj_gc.c', 'lj_err.c', 'lj_char.c', 'lj_bc.c', 'lj_obj.c', 'lj_buf.c',
    'lj_str.c', 'lj_tab.c', 'lj_func.c', 'lj_udata.c', 'lj_meta.c', 'lj_debug.c',
    'lj_prng.c', 'lj_state.c', 'lj_dispatch.c', 'lj_vmevent.c', 'lj_vmmath.c',
    'lj_strscan.c', 'lj_strfmt.c', 'lj_strfmt_num.c', 'lj_serialize.c',
    'lj_api.c', 'lj_profile.c',
    'lj_lex.c', 'lj_parse.c', 'lj_bcread.c', 'lj_bcwrite.c', 'lj_load.c',
    'lj_ir.c', 'lj_opt_mem.c', 'lj_opt_fold.c', 'lj_opt_narrow.c',
    'lj_opt_dce.c', 'lj_opt_loop.c', 'lj_opt_split.c', 'lj_opt_sink.c',
    'lj_mcode.c', 'lj_snap.c', 'lj_record.c', 'lj_crecord.c', 'lj_ffrecord.c',
    'lj_asm.c', 'lj_trace.c', 'lj_gdbjit.c',
    'lj_ctype.c', 'lj_cdata.c', 'lj_cconv.c', 'lj_ccall.c', 'lj_ccallback.c',
    'lj_carith.c', 'lj_clib.c', 'lj_cparse.c',
    'lj_lib.c', 'lj_alloc.c', 'lib_aux.c',
    'lib_init.c')
ljmain_sources = files('luajit.c')

buildvm_commands = [
    {'mode': 'ffdef',   'output': 'lj_ffdef.h',   'input': ljlib_sources},
    {'mode': 'bcdef',   'output': 'lj_bcdef.h',   'input': ljlib_sources},
    {'mode': 'folddef', 'output': 'lj_folddef.h', 'input': 'lj_opt_fold.c'},
    {'mode': 'recdef',  'output': 'lj_recdef.h',  'input': ljlib_sources},
    {'mode': 'libdef',  'output': 'lj_libdef.h',  'input': ljlib_sources},
]

lj_defines = [
    '-D_FILE_OFFSET_BITS=64',
    '-D_LARGEFILE_SOURCE',
    '-U_FORTIFY_SOURCE',
    '-DLUA_MULTILIB="lib"',
    '-DLUA_ROOT="' + get_option('prefix') + '"'
]

if host_machine.system() == 'darwin'
    lj_defines += '-DLUAJIT_UNWIND_EXTERNAL'
elif host_machine.system() != 'windows'
    if 'NO_UNWIND' not in dasm_args
        test_unwind = run_command('test-unwind.sh', ' '.join(cc.cmd_array()), check : false)
        if test_unwind.stdout().strip() == 'E'
            lj_defines += '-DLUAJIT_UNWIND_EXTERNAL'
        endif
    endif
endif

if get_option('portable')
    lj_defines += '-DLUAJIT_PORTABLE_INSTALL'
endif

lj_linkargs = []
if host_machine.system() == 'windows'
    ljcore_sources += custom_target('lj_vm.obj',
        input : [],
        output : 'lj_vm.obj',
        command : [buildvm, '-m', 'peobj', '-o', '@OUTPUT@']
    )

    if get_option('default_library') != 'static'
        lj_defines += '-DLUA_BUILD_AS_DLL'
    endif
    lj_libname = 'lua' + luajit_abiver
    lj_libprefix = ''
elif host_machine.system() == 'darwin'
    ljcore_sources += custom_target('lj_vm.s',
        input : [],
        output : 'lj_vm.s',
        command : [buildvm, '-m', 'machasm', '-o', '@OUTPUT@']
    )
    lj_libname = 'luajit'
    lj_libprefix = 'lib'
else
    ljcore_sources += custom_target('lj_vm.s',
        input : [],
        output : 'lj_vm.s',
        command : [buildvm, '-m', 'elfasm', '-o', '@OUTPUT@']
    )
    lj_libname = 'luajit'
    lj_libprefix = 'lib'
endif

buildvm_headers = []
foreach target : buildvm_commands
    buildvm_headers += custom_target(target['output'],
        input : target['input'],
        output : target['output'],
        command : [buildvm, '-m', target['mode'], '-o', '@OUTPUT@', '@INPUT@']
    )
endforeach

vmdef = custom_target('vmdef.lua',
    input : ljlib_sources,
    output : 'vmdef.lua',
    build_by_default : false,
    install: false,
    install_dir: jitlib_install_dir,
    command : [buildvm, '-m', 'vmdef', '-o', '@OUTPUT@', '@INPUT@']
)


subdir('jit')
