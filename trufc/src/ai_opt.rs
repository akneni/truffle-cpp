use std::{fs, path::{Path, PathBuf}, process::{self, Command}};

use anyhow::Result;

use crate::{cli::AiOptCommand, config::{self, Config}, constants::DATA_DIR};

pub fn handle_cli(command: AiOptCommand) -> Result<()> {

    match command {
        AiOptCommand::ListModels => {
            let launch_cmd = build_pytrufc_cmd(&[
                "list-models"
            ]);
            let app_data = *DATA_DIR;

            let child = Command::new(&launch_cmd[0])
                .args(&launch_cmd[1..])
                .current_dir(app_data)
                .stdout(process::Stdio::piped())
                .spawn()?;
            
            let stdout = child.wait_with_output().unwrap();
            let out_str= String::from_utf8(stdout.stdout)?;

            let installed_models = get_installed_models();

            println!("Models\n{}", "=".repeat(47));
            for (i, s) in out_str.trim().split("|||").enumerate() {
                let mut status = "Not Installed";
                if installed_models.contains(&s.to_string()) {
                    status = "Installed";                }

                println!("{:<3} | {:^25} | {}", i+1, &s[..(s.len()-3)], status);
            }
        }
        AiOptCommand::Pull { model } => {

        }
        AiOptCommand::Optimize => {
            // Get current directory as source path
            let mut source_path = std::env::current_dir()?;

            let mut source_path = PathBuf::from("/home/aknen/Documents/coding-projects/truffle-project/c-test-proj/");


            source_path.push("src");
            let source_path_str = source_path
                .to_str()
                .unwrap()
                .to_string();


            source_path.pop();
            source_path.push("TrufC.toml");
            let mut config = Config::from(source_path.as_path())?;
            

            let app_data = *DATA_DIR;
            let app_data_str = app_data.to_str().unwrap();

            // Build the command
            let launch_cmd = build_pytrufc_cmd(&[
                "inference", 
                "--source-path", 
                &source_path_str,
                "--app-data-path",
                app_data_str,
            ]);

            println!("{:?}", launch_cmd);
            std::process::exit(0);

            // Execute the command and capture output
            let child = Command::new(&launch_cmd[0])
                .args(&launch_cmd[1..])
                .current_dir(app_data)
                .stdout(process::Stdio::piped())
                .spawn()?;

            let output = child.wait_with_output()?;
            if !output.status.success() {
                // The command failed
                eprintln!("Error: pytrufc.py inference command failed");
                return Err(anyhow::anyhow!("pytrufc.py inference command failed"));
            }
            let out_str = String::from_utf8(output.stdout)?;
            let flags: Vec<String> = out_str
                .trim()
                .split("|||")
                .map(|s| s.to_string())
                .collect();

            // Print the generated flags
            println!("Generated flags: {:?}", flags);

            config.profile.insert("release".to_string(), config::Profile { flags });
            config.to_disk(source_path.as_path());
        }
    }

    Ok(())
}

fn build_pytrufc_cmd(args: &[&str]) -> Vec<String> {
    let app_data = *DATA_DIR;

    let app_data_str = app_data
        .to_str()
        .unwrap()
        .to_string();

    let uv = app_data.join("uv");
    let uv_str = uv
        .to_str()
        .unwrap()
        .to_string();

    let py = app_data.join("pytrufc.py");
    let py_str = py
        .to_str()
        .unwrap()
        .to_string();


    let mut launch_cmd = vec![
        // uv_str,
        "uv".to_string(),
        "run".to_string(),
        py_str,
    ];

    for arg in args {
        launch_cmd.push(arg.to_string());
    }

    launch_cmd
}

fn get_installed_models() -> Vec<String> {
    let mut models = vec![];

    let app_data = *DATA_DIR;

    let model_dir = app_data.join("models");

    for file in fs::read_dir(model_dir).unwrap() {
        if let Ok(file) = file {
            let model_str = file
                .path()
                .to_str()
                .unwrap()
                .to_string();

            models.push(model_str);
        }
    }

    models
}