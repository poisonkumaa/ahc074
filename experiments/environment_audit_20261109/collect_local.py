"""Read-only environment inventory. Captures only solver-relevant environment keys."""
from pathlib import Path
import hashlib
import json
import os
import re
import shutil
import subprocess

HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[1]
report={}
def command(label,args,timeout=30):
    try:
        p=subprocess.run(args,cwd=ROOT,capture_output=True,text=True,timeout=timeout)
        report[label]={'command':args,'returncode':p.returncode,'stdout':p.stdout,'stderr':p.stderr}
    except Exception as e:
        report[label]={'command':args,'error':str(e)}
def read(label,path):
    try: report[label]=Path(path).read_text()
    except Exception as e: report[label]={'error':str(e)}

for label,args in [
 ('uname',['uname','-srmv']),('cpu',['lscpu']),('memory',['free','-b']),
 ('gcc_version',['g++','--version']),('gcc_details',['g++','-v']),
 ('gcc_target',['g++','-dumpmachine']),('gcc_libstdcpp',['g++','-print-file-name=libstdc++.so']),
 ('glibc',['getconf','GNU_LIBC_VERSION']),('linker',['ld','--version']),
 ('packages',['dpkg-query','-W','libc6','libstdc++6','libgcc-s1','g++-11','gcc-11','binutils']),
 ('pahcer_version',['pahcer','--version']),('pahcer_help',['pahcer','run','--help']),
 ('cargo_version',['cargo','--version']),('rustc_version',['rustc','-vV']),
 ('python_version',['python3','--version']),
 ('current_binary',['file','a.out','tools/target/release/tester']),
 ('binary_compiler',['readelf','-p','.comment','a.out']),
 ('binary_needed',['readelf','-d','a.out']),('binary_ldd',['ldd','a.out']),
 ('cpu_affinity',['taskset','-pc',str(os.getpid())]),
 ('native_target_flags',['g++','-std=c++20','-O2','-march=native','-Q','--help=target','-x','c++','-c','/dev/null','-o','/dev/null']),
 ('generic_target_flags',['g++','-std=c++20','-O2','-Q','--help=target','-x','c++','-c','/dev/null','-o','/dev/null']),
 ('native_macros',['g++','-std=c++20','-O2','-march=native','-dM','-E','-x','c++','/dev/null']),
]: command(label,args)
for level in ['1','2','3']:
    command('optimizer_O'+level,['g++','-O'+level,'-march=native','-Q','--help=optimizers','-x','c++','-c','/dev/null','-o','/dev/null'])
for label,path in [
 ('os_release','/etc/os-release'),('process_limits','/proc/self/limits'),
 ('cgroup_cpu','/sys/fs/cgroup/cpu.max'),('cgroup_memory','/sys/fs/cgroup/memory.max'),
 ('cpu_governor','/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor'),
 ('cpu_scaling_max','/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq'),
 ('loadavg','/proc/loadavg'),('clocksource','/sys/devices/system/clocksource/clocksource0/current_clocksource'),
 ]: read(label,path)
report['executables']={name:shutil.which(name) for name in ['g++','gcc','clang++','pahcer','cargo','rustc','python3','docker','podman']}
source_bytes=(ROOT/'main.cpp').read_bytes()
text=source_bytes.decode('utf-8')
(HERE/'audited_main.cpp').write_bytes(source_bytes)
envkeys=sorted(set(re.findall(r'(?:getenv|(?:search_budget::)?(?:real|integer))\("([A-Z0-9_]+)"',text)))
report['solver_environment']={key:os.environ[key] for key in envkeys if key in os.environ}
report['runtime_environment']={key:os.environ[key] for key in ['LD_PRELOAD','LD_LIBRARY_PATH','OMP_NUM_THREADS','OPENBLAS_NUM_THREADS','MKL_NUM_THREADS','RAYON_NUM_THREADS','CARGO_BUILD_JOBS'] if key in os.environ}
report['solver_environment_keys']=envkeys
report['files']={}
for name in ['main.cpp','pahcer_config.toml','a.out','tools/target/release/tester','tools/src/bin/tester.rs','tools/src/lib.rs','tools/Cargo.toml','tools/Cargo.lock','tools/in/0000.txt']:
    p=ROOT/name
    report['files'][name]={'size':p.stat().st_size,'mtime':p.stat().st_mtime,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
report['input_files']=len(list((ROOT/'tools/in').glob('*.txt')))
report['audited_source_sha256']=hashlib.sha256(source_bytes).hexdigest()
(HERE/'local_inventory.json').write_text(json.dumps(report,ensure_ascii=False,indent=2))
summary={k:report[k] for k in ['executables','gcc_version','glibc','pahcer_version','cargo_version','rustc_version','os_release','uname','solver_environment','runtime_environment','cgroup_cpu','cgroup_memory','cpu_governor','input_files']}
print(json.dumps(summary,ensure_ascii=False,indent=2))
