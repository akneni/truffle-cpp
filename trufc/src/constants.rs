use std::{env::consts::OS, path::Path};

use once_cell::sync::Lazy;

pub const CONFIG_FILE: &str = "TrufC.toml";

pub static DATA_DIR: Lazy<&Path> = Lazy::new(|| {
    let paths = [
        ("linux", "/usr/share/trufc/", "~/.local/share/trufc/"),
        (
            "macos",
            "/Library/Application Support/TrufC/",
            "~/Library/Application Support/TrufC/",
        ),
        (
            "windows",
            "C:\\ProgramData\\TrufC\\",
            "C:\\Users\\%USERNAME%\\AppData\\Local\\TrufC\\",
        ),
    ];

    #[cfg(debug_assertions)] {
        let app_data = Path::new("/home/aknen/Documents/coding-projects/truffle-project/example-app-data/dist/");
        return app_data;
    }

    for (os, sys_path, user_path) in paths {
        if OS == os {
            if Path::new(sys_path).exists() {
                return Path::new(sys_path);
            } else if Path::new(user_path).exists() {
                return Path::new(user_path);
            }
            panic!(
                "Error, no app data directory found. Please create the directory {} (system) or {} (user)",
                sys_path,
                user_path,
            );
        }
    }

    panic!("OS `{}` not supported", OS);
});
