use crate::{config::Config, constants::CONFIG_FILE};
use crate::utils;

use std::{fs, path::{Path, PathBuf}};
use anyhow::{anyhow, Result};

pub fn create_project(path: &Path) -> Result<()> {
    let mut path_b = PathBuf::from(path);
    path_b.push(CONFIG_FILE);
    if path_b.exists() {
        return Err(anyhow!("directory is already a TrufC project."));
    }

    let mut config = Config::default();

    let dir_name = path.file_name()
        .unwrap()
        .to_str()
        .unwrap();

    config.project.name = dir_name.to_string();

    let config_str = toml::to_string(&config)?;

    fs::write(&path_b, config_str)?;

    path_b.pop();
    path_b.push("src");

    fs::create_dir(&path_b).unwrap();

    path_b.push("main.c");

    let starter_code = "#include<stdio.h>\n\nint main() {\n\tprintf(\"Welcome to Truffle!\");\n\treturn 0;\n}";
    fs::write(&path_b, starter_code)?;

    Ok(())
}

pub fn link_files(path: &Path) -> Vec<String> {
    let mut path = path.to_path_buf();
    path.push("src");

    let mut c_files = vec![];

    for file in fs::read_dir(&path).unwrap() {
        if let Ok(file) = file {
            let file = file.file_name();
            let filename = file.to_str().unwrap();
            c_files.push(format!("src/{}", filename));
        }
    }

    c_files
}

pub fn link_lib(path: &Path) -> Vec<String> {
    let c_lib_mappings = [
        ("<math.h>", "-lm"),                // Math library
        ("<omp.h>", "-fopenmp"),            // OpenMP library
        ("<pthread.h>", "-pthread"),        // POSIX threads
        ("<zlib.h>", "-lz"),                // Compression library (zlib)
        ("<curl/curl.h>", "-lcurl"),        // cURL library for network operations
        ("<ssl.h>", "-lssl"),               // SSL/TLS library
        ("<crypto.h>", "-lcrypto"),         // Cryptography library
        ("<ncurses.h>", "-lncurses"),       // Ncurses for terminal handling
        ("<mariadb/mysql.h>", "-lmariadb"), // MySQL/MariaDB client library
        ("<sqlite3.h>", "-lsqlite3"),       // SQLite library
        ("<GL/gl.h>", "-lGL"),              // OpenGL library
        ("<GL/glut.h>", "-lglut"),          // GLUT library for OpenGL
        ("<X11/Xlib.h>", "-lX11"),          // X11 library for X Window System
    ];

    let includes = utils::extract_include_statements(path);

    let mut libs = vec![];
    for (incl, link) in c_lib_mappings {
        if includes.contains(&incl.to_string()) {
            libs.push(link.to_string())
        }
    }

    libs
}

pub fn opt_flags(profile: &str, config: &Config) -> Result<Vec<String>> {
    if profile == "--dev" {
        return Ok(vec!["-g".to_string(), "-O0".to_string(), "-Wall".to_string(), "-fsanitize=undefined".to_string()]);
    }
    else if profile == "--release" {
        return Ok(vec!["-O3".to_string(), "-funroll-loops".to_string(), "-fprefetch-loop-arrays".to_string(), "-march=native".to_string(), "-ffast-math".to_string()]);
    }
    let profile = &profile[2..];

    if let Some(prof) = config.profile.get(profile) {
        return Ok(prof.flags.clone());
    }
    Err(anyhow!("profile `--{}` does not exist. Choose a different profile or declare it in TrufC.toml", profile))
}

pub fn full_compilation_cmd(config: &Config, profile: &str, link_file: &Vec<String>, link_lib: &Vec<String>, flags: &Vec<String>) -> Result<Vec<String>> {

    let compiler: &str;
    let standard: &str;

    if link_file.contains(&"src/main.cpp".to_string()) {
        compiler  = &config.compiler.cpp.name;
        standard = &config.compiler.cpp.standard;
    }
    else if link_file.contains(&"src/main.c".to_string()) {
        compiler  = &config.compiler.c.name;
        standard = &config.compiler.c.standard;
    }
    else {
        return Err(anyhow!("No `src/main.c` or `src/main.cpp` file found"));
    }

    let mut command = vec![
        compiler.to_string(),
        format!("-std={}", standard),
    ];

    command.extend_from_slice(flags);

    let profile = &profile[2..];
    let build_path = format!("build/{}/{}", profile, &config.project.name);

    command.extend_from_slice(&[
        "-o".to_string(),
        build_path,
    ]);

    command.extend_from_slice(link_file);
    command.extend_from_slice(link_lib);

    Ok(command)
}
