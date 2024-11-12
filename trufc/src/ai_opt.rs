use std::{fs, process::{self, Command}};

use anyhow::Result;

use crate::{cli::AiOptCommand, config::{self, Config}, constants::DATA_DIR};

pub fn handle_cli(command: AiOptCommand) -> Result<()> {

    match command {
        AiOptCommand::ListModels => {
            let launch_cmd = build_pytrufc_cmd(&[
                "list-models"
            ]);
            let app_data = &(*DATA_DIR);

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
            let launch_cmd = build_pytrufc_cmd(&[
                "pull",
                "--app-data-path",
                (*DATA_DIR).to_str().unwrap(),
                "--model",
                &model,
            ]);

            let mut child = Command::new(&launch_cmd[0])
                .args(&launch_cmd[1..])
                .current_dir(&(*DATA_DIR))
                .spawn()?;
            child.wait().unwrap();
        }
        AiOptCommand::Optimize => {
            // Get current directory as source path
            let mut source_path = std::env::current_dir()?;

            source_path.push("src");
            let source_path_str = source_path
                .to_str()
                .unwrap()
                .to_string();


            source_path.pop();
            source_path.push("TrufC.toml");
            let mut config = Config::from(source_path.as_path())?;
            

            let app_data = &(*DATA_DIR);
            let app_data_str = app_data.to_str().unwrap();

            // Build the command
            let launch_cmd = build_pytrufc_cmd(&[
                "inference", 
                "--source-path", 
                &source_path_str,
                "--app-data-path",
                app_data_str,
            ]);

            // Execute the command and capture output
            let child = Command::new(&launch_cmd[0])
                .args(&launch_cmd[1..])
                .current_dir(app_data)
                .stdout(process::Stdio::piped())
                .spawn()?;
            println!("Choosing optimal compiler flags...");

            let output = child.wait_with_output()?;
            if !output.status.success() {
                // The command failed
                let err = String::from_utf8(output.stderr)
                    .unwrap_or("non-utf-8 error message from pytrufc.py".to_string());
                let output_str = String::from_utf8(output.stdout)
                .unwrap_or("non-utf-8 output message from pytrufc.py".to_string());
                eprintln!("Error: pytrufc.py inference command failed:\nOutput:`{}`\nError:`{}`\n", err, output_str);
                return Err(anyhow::anyhow!("pytrufc.py inference command failed"));
            }
            let out_str = String::from_utf8(output.stdout)?;
            if out_str.trim().starts_with("STDOUT-PASSTHROUGH") {
                println!("{}", out_str.split("\n").skip(1).collect::<Vec<&str>>().join("\n"));
                process::exit(0);
            }

            let flags: Vec<String> = out_str
                .trim()
                .split("\n")
                .last()
                .unwrap()
                .split("|||")
                .map(|s| s.to_string())
                .collect();

            println!("Flags chosen: {}", flags.join(" "));

            config.profile.insert("release".to_string(), config::Profile { flags });
            config.to_disk(source_path.as_path());

        }
    }

    Ok(())
}

/// Build a command assuming it's process will be run 
/// with a current working directory in the location of the `uv.lock` file
fn build_pytrufc_cmd(args: &[&str]) -> Vec<String> {
    let mut launch_cmd = vec![
        "./uv".to_string(),
        "run".to_string(),
        "pytrufc.py".to_string(),
    ];

    for arg in args {
        launch_cmd.push(arg.to_string());
    }

    launch_cmd
}

fn get_installed_models() -> Vec<String> {
    let mut models = vec![];

    let app_data = &(*DATA_DIR);

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