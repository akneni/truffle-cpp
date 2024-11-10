### Commands
- Note, this is not a CLI tool intendend by users. It is intended to be used by TrufC.
1. **`inference`**  
   Runs inference on a specified source code directory using a downloaded model. This command processes source code files in the directory, identifies the target model (or uses a default if none is specified), and outputs recommended compilation flags.

   **Usage:**
   ```bash
   python main.py inference --app-data-path <path_to_app_data> --source-path <path_to_source_code> --model <model_name.pt>
   ```

   **Arguments:**
   - `--app-data-path`: Path to the application's data directory, where model files are stored.
   - `--source-path`: Path to the directory containing the source code files for analysis.
   - `--model`: (Optional) Name of the model to use for inference. If not specified, the tool uses the first downloaded model.

   **Example:**
   ```bash
   python main.py inference --app-data-path ./app_data --source-path ./source_code --model custom_model.pt
   ```

2. **`pull`**  
   Downloads a model specified by the user. The model file will be saved to the `models` directory within the specified application data path.

   **Usage:**
   ```bash
   python main.py pull --app-data-path <path_to_app_data> --model <model_name>
   ```

   **Arguments:**
   - `--app-data-path`: Path to the application's data directory, where model files will be saved.
   - `--model`: Name of the model to download. The model name should be a valid identifier from the available model list (checked via the `list-models` command).

   **Example:**
   ```bash
   python main.py pull --app-data-path ./app_data --model custom_model
   ```

3. **`list-models`**  
   Lists all available models that can be downloaded and used with this tool.

   **Usage:**
   ```bash
   python main.py list-models
   ```

   **Example Output:**
   ```plaintext
   model1|||model2|||model3
   ```

