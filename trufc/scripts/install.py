import subprocess
import os
import sys
import platform
import urllib.request
import tarfile

URL = 'https://ucarecdn.com/9b75d3ef-2344-4ed0-baf5-94d4fe7b6073/trufcv00.7linuxx86_64'
PY_DEPS_URL = 'https://ucarecdn.com/c8e36394-d392-41ad-9fae-ae3535097c07/distv005tar.gz'

USER_BIN = os.path.expanduser('~/.local/bin')
USER_APP_DATA = os.path.expanduser('~/.local/share/')

SYSTEM_BIN = '/usr/bin'
SYSTEM_APP_DATA = '/usr/share/'

def install_python_deps(install_mode: str, exist_ok=True):
    app_data = USER_APP_DATA if install_mode == '--user' else SYSTEM_APP_DATA
    app_data = os.path.join(app_data, 'trufc')

    if os.path.exists(app_data) and not exist_ok:
        print('TrufC already exists on your machine.')
        exit()

    # Download binary
    print(f"Downloading to {app_data}")
    os.makedirs(app_data, exist_ok=True)

    zip_path = os.path.join(app_data, "dist.tar.gz")
    urllib.request.urlretrieve(PY_DEPS_URL, zip_path)
    
    with tarfile.open(zip_path, "r:gz") as tar:
        tar.extractall(path=app_data)
    

def install(install_mode: str):
    path = USER_BIN if install_mode == '--user' else SYSTEM_BIN
    path_bin = os.path.join(path, 'trufc')

    app_data = USER_APP_DATA if install_mode == '--user' else SYSTEM_APP_DATA
    app_data = os.path.join(app_data, 'trufc')

    if os.path.exists(path_bin):
        res = input('TrufC already exists on your machine. Reinstall? (y/[n]): ')
        if res.lower().strip() != 'y':
            print("Exiting...")
            exit()
        print("Reinstalling...")
        os.remove(path_bin)

    # Download binary
    print(f"Downloading to {path_bin}")
    os.makedirs(path, exist_ok=True)
    urllib.request.urlretrieve(URL, path_bin)
    
    # Make the binary executable
    os.chmod(path_bin, 0o755)

    # Create the app data directory and install python deps
    os.makedirs(app_data, exist_ok=True)
    install_python_deps(install_mode)

def uninstall():
    for path in [USER_BIN, SYSTEM_BIN]:
        path = os.path.join(path, 'trufc')
        if os.path.exists(path):
            os.remove(path)

def purge():
    uninstall()
    for path in [USER_APP_DATA, SYSTEM_APP_DATA]:
        path = os.path.join(path, 'trufc')
        if os.path.exists(path):
            subprocess.run(f"rm -rf {path}".split(), stdout=subprocess.DEVNULL)
    
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
        if sys.argv[1] == 'install':
            if len(sys.argv) > 2 and sys.argv[2] in ['--user', '--system']:
                install_mode = sys.argv[2]
            elif len(sys.argv) > 2 and sys.argv[2] not in ['--user', '--system']:
                print(f"Invalid flag `{sys.argv[2]}`")
                exit(1)
        elif sys.argv[1] == 'uninstall':
            uninstall()
            print("Uninstalled trufc")
            exit(0)
        elif sys.argv[1] == 'purge':
            purge()
            print("Uninstalled trufc and all associated data")
            exit(0)
        elif sys.argv[1] in ['--user', '--system']:
            install_mode = sys.argv[1]
        else:
            print(f"Invalid flag `{sys.argv[1]}`")
            exit(1)
    if install_mode is None:
        install_mode = input("Install system wide or for current user only? Defaults to user. (sys/user): ").strip()
        if install_mode not in ['', 'sys', 'user']:
            print("Invalid input")
            exit(1)
        elif install_mode in ['', 'user']:
            install_mode = '--user'
        else:
            install_mode = '--system'
    
    install(install_mode)
    print("Installed trufc")
