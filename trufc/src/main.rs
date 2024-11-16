mod build_sys;
mod safety;
mod config;
mod constants;
mod valgrind;
mod ai_opt;
mod cli;
mod utils;

use anyhow::{anyhow, Result};
use constants::{CONFIG_FILE, SEPETATOR};
use config::Config;
use std::{env, fs, process, io::{self, Write}};
use clap::Parser;
use crossterm::{execute, ExecutableCommand, cursor, queue, QueueableCommand, style, terminal};

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

    let mut cwd = env::current_dir().unwrap();
    cwd.push(CONFIG_FILE);
    let config = Config::from(&cwd);

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
            let config = config.unwrap();

            handle_warnings(&config).unwrap();
            handle_build(profile, &config).unwrap();
        }
        cli::Commands::Run { profile, args } => {
            let config = config.unwrap();

            let warnings = handle_warnings(&config).unwrap();
            if let Err(e) = handle_build(profile.clone(), &config) {
                println!("Compilation Failed: {}", e);
                process::exit(1);
            }

            let mut cwd = env::current_dir().unwrap();

            cwd.push("build");
            cwd.push(&profile[2..]);
            cwd.push(config.project.name);

            let bin = cwd.to_str().unwrap();
            let valgrind_out = safety::exec_w_valgrind(bin, &args).unwrap();

            if valgrind_out.errors.len() > 0 {
                println!("{}\n", *SEPETATOR);
                safety::print_vg_errors(&valgrind_out);        
            }
        }
        cli::Commands::AiOpt { command } => {
            ai_opt::handle_cli(command).unwrap();
        }
    }
}

/// Returns true if there were warnings and false if there was no warnings.
fn handle_warnings(config: &Config) -> Result<Vec<safety::Warning>> {
    let warnings = safety::check_files(&config.project.language)?;

    for w in &warnings  {
        utils::print_warning(
            "TrufC", 
            &w.filename, 
            &format!("{}", w.line), 
            &format!("{:?}", w.warning_type), 
            &w.msg,
        );
    }
    if warnings.len() > 0 {
        println!("{}", *SEPETATOR);
    }

    Ok(warnings)
}

fn handle_build(profile: String, config: &Config) -> Result<()> {
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

    let link_file = build_sys::link_files(&cwd);
    let link_lib = build_sys::link_lib(&cwd);
    let opt_flags = build_sys::opt_flags(&profile, config).unwrap();

    let compilation_cmd =
        build_sys::full_compilation_cmd(config, &profile, &link_file, &link_lib, &opt_flags)
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
