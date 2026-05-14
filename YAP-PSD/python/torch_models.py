"""GPU-accelerated regressors backed by PyTorch.

Exposes sklearn-style classes (``fit`` / ``predict``) so the rest of the
analysis pipeline (`regress_waveforms`, parameter sweeps, etc.) is unchanged.
"""
import numpy as np
import torch
import torch.nn as nn


class _TorchRegressorBase:
    """Shared training/prediction loop. Subclasses define the architecture
    and (optionally) override how inputs are reshaped for the model."""

    def __init__(self, max_iter, random_state, verbose, batch_size,
                 learning_rate, device):
        self.max_iter = max_iter
        self.random_state = random_state
        self.verbose = verbose
        self.batch_size = batch_size
        self.learning_rate = learning_rate
        self.device = device
        self.model_ = None
        self.n_features_ = None

    def _resolve_device(self):
        if self.device is not None:
            return torch.device(self.device)
        return torch.device("cuda" if torch.cuda.is_available() else "cpu")

    def _build_model(self, n_features):
        raise NotImplementedError

    def _to_input_tensor(self, X, device):
        return torch.from_numpy(np.ascontiguousarray(X,
                                                     dtype=np.float32)).to(device)

    def fit(self, X, y):
        torch.manual_seed(self.random_state)
        np.random.seed(self.random_state)

        device = self._resolve_device()
        X = np.asarray(X, dtype=np.float32)
        y = np.asarray(y, dtype=np.float32).reshape(-1, 1)
        self.n_features_ = X.shape[1]

        self.model_ = self._build_model(self.n_features_).to(device)
        optimizer = torch.optim.Adam(self.model_.parameters(),
                                     lr=self.learning_rate)
        loss_fn = nn.MSELoss()

        X_t = self._to_input_tensor(X, device)
        y_t = torch.from_numpy(y).to(device)
        n = len(X_t)

        self.model_.train()
        for epoch in range(self.max_iter):
            idx = torch.randperm(n, device=device)
            running = 0.0
            for start in range(0, n, self.batch_size):
                b = idx[start:start + self.batch_size]
                optimizer.zero_grad()
                loss = loss_fn(self.model_(X_t[b]), y_t[b])
                loss.backward()
                optimizer.step()
                running += loss.item() * len(b)
            if self.verbose:
                print(f"  epoch {epoch + 1}/{self.max_iter}  "
                      f"loss={running / n:.6f}")
        return self

    def predict(self, X):
        device = next(self.model_.parameters()).device
        X = np.asarray(X, dtype=np.float32)
        self.model_.eval()
        out = []
        with torch.no_grad():
            for start in range(0, len(X), 4096):
                xb = self._to_input_tensor(X[start:start + 4096], device)
                out.append(self.model_(xb).cpu().numpy().reshape(-1))
        return np.concatenate(out)

    # Pickle the architecture spec + a CPU state_dict so trained models round-
    # trip cleanly across processes / machines, with or without a GPU present.
    def __getstate__(self):
        state = self.__dict__.copy()
        if self.model_ is not None:
            state["_model_state_dict"] = {
                k: v.cpu()
                for k, v in self.model_.state_dict().items()
            }
            state["model_"] = None
        return state

    def __setstate__(self, state):
        sd = state.pop("_model_state_dict", None)
        self.__dict__.update(state)
        if sd is not None and self.n_features_ is not None:
            self.model_ = self._build_model(self.n_features_)
            self.model_.load_state_dict(sd)
            self.model_.to(self._resolve_device())


class TorchMLPRegressor(_TorchRegressorBase):
    """Fully-connected feedforward regressor."""

    def __init__(self,
                 hidden_layer_sizes=(128, 64),
                 max_iter=500,
                 random_state=42,
                 verbose=False,
                 batch_size=256,
                 learning_rate=1e-3,
                 device=None):
        super().__init__(max_iter=max_iter,
                         random_state=random_state,
                         verbose=verbose,
                         batch_size=batch_size,
                         learning_rate=learning_rate,
                         device=device)
        self.hidden_layer_sizes = tuple(hidden_layer_sizes)

    def _build_model(self, n_features):
        layers = []
        in_dim = n_features
        for h in self.hidden_layer_sizes:
            layers.append(nn.Linear(in_dim, h))
            layers.append(nn.ReLU())
            in_dim = h
        layers.append(nn.Linear(in_dim, 1))
        return nn.Sequential(*layers)


class TorchCNNRegressor(_TorchRegressorBase):
    """1D convolutional regressor for waveform inputs.

    Treats each row of X as a 1-channel signal of length ``n_features``.
    Stacks Conv1d(+ReLU+MaxPool) blocks, then a small MLP head.
    """

    def __init__(self,
                 conv_channels=(16, 32, 64),
                 kernel_size=5,
                 fc_sizes=(64, ),
                 dropout=0.2,
                 max_iter=50,
                 random_state=42,
                 verbose=False,
                 batch_size=256,
                 learning_rate=1e-3,
                 device=None):
        super().__init__(max_iter=max_iter,
                         random_state=random_state,
                         verbose=verbose,
                         batch_size=batch_size,
                         learning_rate=learning_rate,
                         device=device)
        self.conv_channels = tuple(conv_channels)
        self.kernel_size = kernel_size
        self.fc_sizes = tuple(fc_sizes)
        self.dropout = dropout

    def _build_model(self, n_features):
        layers = []
        in_ch = 1
        cur_len = n_features
        for c in self.conv_channels:
            if cur_len < 4:
                # Signal too short to keep pooling without going to length 0.
                break
            layers.append(
                nn.Conv1d(in_ch,
                          c,
                          kernel_size=self.kernel_size,
                          padding=self.kernel_size // 2))
            layers.append(nn.ReLU())
            layers.append(nn.MaxPool1d(2))
            cur_len //= 2
            in_ch = c
        layers.append(nn.Flatten())
        flat_dim = in_ch * cur_len
        for h in self.fc_sizes:
            layers.append(nn.Linear(flat_dim, h))
            layers.append(nn.ReLU())
            if self.dropout > 0:
                layers.append(nn.Dropout(self.dropout))
            flat_dim = h
        layers.append(nn.Linear(flat_dim, 1))
        return nn.Sequential(*layers)

    def _to_input_tensor(self, X, device):
        X = np.ascontiguousarray(X, dtype=np.float32)
        return torch.from_numpy(X).unsqueeze(1).to(device)
