import subprocess
import os
import sys
import platform
import urllib.request

URL = 'https://ucarecdn.com/c98cc117-b9c3-43f6-9d12-5668679e5a6b/trufcv00.4linuxx84_64'

USER_BIN = os.path.expanduser('~/.local/bin')
SYSTEM_BIN = '/usr/bin'

USER_APP_DATA = '/usr/share/'
SYSTEM_APP_DATA = os.path.expanduser('~/.local/share/')

def install(install_mode: str):
    path = USER_BIN if install_mode == '--user' else SYSTEM_BIN
    path_bin = os.path.join(path, 'trufc')

    app_data = USER_APP_DATA if install_mode == '--user' else SYSTEM_APP_DATA
    app_data = os.path.join(app_data, 'trufc')

    if os.path.exists(path_bin):
        res = input('TrufC already exists on your machine. Reinstall? (y/[n])')
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
    os.chmod(path, 0o755)

    # Create the app data directory
    os.makedirs(app_data, exist_ok=True)

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
