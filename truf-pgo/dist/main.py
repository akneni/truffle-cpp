import sys
import os
import subprocess
import torch
import utils
import torch_utils
from transformers import AutoTokenizer

def parse_cli(flag: str, required=True) -> str:
    for i, arg in enumerate(sys.argv):
        if flag == arg:
            return sys.argv[i+1]
        elif arg.startswith(flag + '='):
            return arg.partition(flag + '=')[2]
    
    if required:
        print(f"Required flag `{flag}` not passed")
        exit(1)
    return None

MODELS = {
    "trufling-flags-70M.pt": {
        "url": "https://ucarecdn.com/6b12fe30-14ff-46a8-ab7e-3ae7e74b9776/truflingflags70M.pt",
        "tokenizer": "distilbert-base-uncased",
    }
}

def pull_model(model: str, app_data_path: str):
    model_dir_path = os.path.join(app_data_path, 'models')
    if not os.path.exists(model_dir_path):
        os.mkdir(model_dir_path)

    out_path = os.path.join(model_dir_path, model)

    command = ["curl", MODELS[model]['url'], "-o", out_path]

    subprocess.run(command)
    
    print("finished downloading model")

def infer_flags(model_path: str, source_path: str) -> list[str]:
    device = 'cuda' if torch.cuda.is_available() else 'cpu'
    model_name = os.path.basename(model_name)

    model = torch.load(model_path).to(device)
    tokenizer = AutoTokenizer.from_pretrained(MODELS[model_name]['tokenizer'])


    sys_spec = utils.SystemSpec.generate()
    proj_source = ""

    for file in os.listdir(source_path):
        filepath = os.path.join(source_path, file)
        with open(filepath, 'r') as f:
            c_source = f.read()
        proj_source += f"// Filename: {file}\n{c_source}\n\n\n"
    
    prompt = torch_utils.InferenceUtils.format_prompt(
        c_source=proj_source,
        cpu=sys_spec.cpu,
        arch=sys_spec.cpu_arch,
        os=sys_spec.os_version,
        compiler=sys_spec.compiler
    )

    in_tsr = torch_utils.InferenceUtils.tokenize(tokenizer, prompt)

    with torch.no_grad():
        out_tsr = model(in_tsr)
    flags = torch_utils.InferenceUtils.gen_flags(out_tsr)
    return flags

if __name__ == '__main__':
    if len(sys.argv) <= 1:
        print("Must enter a command")
    command = sys.argv[1]

    if command == "inference":
        app_data_path = parse_cli('--app-data-path')
        source_path = parse_cli('--source-path')
        target_model = parse_cli('--model', required=False)
        if target_model is not None and not target_model.endswith('.pt'):
            target_model += '.pt'
        if target_model is not None and target_model not in MODELS:
            print("Model specified ")

        model_dir_path = os.path.join(app_data_path, 'models')
        existing_models = os.listdir(model_dir_path)
        if len(existing_models) == 0:
            print("You haven't downloaded any models yet. Available models are: ")
            for i, model in enumerate(MODELS.keys()):
                print(f"{i:<2}) {model[:-3]}")
            exit(1)
        
        if target_model is None:
            model_path = existing_models[0]
        elif target_model in existing_models:
            model = target_model
        else:
            print("Model specified hasn't been downloaded. ")


        flags = infer_flags(model_path, source_path)
        print("|||".join(flags))

    elif command == "pull":
        model = parse_cli('--model')
        app_data_path = parse_cli('--app-data-path')

        if not model.endswith('.pt'):
            model += ".pt"
        if model not in MODELS:
            print(f"Model `{model[:-3]}` does not exist")
        
        pull_model(model, app_data_path)


    