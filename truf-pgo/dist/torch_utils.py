import torch
from torch import nn
import numpy as np
from transformers.models.distilbert.modeling_distilbert import DistilBertModel
from transformers.models.bert.modeling_bert import BertModel
import sys

class FlagModel(nn.Module):
    def __init__(
            self, 
            qwen_model: DistilBertModel | BertModel, 
            num_flags: int, 
            *args, 
            **kwargs,
        ) -> None:
        super().__init__(*args, **kwargs)

        self.fc_in = nn.Linear(4096, 512)
        self.language_model = qwen_model
        self.fc1 = nn.Linear(768, 32)
        self.fc2 = nn.Linear(32*512, 32)

        self.out = nn.Linear(32, num_flags)

    def forward(self, tokens: dict) -> torch.Tensor:
        tsr = self.fc_in(tokens['input_ids'].to(torch.float32))
        tsr = torch.relu(tsr)
        tsr  = tsr.to(torch.long)

        tsr = self.language_model(input_ids=tsr, attention_mask=torch.ones_like(tsr)).last_hidden_state

        tsr = self.fc1(tsr)
        tsr = torch.relu(tsr)
        tsr = torch.flatten(tsr, start_dim=1)

        tsr = self.fc2(tsr)
        tsr = torch.relu(tsr)

        tsr = self.out(tsr)

        return tsr

class InferenceUtils:
    FLAGS = [
        '-O3',
        '-falign-functions=32',
        '-falign-loops=32',
        '-ffast-math',
        '-fipa-pta',
        '-flto',
        '-fmerge-all-constants',
        '-fno-math-errno',
        '-fomit-frame-pointer',
        '-fprefetch-loop-arrays',
        '-ftree-vectorize',
        '-funroll-loops',
        '-funsafe-math-optimizations',
        '-fwhole-program',
        '-march=native',
        '-mtune=native'
    ]

    @staticmethod
    def format_prompt(
        c_source: str,
        cpu: str,
        arch: str,
        os: str,
        compiler: str,
    ) -> str:
        return f"CPU:{cpu}\nARCH: {arch}\nOS: {os}\nCompiler: {compiler}\n\n\n\nSource Code:\n{c_source}"
    
    @staticmethod
    def tokenize(tokenizer, text: str | list[str]) -> dict[str, torch.Tensor]:
        if isinstance(text, str):
            text = [text]
        
        tokens = tokenizer(text, padding="max_length", max_length=4096, return_tensors="pt")
        return tokens

    @staticmethod
    def gen_flags(model_output: torch.Tensor) -> list[str]:
        if len(model_output.size()) > 1 :
            model_output = torch.flatten(model_output)
        
        model_output = torch.sigmoid(model_output)

        return [
            f
            for i, f in zip(model_output, InferenceUtils.FLAGS)
            if i >= 0.5
        ]

    @staticmethod
    def onhot_encode(flags: str | list[str]) -> np.ndarray:
        if isinstance(flags, str):
            flags = flags.split()

        one_h = np.zeros([len(InferenceUtils.FLAGS)])

        for i, f in enumerate(InferenceUtils.FLAGS):
            if f in flags:
                one_h[i] = 1

        return one_h