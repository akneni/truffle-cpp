mod build_sys;
mod config;
mod constants;
mod ai_opt;
mod cli;
mod utils;

use anyhow::{anyhow, Result};
use constants::CONFIG_FILE;
use config::Config;
use std::{env, fs, process};
use clap::Parser;

fn main() {
    let cli_args: cli::CliCommand;

    let raw_cli_args = std::env::args().collect::<Vec<String>>();

    if raw_cli_args.len() < 2 {
        // Let the program fail and have Clap display it's help message
        cli_args = cli::CliCommand::parse();
    } else if raw_cli_args[1] == "run" || raw_cli_args[1] == "build" {
        let mut profile = "--dev".to_string();
        let mut args = vec![];
        if raw_cli_args.len() >= 3 && raw_cli_args[2].starts_with("--") && raw_cli_args.len() > 2 {
            profile = raw_cli_args[2].clone();
        }
        if let Some(idx) = raw_cli_args.iter().position(|i| i == "--") {
            assert!([2_usize, 3_usize].contains(&idx));
            args = raw_cli_args[(idx + 1)..].to_vec();
        } else {
            assert!(raw_cli_args.len() <= 3);
        }

        cli_args = cli::CliCommand {
            command: cli::Commands::new(&raw_cli_args[1], &profile, args),
        }
    } else {
        cli_args = cli::CliCommand::parse();
    }

    match cli_args.command {
        cli::Commands::Init { language } => {
            let cwd = env::current_dir().unwrap();

            if let Err(e) = build_sys::create_project(&cwd, language) {
                println!("An error occurred while creating the project:\n{}", e);
                process::exit(1);
            }
        }
        cli::Commands::New {
            proj_name,
            language,
        } => {
            let mut target_dir = env::current_dir().unwrap();
            target_dir.push(proj_name);
            if target_dir.exists() {
                println!("Error: file of directory already exists");
                process::exit(1);
            }
            fs::create_dir(&target_dir).unwrap();

            if let Err(e) = build_sys::create_project(&target_dir, language) {
                println!("An error occurred while creating the project:\n{}", e);
            }
        }
        cli::Commands::Build { profile } => {
            handle_build(profile).unwrap();
        }
        cli::Commands::Run { profile, args } => {
            if let Err(e) = handle_build(profile.clone()) {
                println!("Compilation Failed: {}", e);
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

            // Spawn the compiled binary and pass any arguments if necessary
            let mut child_builder = process::Command::new(bin);
            let child: &mut process::Command;
            if args.len() > 0 {
                child = child_builder.args(&args);
            } else {
                child = &mut child_builder;
            }
            let mut child = child.spawn().unwrap();
            child.wait().unwrap();
        }
        cli::Commands::AiOpt { command } => {
            ai_opt::handle_cli(command).unwrap();
        }
    }
}

fn handle_build(profile: String) -> Result<()> {
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

    let compilation_cmd =
        build_sys::full_compilation_cmd(&config, &profile, &link_file, &link_lib, &opt_flags)
            .unwrap();

    let child = process::Command::new(&compilation_cmd[0])
        .args(&compilation_cmd[1..])
        .stdout(process::Stdio::inherit())
        .stderr(process::Stdio::inherit())
        .stdin(process::Stdio::inherit())
        .output()?;

    if !child.status.success() {
        return Err(anyhow!("Compilation command exited with non-zero exit code"));
    }

    Ok(())
}
