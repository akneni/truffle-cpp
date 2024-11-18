use std::collections::HashSet;
use std::fs;
use std::path::Path;
use std::process::{self, Command};

use anyhow::{anyhow, Result};
use colored::*;

#[derive(Debug, Clone, Copy, PartialEq, serde::Serialize, serde::Deserialize, clap::ValueEnum)]
pub enum Language {
    C,
    Cpp,
}

impl Language {
    pub fn new(mut s: &str) -> Result<Self> {
        if s.starts_with("--") {
            s = &s[2..];
        }
        else if s.starts_with(".") {
            s = &s[1..];
        }
        let s = s.to_lowercase();
        match s.as_str() {
            "c" => Ok(Language::C),
            "cpp" => Ok(Language::Cpp),
            _ => Err(anyhow!("string not valid")),
        }
    }

    pub fn file_ext(&self) -> &'static str {
        match self {
            Self::C => ".c",
            Self::Cpp => ".cpp",
        }
    }
}

#[allow(unused)]
#[derive(Debug, Default, Clone)]
pub struct CompilerVersions {
    gcc: Option<String>,
    gpp: Option<String>,
    clang: Option<String>,
    clangpp: Option<String>,
}

impl CompilerVersions {
    #[allow(unused)]
    pub fn new() -> Result<Self> {
        let mut ver = CompilerVersions::default();

        let compilers = ["gcc", "g++", "clang", "clang++"];
        let mut handles = vec![];

        for c in compilers {
            let cmd = Command::new(c)
                .arg("--version")
                .stdout(process::Stdio::piped())
                .spawn();
            handles.push(cmd);
        }

        let mut stdout_vec = vec![];
        for h in handles {
            if let Ok(h) = h {
                let process_res = h.wait_with_output();
                if let Ok(out) = process_res {
                    if out.status.success() {
                        let stdout = String::from_utf8(out.stdout).unwrap();
                        stdout_vec.push(Some(stdout));
                        continue;
                    }
                }
            }
            stdout_vec.push(None);
        }

        let gcc_installed = stdout_vec[..2].iter().all(|s| s.is_some());
        let clang_installed = stdout_vec[2..].iter().all(|s| s.is_some());

        if !gcc_installed && !clang_installed {
            return Err(anyhow!("no version of gcc or clang found"));
        }

        let fields = [&mut ver.gcc, &mut ver.gpp, &mut ver.clang, &mut ver.clangpp];
        for (field, out) in fields.into_iter().zip(stdout_vec.iter()) {
            if let Some(s) = out {
                *field = Some(Self::parse_gc(s));
            }
        }

        Ok(ver)
    }

    fn parse_gc(out: &str) -> String {
        if out.starts_with("gcc") || out.starts_with("g++") {
            let first_line = out.split_once("\n").unwrap().0;
            let version = first_line.split_once(")").unwrap().1.trim();
            return version.to_string();
        }
        let first_line = out.split_once("-").unwrap().0;
        let version = first_line.split_once("version").unwrap().1.trim();
        version.to_string()
    }
}

/// Returns a vector of the included statements
/// Ex) `["stdio.h", "<math.h>"]`
pub fn extract_include_statements(path: &Path) -> Vec<String> {
    let mut path = path.to_path_buf();
    path.push("src");

    let mut includes = HashSet::new();

    for p in path.read_dir().unwrap() {
        let p = p.unwrap();

        let text = fs::read_to_string(p.path()).unwrap();

        let local_include = text
            .split("\n")
            .map(|s| s.trim())
            .filter(|s| s.starts_with("#include") && s.ends_with(">"))
            .map(|s| format!("<{}", s.split_once("<").unwrap().1));

        for inc in local_include {
            includes.insert(inc);
        }
    }

    includes.into_iter().collect()
}

pub fn expand_user(path: &str) -> String {
    if path.starts_with("~/") {
        if let Some(home_dir) = std::env::var_os("HOME") {
            let path_without_tilde = &path[2..]; // Remove "~/" prefix
            return Path::new(&home_dir)
                .join(path_without_tilde)
                .to_str()
                .unwrap()
                .to_string();
        }
    }
    path.to_string()
}

/// Prints a warning message in a standardized way
pub fn print_warning(
    warning_source: &str,
    filename: &str,
    line: &str,
    warning_type: &str,
    msg: &str,
) {
    let err_msg = format!(
        "{} {} [src/{} | Line {} ]: {:?}\n{}", 
        warning_source.red().bold(),
        "Warning".red().bold(),
        filename, 
        line, 
        warning_type, 
        msg,
    );
    println!("{}\n", err_msg);
}

