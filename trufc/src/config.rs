use anyhow::{anyhow, Result};
use serde::{Deserialize, Serialize};
use std::{fs, collections::HashMap, path::Path};
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
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Compiler {
    pub c: CompilerDetail,
    pub cpp: CompilerDetail,
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
    const RESERVED_PROFILES: [&str; 2] = ["dev", "release"];

    pub fn from(path: &Path) -> Result<Self> {
        let toml_str = fs::read_to_string(path)?;

        let config: Config = toml::from_str(&toml_str)?;
        config.validate_profiles()?;

        Ok(config)
    }

    pub fn validate_profiles(&self) -> Result<()> {
        for k in self.profile.keys() {
            if Self::RESERVED_PROFILES.contains(&k.as_str()) {
                return Err(anyhow!("cannot use reserved profile name `{}`", k));
            }
        }

        Ok(())
    }
}

impl Default for Config {
    fn default() -> Self {
        let project = Project {
            name: "c_cpp_project".to_string(),
            version: "0.0.1".to_string(),
        };

        let compiler = Compiler {
            c: CompilerDetail {
                name: "gcc".to_string(), 
                version: "11.4.0".to_string(),
                standard: "c23".to_string(),
            },
            cpp: CompilerDetail {
                name: "g++".to_string(), 
                version: "11.4.0".to_string(),
                standard: "c++23".to_string(),
            },
        };

        let profile = HashMap::new();

        Config { project, compiler, profile }
    }
}
