#!/usr/bin/env python3
"""构建前执行：修复 devkitA64 (gcc 15+) 下 libnx 的 PIE 链接问题（幂等）。

新版工具链下，libnx switch.specs 里的 -pie/--spare-dynamic-tags=0/
--pack-relative-relocs 会生成损坏的 ELF（entry=0、只读段动态重定位），
导致 NRO/OVL/SYSMODULE 退出时大气层崩溃。本脚本做两件事：

  1. 就地修复 $(DEVKITPRO)/libnx/switch.specs 与 switch.ld
     - specs: 整体替换为已验证的无 PIE 版本
     - ld:    在 .dynamic 段前补 PROVIDE_HIDDEN( _DYNAMIC = . )
               （静态链接无 .dynamic 段，libnx crt0 需要该符号）
  2. 删除本项目内所有 Makefile 中 ARCH 行的 -fPIE（临时改工作区，不提交）

用法：make 构建前自动调用（根 Makefile 的 order-only 前置），也可手动运行。
"""
import os

ROOT = os.path.dirname(os.path.abspath(__file__))
DKP = os.environ.get('DEVKITPRO', '/opt/devkitpro')

# 已在 vgedit 上验证可产出正常 NRO（type=EXEC, entry 正确）的 specs 内容
SPECS_FIXED = (
    "*link:\n"
    "+ -T %:getenv(DEVKITPRO /libnx/switch.ld) --no-dynamic-linker "
    "--gc-sections -z text -z now -z nodynamic-undefined-weak "
    "--build-id=sha1 --nx-module-name\n\n"
    "*startfile:\n"
    "crti%O%s crtbegin%O%s --require-defined=main\n"
)


def fix_libnx():
    specs = os.path.join(DKP, 'libnx', 'switch.specs')
    if os.path.isfile(specs):
        with open(specs) as f:
            cur = f.read()
        if cur != SPECS_FIXED:
            with open(specs, 'w') as f:
                f.write(SPECS_FIXED)

    ld = os.path.join(DKP, 'libnx', 'switch.ld')
    if os.path.isfile(ld):
        with open(ld) as f:
            src = f.read()
        anchor = '\t.dynamic : {'
        if '_DYNAMIC =' not in src and anchor in src:
            src = src.replace(anchor,
                              '\tPROVIDE_HIDDEN( _DYNAMIC = . );\n' + anchor, 1)
            with open(ld, 'w') as f:
                f.write(src)


def strip_fpie(makefile):
    if not os.path.isfile(makefile):
        return False
    with open(makefile) as f:
        src = f.read()
    out = src.replace(' -fPIE', '')
    if out != src:
        with open(makefile, 'w') as f:
            f.write(out)
        return True
    return False


def main():
    fix_libnx()
    count = 0
    for dirpath, dirs, files in os.walk(ROOT):
        dirs[:] = [x for x in dirs if x not in ('.git', 'build', 'build_nro')]
        for fn in files:
            if fn == 'Makefile':
                mf = os.path.join(dirpath, fn)
                try:
                    if strip_fpie(mf):
                        count += 1
                        print(f"  [fix_pie] stripped -fPIE: {os.path.relpath(mf, ROOT)}")
                except PermissionError:
                    pass
    print(f"[fix_pie] done ({count} makefile(s) fixed)")


if __name__ == '__main__':
    main()
