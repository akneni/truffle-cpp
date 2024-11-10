use crate::utils;

use clap::{Parser, Subcommand};

#[derive(Parser, Debug)]
#[command(name = "TrufC")]
#[command(version = "0.0.4")]
#[command(about = "A build system that integrates with truffle optimizations.", long_about = None)]
pub struct CliCommand {
    #[command(subcommand)]
    pub command: Commands,
}

#[derive(Subcommand, Debug)]
pub enum Commands {
    Init {
        #[arg(value_enum, long, default_value = "c")]
        language: utils::Language,
    },
    New {
        proj_name: String,

        #[arg(value_enum, long, default_value = "c")]
        language: utils::Language,
    },

    // Clap doesn't provide any way to structure the syntax to be `trufc run --profile
    // So, we'll have to paese these manually.
    Build {
        profile: String,
    },
    Run {
        profile: String,
        args: Vec<String>,
    },
    AiOpt {
        #[command(subcommand)]
        command: AiOptCommand
    }
}

impl Commands {
    pub fn new(variant: &str, profile: &str, args: Vec<String>) -> Self {
        match variant {
            "build" => Self::Build {
                profile: profile.to_string(),
            },
            "run" => Self::Run {
                profile: profile.to_string(),
                args,
            },
            _ => panic!("Parameter `variant` must be one of 'build' or 'run'"),
        }
    }
}

#[derive(Subcommand, Debug)]
pub enum AiOptCommand {
    ListModels,
    Pull {
        model: String
    },
    Optimize,
}
