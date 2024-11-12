import sys
import os
import subprocess
import requests
import json

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

def get_model_list() -> dict[str, dict[str, str]]:
    """
    returns `{model_name: {url: str, tokenizer: str}}`
    """
    app_data = parse_cli('--app-data-path', required=False)
    json_path = os.path.join(app_data, 'model-list.json') if app_data is not None else None

    model_lst_url = "https://raw.githubusercontent.com/akneni/truffle/refs/heads/main/truf-pgo/dist/models/model-list.json"
    try:
        models = requests.get(model_lst_url).json()
        if json_path is not None:
            with open(json_path, 'w') as f:
                f.write(json.dumps(models))
    except Exception as err:
        print(f"Warning: network error. Cannot get updated list of trufling models: {err}\n\n")
        if json_path is not None:
            with open(json_path, 'r') as f:
                models = f.read()
            models = json.loads(models)
        else:
            exit(1)
    return models

def pull_model(model: str, app_data_path: str):
    model_dir_path = os.path.join(app_data_path, 'models')
    if not os.path.exists(model_dir_path):
        os.mkdir(model_dir_path)

    out_path = os.path.join(model_dir_path, model)

    command = ["curl", MODELS[model]['url'], "-o", out_path]

    subprocess.run(command)
    
    print("finished downloading model")

def infer_flags(model_path: str, source_path: str) -> list[str]:
    import torch
    from transformers import AutoTokenizer
    import torch_utils
    import utils

    device = 'cuda' if torch.cuda.is_available() else 'cpu'
    device = torch.device(device)

    model_name = os.path.basename(model_path)

    model = torch_utils.FlagModel.from_disk(model_path, device).to(device)
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
    in_tsr = {k:v.to(device) for k, v in in_tsr.items()}

    with torch.no_grad():
        out_tsr = model(in_tsr)
    flags = torch_utils.InferenceUtils.gen_flags(out_tsr)
    return flags

MODELS = get_model_list()

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
        os.makedirs(model_dir_path, exist_ok=True)
        existing_models = os.listdir(model_dir_path)
        if len(existing_models) == 0:
            print("STDOUT-PASSTHROUGH")
            print("You haven't downloaded any models yet. Available models are: ")
            for i, model in enumerate(MODELS.keys()):
                print(f"{i:<2}) {model[:-3]}")
            exit(0)
        
        if target_model is None:
            model_path = existing_models[0]
        elif target_model in existing_models:
            model = target_model
        else:
            print("Model specified hasn't been downloaded. ")

        model_path = os.path.join(model_dir_path, model_path)
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

    elif command == 'list-models':
        print('|||'.join(MODELS.keys()))
    
    else:
        print(f"Error: Command `{command}` is invalid")
    