// clang -O3 -fprofile-instr-generate -o main-inst.lpgo src/main.c
// export LLVM_PROFILE_FILE="main.profraw"
// ./main-inst.lpgo
// rm main-inst.lpgo
// mv default.profraw main.profraw
// llvm-profdata merge -output=main.profdata main.profraw
// clang -O3 -fprofile-instr-use=main.profdata -o main.lpgo src/main.c

use std::{fs, path::Path, process::Command};
use anyhow::{Result, anyhow};

use crate::constants::TMP_DIR;


pub fn compile_llvm_pgo(target: &str, output: &str) -> Result<()> {
    let inst_bin = build_instrumented(target)?;
    let prof_raw = exec_instrumented(&inst_bin)?;
    let prof_data = cvt_prof_data(&prof_raw)?;


    build_optimized_binary(target, &prof_data, output)?;

    let mut err = None;
    for path in [inst_bin, prof_raw, prof_data] {
        let e = fs::remove_file(&path);
        if let Err(e) = e {
            err = Some(e);
        }
    }
    if let Some(err) = err {
        println!("WARNING [error removing files]: {}", err);
        // return Err(anyhow!("Error removing files: {}", err));
    }
    
    Ok(())
}

/// clang -O3 -fprofile-instr-generate -o ./.tmp/c-exe-llvm-pgo <target>
/// Returns the output of the compiled c file
fn build_instrumented(target: &str) -> Result<String> {
    if !Path::new(TMP_DIR).exists() {
        fs::create_dir(TMP_DIR).unwrap();
    }

    let out_path = format!("{}/{}", TMP_DIR, "c-exe-llvm-pgo");

    let mut child = Command::new("clang")
        .args([
            "-O3",
            "-fprofile-instr-generate",
            "-o",
            &out_path,
            target,
        ])
        .stdout(std::process::Stdio::piped())
        .spawn()
        .map_err(|e| anyhow!("Error spawning `clang` process: {}", e))?;

    child.wait().map(|e| anyhow!("Error waiting on `clang` process: {}", e))?;    
    
    Ok(out_path)
}

/// ./<target>
/// mv default.profraw main.profraw
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

    child.wait().map_err(|e| anyhow!("Error waiting on {}: {}", target, e))?;
    
    let out_dir = format!("{}/llvm-pgo.profraw", TMP_DIR);
    fs::rename("default.profraw", &out_dir)
        .map_err(|e| anyhow!("Failed to move default.profraw file: {}", e))?;

    Ok(out_dir)
}

/// llvm-profdata merge -output=./.tmp./llvm-pgo.profdata <prof_data>
fn cvt_prof_data(prof_raw: &str) -> Result<String> {
    if !Path::new(TMP_DIR).exists() {
        fs::create_dir(TMP_DIR).unwrap();
    }

    let output = format!("{}/llvm-pgo.profdata", TMP_DIR);

    let mut child = Command::new("llvm-profdata")
        .args([
            "merge",
            &format!("-output={}", output),
            &prof_raw,
        ])
        .stdout(std::process::Stdio::piped())
        .spawn()
        .map_err(|e| anyhow!("Error spawning `llvm-profdata`: {}", e))?;

    child.wait().map_err(|e| anyhow!("Error waiting on `llvm-profdata`: {}", e))?;
    Ok(output)
}

/// clang -O3 -fprofile-instr-use=<prof_data> -o <out_filename> <c_source>
fn build_optimized_binary(c_source: &str, prof_data: &str, out_filename: &str) -> Result<()> {

    let mut child = Command::new("clang")
        .args([
            "-O3",
            &format!("-fprofile-instr-use={}", prof_data),
            "-o",
            out_filename,
            c_source,
        ])
        .stdout(std::process::Stdio::piped())
        .spawn()
        .map_err(|e| anyhow!("Error spawning `clang` process: {}", e))?;
    
    child.wait().map(|e| anyhow!("Error waiting on `clang` process: {}", e))?;
    
    Ok(())
}