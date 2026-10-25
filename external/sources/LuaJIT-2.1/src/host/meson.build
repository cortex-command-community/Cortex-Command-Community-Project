minilua = executable('minilua', 'minilua.c', dependencies: libm, install: false, native: true)

if meson.can_run_host_binaries()
    lj_arch_run = cc.run(lj_arch_test_source)
    if not lj_arch_run.compiled() or lj_arch_run.returncode() != 0
        error('error running luajit arch testing program: ' + lj_arch_run.stderr())
    endif
    lj_arch_output = lj_arch_run.stdout()
else
    # lj_arch_test can't be run when cross compiling, so each check it makes is done by compiling against lj_arch.h for the target instead, and its output is put together from them.
    lj_arch_checks = {}
    foreach check : [
        ['x64', 'defined(LJ_TARGET_X64)'],
        ['x86', 'LJ_TARGET_X86'],
        ['arm', 'LJ_TARGET_ARM'],
        ['arm64', 'LJ_TARGET_ARM64'],
        ['ppc', 'LJ_TARGET_PPC'],
        ['ppcspe', 'LJ_TARGET_PPCSPE'],
        ['mips', 'LJ_TARGET_MIPS'],
        ['MIPSEL', 'defined(MIPSEL)'],
        ['LE', 'defined(LJ_LE)'],
        ['P64', 'LJ_ARCH_BITS == 64'],
        ['JIT', 'LJ_HASJIT == 1'],
        ['FFI', 'LJ_HASFFI == 1'],
        ['DUALNUM', 'LJ_DUALNUM == 1'],
        ['FPU', 'LJ_ARCH_HASFPU == 1'],
        ['SOFTFP', 'LJ_ABI_SOFTFP == 1'],
        ['NO_UNWIND', 'LJ_NO_UNWIND == 1'],
        ['VER', 'LJ_ARCH_VERSION'],
        ['WIN', 'defined(_WIN32)'],
        ['SSE', '!defined(LJ_TARGET_X64) && defined(LJ_TARGET_X86) && __SSE2__ == 1'],
    ]
        lj_arch_checks += {check[0]: cc.compiles('#include "lj_arch.h"\n#if !(' + check[1] + ')\n#error\n#endif\n',
            include_directories: luajit_source_dir, name: 'LuaJIT target ' + check[1])}
    endforeach

    lj_arch = ''
    foreach arch : ['x64', 'x86', 'arm', 'arm64', 'ppc', 'ppcspe', 'mips']
        if lj_arch == '' and lj_arch_checks[arch]
            lj_arch = arch
        endif
    endforeach
    if lj_arch == ''
        error('Unsupported architecture')
    endif

    lj_arch_defs = ['-DLUAJIT_TARGET=LUAJIT_ARCH_' + lj_arch]
    if lj_arch == 'mips' and lj_arch_checks['MIPSEL']
        lj_arch_defs += '-D__MIPSEL__=1'
    endif

    lj_dasm_defs = ['-D', lj_arch_checks['LE'] ? 'ENDIAN_LE' : 'ENDIAN_BE']
    foreach flag : ['P64', 'JIT', 'FFI', 'DUALNUM']
        if lj_arch_checks[flag]
            lj_dasm_defs += ['-D', flag]
        endif
    endforeach
    if lj_arch_checks['FPU']
        lj_dasm_defs += ['-D', 'FPU']
        lj_arch_defs += '-DLJ_ARCH_HASFPU=1'
    else
        lj_arch_defs += '-DLJ_ARCH_HASFPU=0'
    endif
    if lj_arch_checks['SOFTFP']
        lj_arch_defs += '-DLJ_ABI_SOFTFP=1'
    else
        lj_dasm_defs += ['-D', 'HFABI']
        lj_arch_defs += '-DLJ_ABI_SOFTFP=0'
    endif
    if lj_arch_checks['NO_UNWIND']
        lj_dasm_defs += ['-D', 'NO_UNWIND']
        lj_arch_defs += '-DLUAJIT_NO_UNWIND'
    endif
    lj_arch_version = ''
    if lj_arch_checks['VER']
        lj_arch_version = cc.compute_int('LJ_ARCH_VERSION', prefix: '#include "lj_arch.h"', include_directories: luajit_source_dir).to_string()
    endif
    lj_dasm_defs += ['-D', 'VER=' + lj_arch_version]
    foreach flag : ['WIN', 'SSE']
        if lj_arch_checks[flag]
            lj_dasm_defs += ['-D', flag]
        endif
    endforeach

    lj_arch_output = ':'.join([lj_arch, lj_arch, ','.join(lj_arch_defs), ','.join(lj_dasm_defs)])
endif
lj_arch_list = lj_arch_output.split(':')

# Print output of lj_arch_test for information purpose.
foreach line : lj_arch_list
    message(line)
endforeach

lj_arch = lj_arch_list[0]
dasm_arch = lj_arch_list[1]
buildvm_defines = lj_arch_list[2].split(',')
# buildvm is built for the machine doing the build, so if LuaJIT is being built for another OS it has to be told which, like LuaJIT's Makefile does.
if build_machine.system() != host_machine.system()
    lj_target_os = {'windows': 'LUAJIT_OS_WINDOWS', 'linux': 'LUAJIT_OS_LINUX', 'darwin': 'LUAJIT_OS_OSX'}
    buildvm_defines += '-DLUAJIT_OS=' + lj_target_os.get(host_machine.system(), 'LUAJIT_OS_OTHER')
endif

dasm_args = [minilua, dynasm]
dasm_args += lj_arch_list[3].split(',')
dasm_args += ['-o', '@OUTPUT@', '@INPUT@']

vm_dasc = files('../vm_' + dasm_arch + '.dasc')

buildvm_arch_h = custom_target('buildvm_arch.h',
    input : vm_dasc,
    output : 'buildvm_arch.h',
    command : dasm_args,
)

buildvm_sources = files('buildvm.c', 'buildvm_asm.c', 'buildvm_peobj.c', 'buildvm_lib.c', 'buildvm_fold.c')
buildvm_sources += buildvm_arch_h

buildvm = executable('buildvm',
    buildvm_sources,
    include_directories: luajit_source_dir,
    c_args: buildvm_defines,
    dependencies: libm,
    build_by_default: false,
    install: false,
    native: true
)
