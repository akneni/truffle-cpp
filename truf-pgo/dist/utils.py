import hashlib
import subprocess
from dataclasses import dataclass
import platform
from datetime import datetime
import random

@dataclass
class SystemSpec:
    os: str
    cpu: str
    compiler: str
    os_version: str = None
    cpu_arch: str = None
    cpu_physical_cores: int = None
    cpu_logical_cores: int = None
    mem_size_gb: float = None
    mem_speed_mhz: int = None
    l1_i_size: str = None
    l1_d_size: str = None
    l2_size: str = None
    l3_size: str = None

    @staticmethod
    def generate():
        spec = SystemSpec(
            os=platform.system(),
            os_version=platform.platform(),
            cpu_arch=platform.machine(),
            **SystemSpec.compiler_specs(),
            **SystemSpec.cpu_specs(),
            **SystemSpec.mem_specs(),
        )
        return spec

    @staticmethod
    def compiler_specs() -> dict:
        specs = {'compiler': 'gcc (unknown version)'}
        try:
            result = subprocess.run(["gcc", "--version"], stdout=subprocess.PIPE)
            try:
                line = result.stdout.decode('utf-8').strip().split('\n')[0].strip()
                specs['compiler'] = line
            except Exception:
                pass
        except Exception:
            pass

        return specs

    @staticmethod
    def cpu_specs():
        specs = {}
        try:
            result = subprocess.run(["lscpu"], stdout=subprocess.PIPE)
            # Extract speed from the output
            for line in result.stdout.decode('utf-8').split('\n'):
                try:
                    line = line.strip()
                    parser_rules = [
                        # [starts_with, specs key, type],
                        ['CPU(s):', 'cpu_logical_cores', int],
                        ['Model name:', 'cpu', str],
                        ['Core(s) per socket:', 'cpu_physical_cores', int],
                        ['L1d', 'l1_i_size', str],
                        ['L1i', 'l1_d_size', str],
                        ['L2', 'l2_size', str],
                        ['L3', 'l3_size', str],
                    ]

                    for start, key, type_f, in parser_rules:
                        if line.startswith(start):
                            line = line.partition(':')[2].strip()
                            line = type_f(line)
                            specs[key] = line
                            break
                except Exception:
                    pass
            
        except Exception:
            pass

        return specs
    
    @staticmethod
    def mem_specs():
        specs = {}
        try:
            result = subprocess.run(["cat", "/proc/meminfo"], stdout=subprocess.PIPE)
            # Extract speed from the output
            for line in result.stdout.decode('utf-8').split('\n'):
                line = line.strip()
                if line.startswith('MemTotal:'):
                    line = line.partition(':')[2].strip()
                    mem_size, _, units = line.partition(' ')
                    mem_size = float(mem_size)

                    if units.lower() == 'kb':
                        mem_size /= 1_000_000
                    elif units.lower() == 'mb':
                        mem_size /= 1_000
                    specs['mem_size_gb'] = mem_size
                    break
        except Exception:
            pass

        return specs

@dataclass
class ExecutionTrial:
    c_filename: str
    c_source: str
    flags: list[str]
    output: str
    system_specs: SystemSpec
    created_at: datetime

    def gen_uid(self) -> str:
        uid = self.c_filename + "".join(self.flags)
        uid = hashlib.sha256(uid.encode('utf-8')).hexdigest()
        rand = hashlib.sha256(random.randbytes(128)).hexdigest()
        return f"{uid}-{rand}"
    
    def time_elapsed(self) -> float:
        output = self.output.strip()
        time_elapsed = output.split('\n')[-1]
        return float(time_elapsed.partition(':')[2].strip())
