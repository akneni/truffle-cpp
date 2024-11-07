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
from utils import ExecutionTrial, SystemSpec
# import pymongo

SHELVE_PATH: str = "./benchmarking-dataset/benchmark.shelve"
# MONGO_KEY: str = os.environ['MONGO_KEY']

def benchmark_gcc_flags():
    flags = [
        # Control
        "-O3",

        # Base optimization flags
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
        "-O3 -falign-loops=32",
        "-O3 -falign-functions=32",

        # Combinations of optimization flags
        "-O3 -funroll-loops -flto",                              
        "-O3 -march=native -mtune=native -flto",                 
        "-O3 -funroll-loops -fomit-frame-pointer -flto",         
        "-O3 -ffast-math -fno-math-errno -funsafe-math-optimizations",  
        "-O3 -march=native -funroll-loops -ffast-math",          
        "-O3 -march=native -fomit-frame-pointer -flto",          
        "-O3 -fwhole-program -flto -march=native",               
        "-O3 -funroll-loops -funsafe-math-optimizations -flto",  
        "-O3 -funroll-loops -fno-math-errno -march=native",
        "-O3 -fwhole-program -funroll-loops -march=native -ffast-math", 

        # GPT suggested combinations
        "-O3 -march=native -mtune=native -funsafe-math-optimizations",
        "-O3 -flto -fomit-frame-pointer -fno-math-errno",
        "-O3 -funroll-loops -march=native -fwhole-program -flto",
        "-O3 -mtune=native -funroll-loops -funsafe-math-optimizations",
        "-O3 -funroll-loops -fprefetch-loop-arrays -flto",
        "-O3 -ffast-math -funroll-loops -fomit-frame-pointer -flto",
        "-O3 -march=native -fwhole-program -funsafe-math-optimizations",
        "-O3 -march=native -funroll-loops -funsafe-math-optimizations -ffast-math",
        "-O3 -fwhole-program -flto -fomit-frame-pointer -funsafe-math-optimizations",
        "-O3 -mtune=native -funroll-loops -fwhole-program -flto",
        "-O3 -fno-math-errno -funsafe-math-optimizations -march=native -funroll-loops",
        "-O3 -march=native -mtune=native -flto -fwhole-program",
        "-O3 -funsafe-math-optimizations -fomit-frame-pointer -funroll-loops -flto",
        "-O3 -ffast-math -funroll-loops -fwhole-program -march=native",
        "-O3 -fprefetch-loop-arrays -funroll-loops -flto",
        "-O3 -funroll-loops -march=native -mtune=native -fno-math-errno -ffast-math",
        "-O3 -funroll-loops -fprefetch-loop-arrays -march=native -ffast-math",
        "-O3 -flto -fwhole-program -march=native -funroll-loops",
        "-O3 -ffast-math -fno-math-errno -fomit-frame-pointer -flto",
        "-O3 -march=native -fomit-frame-pointer -fwhole-program -flto",
        "-O3 -funroll-loops -fprefetch-loop-arrays -march=native -flto",

        # Claude Suggested Optimizations
        "-O3 -ftree-vectorize",
        "-O3 -fipa-pta",

        # Maximum Optimizations
        "-O3 -march=native -mtune=native -funroll-loops -fprefetch-loop-arrays -flto -ffast-math -fno-math-errno -funsafe-math-optimizations -fomit-frame-pointer -fwhole-program -fmerge-all-constants -fipa-pta -falign-functions=32 -falign-loops=32",
    ]
    flags = [
        [f, hashlib.sha256(f.encode('ascii')).hexdigest()[:8]]
        for f in flags
    ]

    # client = pymongo.MongoClient(MONGO_KEY)
    # collection = client.get_database('truffle').get_collection('truffle-pgo-benchmarking')

    for c_file in os.listdir("C-dataset"):
        c_filepath = os.path.join("C-dataset", c_file)
        
        links = ""
        with open(c_filepath, 'r') as f:
            line = f.readline()
            if line.startswith("// LINK:") or line.startswith("//LINK:"):
                dynamic_links = line.partition(':')[2].strip()
                links = f" {dynamic_links}"
        
        child_p = []
        for (flag, hash) in flags:
            try:
                output_filename = f"c-benchmark-tester-{c_file}-{hash}.exe"
                cmd = f"gcc {flag} -o {output_filename} {c_filepath}{links}"
                child = subprocess.Popen(cmd.split(), stdout=subprocess.DEVNULL)
                child_p.append(child)
            except Exception as err:
                print(f"ERROR: {err}")
        
        system_specs = SystemSpec.generate()

        for child in child_p:
            try:
                child.wait()
            except Exception as err:
                print(f"ERROR: {err}")
        
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

                    record_dict = dataclasses.asdict(record)

                    with shelve.open(SHELVE_PATH) as db:
                        db[record.gen_uid()] = record_dict

                    # collection.insert_one(record_dict)
                    
                    print(f"Completed benchmarking [{c_file:<15}] ( {flag:<40} )  {record.time_elapsed()}")
                except Exception as err:
                    print(f"Error running running exe: {err}")
        print('\n'*3)
        for (flag, hash) in flags:
            try:
                output_filename = f"c-benchmark-tester-{c_file}-{hash}.exe"
                os.remove(output_filename)
            except Exception as err:
                print(f"ERROR: {err}")


if __name__ == '__main__':
    benchmark_gcc_flags()