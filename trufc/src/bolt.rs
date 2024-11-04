use std::{cell::RefCell, sync::Arc, env, fs, path::Path, process::Command, sync::Mutex};
use anyhow::{Result, anyhow};
use once_cell::sync::Lazy;

use crate::constants::TMP_DIR;


const BOLT_BIN_DEFAULT: &str = "/sas/home/aknen/Documents/coding-projects/llm-pgo/bolt-build/bin/";

static BOLT_BIN: Lazy<Arc<Mutex<String>>> = Lazy::new(|| {
    let bolt_bin_path = env::var("BOLT_BIN");
    if let Ok(bolt_bin_path) = bolt_bin_path {
        return Arc::new(Mutex::new(bolt_bin_path));
    }
    Arc::new(Mutex::new(BOLT_BIN_DEFAULT.to_string()))
});

pub fn compile_bolt(target: &str, output: &str, bolt_path: Option<String>) -> Result<()> {
    if let Some(bolt_path) = bolt_path {
        let mut lock = BOLT_BIN.lock().unwrap();
        *lock = bolt_path;
    }

    let c_bin = build_c(target)?;
    let inst_bin = build_instrumented(&c_bin)?;
    let perf_data = exec_instrumented(&inst_bin)?;

    build_optimized_binary(&c_bin, &perf_data, output)?;

    let mut err = None;
    for path in [c_bin, inst_bin, perf_data] {
        let e = fs::remove_file(&path);
        if let Err(e) = e {
            err = Some(e);
        }
    }
    if let Some(err) = err {
        return Err(anyhow!("Error removing files: {}", err));
    }
    
    Ok(())
}

/// Returns the output of the compiled c file
fn build_c(target: &str) -> Result<String> {
    if !Path::new(TMP_DIR).exists() {
        fs::create_dir(TMP_DIR).unwrap();
    }

    let out_path = format!("{}/{}", TMP_DIR, "c-exe-bolt");

    let mut child = Command::new("gcc")
        .args([
            "-fno-reorder-blocks-and-partition",
            "-fno-omit-frame-pointer",
            "-Wl,--emit-relocs",
            "-O3",
            "-o",
            &out_path,
            target,
        ])
        .stdout(std::process::Stdio::piped())
        .spawn()
        .map_err(|e| anyhow!("Error spawning `gcc` process: {}", e))?;

    child.wait().map(|e| anyhow!("Error waiting on `gcc` process: {}", e))?;    
    
    Ok(out_path)
}

/// llvm-bolt <target> -instrument -o ./.tmp/bolt-exe-instrumented
/// Returns the filepath of the instrumented binary
fn build_instrumented(target: &str) -> Result<String> {
    if !Path::new(TMP_DIR).exists() {
        fs::create_dir(TMP_DIR).unwrap();
    }

    let out_path = format!("{}/{}", TMP_DIR, "bolt-exe-instrumented");
    let bolt = format!("{}/llvm-bolt", *BOLT_BIN.lock().unwrap());

    let mut child = Command::new(&bolt)
        .args([
            target,
            "-instrument",
            "-o",
            &out_path,
        ])
        .stdout(std::process::Stdio::piped())
        .spawn()
        .map_err(|e| anyhow!("Error spawning `llvm-bolt` process: {}", e))?;

    child.wait().map(|e| anyhow!("Error waiting on `llvm-bolt` process: {}", e))?;
    
    Ok(out_path)
}

fn exec_instrumented(target: &str) -> Result<String> {
    if !Path::new(TMP_DIR).exists() {
        fs::create_dir(TMP_DIR).unwrap();
    }

    let mut inst_path = target.to_string();
    if !inst_path.starts_with("./") {
        inst_path = format!("./{}", inst_path);
    }

    let mut child = Command::new(&inst_path)
        .stdout(std::process::Stdio::piped())
        .spawn()
        .map_err(|e| anyhow!("Error spawning {}: {}", target, e))?;

    child.wait().map(|e| anyhow!("Error waiting on {}: {}", target, e))?;
    
    let out_dir = format!("{}/perf.fdata", TMP_DIR);
    fs::rename("/tmp/prof.fdata", &out_dir)
        .map_err(|e| anyhow!("Failed to move perf.fdata file: {}", e))?;

    Ok(out_dir)
}

/// llvm-bolt <c_bin> -o <out_filename> -data=<perf_data> -reorder-blocks=ext-tsp -reorder-functions=hfsort -split-functions -split-all-cold -dyno-stats
fn build_optimized_binary(c_bin: &str, perf_data: &str, out_filename: &str) -> Result<()> {
    let bolt = format!("{}/llvm-bolt", *BOLT_BIN.lock().unwrap());

    let mut child = Command::new(&bolt)
        .args([
            c_bin,
            "-o",
            out_filename,
            &format!("-data={}", perf_data),
            "-reorder-blocks=ext-tsp",
            "-split-functions",
            "-split-all-cold",
            "-dyno-stats",
        ])
        .stdout(std::process::Stdio::piped())
        .spawn()
        .map_err(|e| anyhow!("Error spawning `llvm-bolt` process: {}", e))?;
    
    child.wait().map(|e| anyhow!("Error waiting on `llvm-bolt` process: {}", e))?;
    
    Ok(())
}