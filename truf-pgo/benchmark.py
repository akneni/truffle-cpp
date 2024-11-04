import hashlib
import os
import subprocess
import dataclasses
from dataclasses import dataclass
import shelve
import platform
from datetime import datetime
import random
import sys
import pymongo

SHELVE_PATH: str = "/docker-volume/benchmark.shelve"
MONGO_KEY: str = os.environ['MONGO_KEY']

@dataclass
class SystemSpec:
    os: str
    cpu: str
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
            **SystemSpec.cpu_specs(),
            **SystemSpec.mem_specs(),
        )
        return spec

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


def benchmark_gcc_flags():
    flags = [
        "-O3",
        "-O3 -funroll-loops",
        "-O3 -fprefetch-loop-arrays",
        "-O3 -flto",
        "-O3 -ffast-math",
        "-O3 -fno-math-errno",
        "-O3 -funsafe-math-optimizations",
        "-O3 -march=native",
        "-O3 -mtune=native",
        "-O3 -fomit-frame-pointer",
        "-O3 -fwhole-program",        
    ]
    flags = [
        [f, hashlib.sha256(f.encode('ascii')).hexdigest()[:8]]
        for f in flags
    ]

    # client = pymongo.MongoClient(MONGO_KEY)
    # collection = client.get_database('truffle').get_collection('truffle-pgo-benchmarking')

    for c_file in os.listdir("C-DATASET"):
        c_filepath = os.path.join("C-DATASET", c_file)
        
        links = ""
        with open(c_filepath, 'r') as f:
            line = f.readline()
            if line.startswith("// LINK:") or line.startswith("//LINK:"):
                dynamic_links = line.partition(':')[2].strip()
                links = f" {dynamic_links}"
        
        child_p = []
        for (flag, hash) in flags:
            output_filename = f"c-benchmark-tester-{c_file}-{hash}.exe"
            cmd = f"gcc {flag} -o {output_filename} {c_filepath}{links}"
            child = subprocess.Popen(cmd.split(), stdout=subprocess.DEVNULL)
            child_p.append(child)
        
        system_specs = SystemSpec.generate()

        for child in child_p:
            child.wait()
        print(f"Finished compiling {c_file}")
        print('_'*40)

        num_trials = 1
        argv = [i for i in sys.argv if i.startswith('--trials')]
        if len(argv) > 0:
            num_trials = int(argv[0].partition('=')[2])

        for _ in range(num_trials):
            for (flag, hash) in flags:
                try:
                    output_filename = f"c-benchmark-tester-{c_file}-{hash}.exe"
                    stdout = subprocess.run(f"./{output_filename}", stdout=subprocess.PIPE)

                    with open(c_filepath, 'r') as f:
                        c_source = f.read()

                    record = ExecutionTrial(
                        c_filename=c_file,
                        c_source=c_source,
                        flags=flag.strip().split(),
                        output=stdout.stdout.decode("utf-8"),
                        system_specs=system_specs,
                        created_at=datetime.now()
                    )

                    record = dataclasses.asdict(record)

                    with shelve.open(SHELVE_PATH) as db:
                        db[record.gen_uid()] = record

                    # collection.insert_one(record)
                    
                    print(f"Completed benchmarking [{c_file:<15}] ( {flag:<30} )")
                except Exception as err:
                    print(f"Error running running exe: {err}")
        print('\n'*3)

if __name__ == '__main__':
    benchmark_gcc_flags()