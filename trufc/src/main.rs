mod constants;
mod utils;
mod config;
mod build_sys;


use std::{env, fs, process};

use clap::{Parser, Subcommand};
use config::Config;
use constants::CONFIG_FILE;

#[derive(Parser, Debug)]
#[command(name = "TrufC")]
#[command(version = "1.0")]
#[command(about = "A build system that integrates with truffle optimizations.", long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand, Debug)]
enum Commands {
    Init,
    New {proj_name: String},
    Build {
        profile: Option<String>
    },
    Run {
        profile: Option<String>
    },
}

fn main() {
    let cli = Cli::parse();

    match cli.command {
        Commands::Init => {
            let cwd = env::current_dir().unwrap();
            if let Err(e) = build_sys::create_project(&cwd) {
                println!("An error occurred while creating the project:\n{}", e);
                process::exit(1);
            }
        }
        Commands::New {proj_name} => {
            let mut target_dir = env::current_dir().unwrap();
            target_dir.push(proj_name);
            if target_dir.exists() {
                println!("Error: file of directory already exists");
                process::exit(1);
            }
            fs::create_dir(&target_dir).unwrap();

            if let Err(e) = build_sys::create_project(&target_dir) {
                println!("An error occurred while creating the project:\n{}", e);
            }
        }
        Commands::Build {profile} => {
            handle_build(profile);
        }
        Commands::Run {profile} => {
            handle_build(profile.clone());

            let profile = profile
                .unwrap_or("--dev".to_string());
            if !profile.starts_with("--") {
                println!("Error: profile must start with `--`");
                process::exit(1);
            }

            let mut cwd = env::current_dir().unwrap();

            cwd.push(CONFIG_FILE);
            let config = Config::from(&cwd).unwrap();
            cwd.pop();

            cwd.push("build");
            cwd.push(&profile[2..]);
            cwd.push(config.project.name);

            let bin = cwd.to_str().unwrap();
            // println!("bin = {}", bin);

            let mut child = process::Command::new(bin)
                .spawn()
                .unwrap();

            child.wait().unwrap();
        }
    }
}


fn handle_build(profile: Option<String>) {
    let profile = profile
        .unwrap_or("--dev".to_string());
    if !profile.starts_with("--") {
        println!("Error: profile must start with `--`");
        process::exit(1);
    }

    let mut cwd = env::current_dir().unwrap();
    cwd.push("build");
    cwd.push(&profile[2..]);
    if !cwd.exists() {
        fs::create_dir_all(&cwd).unwrap();
    }
    cwd.pop();
    cwd.pop();

    cwd.push(CONFIG_FILE);
    let config = Config::from(&cwd).unwrap();
    cwd.pop();

    let link_file = build_sys::link_files(&cwd);
    let link_lib = build_sys::link_lib(&cwd);
    let opt_flags = build_sys::opt_flags(&profile, &config).unwrap();

    let compilation_cmd = build_sys::full_compilation_cmd(&config, &profile, &link_file, &link_lib, &opt_flags)
        .unwrap();

    println!("{:?}", compilation_cmd);

    let mut child = process::Command::new(&compilation_cmd[0])
        .args(&compilation_cmd[1..])
        .spawn()
        .unwrap();

    child.wait().unwrap();
}