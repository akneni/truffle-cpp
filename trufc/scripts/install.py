import subprocess
import os
import sys
import platform

URL = 'https://ucarecdn.com/ea48ad5b-08c4-4463-8901-0a7eb7478cf0/trufcv00.1linuxx86_64'
USER_BIN = '~/.local/bin'
SYSTEM_BIN = '/usr/bin'

def install(install_mode: str):
    path = USER_BIN if install_mode == '--user' else SYSTEM_BIN
    path = os.path.join(path, 'trufc')

    if os.path.exists(path):
        res = input('TrufC already exists on your machine. Reinstall? (y/[n])')
        if res.lower().strip() != 'y':
            print("Exiting...")
            exit()
        print("Reinstalling...")
        os.remove(path)

    print(f"curl {URL} -o {path}")
    exit(0)

    cmd = subprocess.Popen(f"curl {url} -o {path}".split(), stdout=subprocess.STDOUT)
    cmd.wait()

    cmd = subprocess.Popen(f"chmod +x {path}".split(), stdout=subprocess.DEVNULL)
    cmd.wait()

def uninstall():
    for path in [USER_BIN, SYSTEM_BIN]:
        path = os.path.join(path, 'trufc')
        if os.path.exists(path):
            os.remove(path)
    
if __name__ == '__main__':
    # Verifies OS
    if platform.system().lower() == 'windows':
        print(f"Unfortunately, Windows is not currently supported. However, Truffle does support WSL! (https://learn.microsoft.com/en-us/windows/wsl/install)")
        exit()
    elif platform.system().lower() != 'linux':
        print(f"Unfortunately, {platform.system()} is not currently supported.")
        exit()

    # Gets the location to install the binary
    install_mode = None
    if len(sys.argv) > 1:
        if sys.argv[1] in ['--user', '--system']:
            install_mode = sys.argv[1]
        elif sys.argv[1] == '--uninstall':
            uninstall()
            exit(0)
        else:
            print(f"Invalid flag `{sys.argv[1]}`")
            exit(1)
    else:
        install_mode = input("Install as system or user? Defaults to user. (sys/user): ").strip()
        if install_mode not in ['', 'sys', 'user']:
            print("Invalid input")
            exit(1)
        elif install_mode in ['', 'user']:
            install_mode = '--user'
        else:
            install_mode = '--system'
    
    install(install_mode)
