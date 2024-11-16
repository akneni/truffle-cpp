use crate::utils::expand_user;
use std::{env::consts::OS, path::{Path, PathBuf}};
use once_cell::sync::Lazy;
use colored::*;

pub const CONFIG_FILE: &str = "TrufC.toml";
pub const VALGRIND_OUT: &str = "trufc-valgrind-output.xml";

pub static DATA_DIR: Lazy<PathBuf> = Lazy::new(|| {

    let paths = [
        ("linux", "/usr/share/trufc/", "~/.local/share/trufc/"),
        (
            "macos",
            "/Library/Application Support/trufc/",
            "~/Library/Application Support/trufc/",
        ),
        (
            "windows",
            "C:\\ProgramData\\trufc\\",
            "C:\\Users\\%USERNAME%\\AppData\\Local\\trufc\\",
        ),
    ];

    for (os, sys_path, user_path) in paths {
        if OS == os {
            let user_path_s = expand_user(&user_path);
            if Path::new(sys_path).exists() {
                return Path::new(sys_path).to_path_buf();
            } 
            else if Path::new(&user_path_s).exists() {
                return Path::new(&user_path_s).to_path_buf();
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

pub static SEPETATOR: Lazy<ColoredString> = Lazy::new(|| {
    "✦ ═════════════════════════════════ ⚔ ═════════════════════════════════ ✦".to_string().blue().bold()
});