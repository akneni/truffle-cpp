mod bolt;
mod gcc;
mod llvm_pgo;
mod constants;

use std::{env, process};

fn parse_target(cli: &[String]) -> String {
    let target = cli.last().unwrap();
    assert!(target.ends_with(".c"));
    target.clone()
}

/// Returns the value of a flag.
/// Ex) calling `parse_flag_kv("gcc -O3 -o main main.c", "-o")` will return `"main"`
fn parse_flag_kv(cli: &[String], flag: &str) -> Option<String> {
    let cmp_target = format!("{}=", flag);
    for (i, arg) in cli.iter().enumerate() {
        if arg == flag {
            return Some(cli[i+1].clone());
        }
        else if arg.starts_with(&cmp_target) {
            return Some(arg.split_once("=").unwrap().1.to_string());
        }
    }
    None
}

/// Returns a bool based on the existence of a flag.
/// Ex) calling `parse_flag_kv("gcc -O3 -o main main.c", "-O3")` will return `true`
fn parse_flag_k(cli: &[String], flag: &str) -> bool {
    let idx = cli.iter().position(|s| s == flag);
    idx.is_some()
}

/// Accepts multiple possible flags to search for, and returns one if it exists.
/// Ex) calling `parse_flags_kv("gcc -O3 -o main main.c", ["-O3", "-02"])` will return `"-O3"`
fn parse_flags_k(cli: &[String], flag: &[&str]) -> Option<String> {
    let idx = cli.iter().position(|s| flag.contains(&s.as_str()));
    if let Some(idx) = idx {
        return Some(cli[idx].to_string());
    }
    None
}

fn main() {
    let cli: Vec<String> = env::args().collect();
    if cli.len() < 2 {
        println!("Error, must pass a command");
        process::exit(0);
    }

    match cli[1].as_str() {
        "gcc" => {
            gcc::build_c(&cli[2..]).unwrap();
        },
        "gcc-pgo" => {

        },
        "bolt" => {
            let target = parse_target(&cli);
            let output_filepath = parse_flag_kv(&cli, "-o")
                .unwrap_or(format!("{}.exe", &target[0..(target.len()-2)]));

            let bolt_path = parse_flag_kv(&cli, "--bolt-path");

            bolt::compile_bolt(&target, &output_filepath, bolt_path).unwrap();
        },
        "lvm-pgo" => {
            let target = parse_target(&cli);
            let output_filepath = parse_flag_kv(&cli, "-o")
                .unwrap_or(format!("{}.exe", &target[0..(target.len()-2)]));

            llvm_pgo::compile_llvm_pgo(&target, &output_filepath).unwrap();
        },
        _ => println!("In valid command: `{}`", cli[1])
    }
}
