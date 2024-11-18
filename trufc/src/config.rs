use crate::utils::CompilerVersions;

use anyhow::{anyhow, Result};
use serde::{Deserialize, Serialize};
use std::{collections::HashMap, fs, path::Path};
use toml;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Config {
    pub project: Project,
    pub compiler: Compiler,
    pub profile: HashMap<String, Profile>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Project {
    pub name: String,
    pub version: String,
    pub language: String,
    pub use_valgrind: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Compiler {
    pub c: CompilerDetail,
    pub cpp: CompilerDetail,
}

impl Compiler {
    #![allow(unused)]
    pub fn from(ver: &CompilerVersions) {
        todo!("implement this");
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CompilerDetail {
    pub name: String,
    pub version: String,
    pub standard: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Profile {
    pub flags: Vec<String>,
}

impl Config {
    const REQUIRED_PROFILES: [&str; 2] = ["dev", "release"];

    pub fn new(proj_name: &str) -> Self {
        let project = Project {
            name: proj_name.to_string(),
            version: "0.0.1".to_string(),
            language: "c".to_string(),
            use_valgrind: true,
        };

        let compiler = Compiler {
            c: CompilerDetail {
                name: "gcc".to_string(),
                version: "11.4.0".to_string(),
                standard: "c17".to_string(),
            },
            cpp: CompilerDetail {
                name: "g++".to_string(),
                version: "11.4.0".to_string(),
                standard: "c++17".to_string(),
            },
        };

        let mut profile = HashMap::new();

        profile.insert(
            "dev".to_string(),
            Profile {
                flags: vec![
                    "-g".to_string(),
                    "-O0".to_string(),
                    "-Wall".to_string(),
                    "-fsanitize=undefined".to_string(),
                ],
            },
        );

        profile.insert(
            "release".to_string(),
            Profile {
                flags: vec![
                    "-Wall".to_string(),
                    "-O3".to_string(),
                ],
            },
        );

        Config {
            project,
            compiler,
            profile,
        }
    }

    pub fn from(path: &Path) -> Result<Self> {
        let toml_str = fs::read_to_string(path)?;

        let config: Config = toml::from_str(&toml_str)?;
        config.validate_profiles()?;

        Ok(config)
    }

    pub fn to_disk(&self, path: &Path) {
        let s = toml::to_string(&self).unwrap();
        fs::write(path, s).unwrap();
    }

    pub fn validate_profiles(&self) -> Result<()> {
        for k in self.profile.keys() {
            if !Self::REQUIRED_PROFILES.contains(&k.as_str()) {
                return Err(anyhow!("Missing required profile `{}`", k));
            }
        }

        Ok(())
    }
}
