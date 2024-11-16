#![allow(unused)]

use std::fs;

use serde::Deserialize;
use serde_xml_rs;
use anyhow::Result;

#[derive(Debug, Default, Deserialize)]
pub struct VgOutput {
    #[serde(rename = "protocolversion", default)]
    pub protocol_version: Option<String>,

    #[serde(rename = "protocoltool", default)]
    pub protocol_tool: Option<String>,

    #[serde(rename = "preamble", default)]
    pub preamble: Option<VgPreamble>,

    #[serde(default)]
    pub pid: Option<u32>,

    #[serde(default)]
    pub ppid: Option<u32>,

    #[serde(default)]
    pub tool: Option<String>,

    #[serde(rename = "args", default)]
    pub arguments: Option<VgArguments>,

    #[serde(rename = "status", default)]
    pub statuses: Option<Vec<VgStatus>>,

    #[serde(rename = "error", default)]
    pub errors: Vec<VgError>,
}

impl VgOutput {
    pub fn from(filepath: &str) -> Result<Self> {
        let xml = fs::read_to_string(filepath)?;
        let vg_out = serde_xml_rs::from_str(&xml)?;
        Ok(vg_out)
    }

    pub fn from_str(xml_text: &str) -> Result<Self> {
        let vg_out = serde_xml_rs::from_str(&xml_text)?;
        Ok(vg_out)
    }
}

#[derive(Debug, Deserialize)]
pub struct VgPreamble {
    #[serde(rename = "line", default)]
    pub lines: Option<Vec<String>>,
}

#[derive(Debug, Deserialize)]
pub struct VgArguments {
    #[serde(rename = "vargv", default)]
    pub valgrind_args: Option<VgCommand>,

    #[serde(rename = "argv", default)]
    pub program_args: Option<VgCommand>,
}

#[derive(Debug, Deserialize)]
pub struct VgCommand {
    #[serde(default)]
    pub exe: Option<String>,

    #[serde(rename = "arg", default)]
    pub args: Option<Vec<String>>,
}

#[derive(Debug, Deserialize)]
pub struct VgStatus {
    #[serde(default)]
    pub state: Option<String>,

    #[serde(default)]
    pub time: Option<String>,
}

#[derive(Debug, Deserialize)]
pub struct VgError {
    pub unique: String,
    pub tid: u32,
    pub kind: String,

    #[serde(rename = "xwhat")]
    pub xwhat: VgXWhat,

    pub stack: VgStack,
}

#[derive(Debug, Deserialize)]
pub struct VgXWhat {
    pub text: String,

    #[serde(rename = "leakedbytes")]
    pub leaked_bytes: u32,

    #[serde(rename = "leakedblocks")]
    pub leaked_blocks: u32,
}

#[derive(Debug, Deserialize)]
pub struct VgStack {
    #[serde(rename = "frame", default)]
    pub frames: Option<Vec<VgFrame>>,
}

#[derive(Debug, Deserialize)]
pub struct VgFrame {
    #[serde(default)]
    pub ip: Option<String>,

    #[serde(default)]
    pub obj: Option<String>,

    #[serde(rename = "fn", default)]
    pub fn_name: Option<String>,

    #[serde(rename = "dir", default)]
    pub dir: Option<String>,

    #[serde(rename = "file", default)]
    pub file: Option<String>,

    #[serde(rename = "line", default)]
    pub line: Option<u32>,
}
