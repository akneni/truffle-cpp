import os
import subprocess
from fastapi import FastAPI
import uvicorn
from pydantic import BaseModel
import json
from io import StringIO
import sys

TRUFC_PATH = './target/x86_64-unknown-linux-musl/release/trufc'

app = FastAPI()

class Code(BaseModel):
    code: str

@app.post("/")
def exec_code(item: Code):
    # Redirect stdout to capture exec output
    old_stdout = sys.stdout
    sys.stdout = mystdout = StringIO()
    
    try:
        exec(item.code)
        res = mystdout.getvalue()  # Get captured output
    except Exception as e:
        res = f'Error: {e}'
    finally:
        # Restore stdout
        sys.stdout = old_stdout
    
    return {'res': res}


def compile_and_benchmark():
    try:
        for c_file in os.listdir('./C-DATASET'):
            c_filepath = os.path.join('C-DATASET', c_file)
            tools = [
                'gcc', 
                # 'bolt', 
                # 'llvm-pgo'
            ]
            child_processes = []

            for tool in tools:
                child = subprocess.Popen(
                    f"{TRUFC_PATH} {tool} -o main-{tool} {c_filepath}".split(),
                    stdout=subprocess.DEVNULL
                )
                child_processes.append(child)
            
            for child in child_processes:
                child.wait()
            print(f'Finished compiling {c_file}')

            with open(f'./docker-volume/{c_file}.txt', 'w') as f:
                for tool in tools:
                    child = subprocess.run(
                        f"{TRUFC_PATH} {tool} -o main-{tool} {c_filepath}".split(),
                        stdout=subprocess.PIPE
                    )
                    out = child.stdout.decode('utf-8')
                    f.write(f"Tool: {tool}\n==================================\n{out}")
                    f.write('\n'*5)
            print(f'Finished benchmarking {c_file}')
            print('\n\n')
    except Exception as e:
        print(f"Error in compile_and_benchmark: {e}")

if __name__ == '__main__':
    import threading
    # Run compile and benchmark in a separate thread
    threading.Thread(target=compile_and_benchmark).start()
    # Start FastAPI server
    # uvicorn.run(app, host="0.0.0.0", port=8080)
