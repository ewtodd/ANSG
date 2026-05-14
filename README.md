# ANSG (Applied Nuclear Science Group) Analysis
<!---->
## Projects
<!---->
- **73mGe** — Precision gamma-ray measurements of 73mGe using a state-of-the-art position sensitive CZT detector.
- **78mBr** — Half-life measurement of 78mBr by self-coincidence analysis in a LaBr3 detector.
- **YAG-PreProcessing** — Pre-processing of YAG detector data.
- **YAP-PSD** — Machine learning-based pulse shape discrimination in YAP:Ce, comparing Random Forest, Gradient Boosting, XGBoost, and MLP models.
<!---->
## Dependencies
<!---->
All projects depend on [Analysis-Utilities](https://github.com/ewtodd/Analysis-Utilities), a shared library providing common ROOT-based analysis tools.
<!---->
## Reproducibility
<!---->
This project uses [Nix](https://nixos.org/) to manage dependencies.
Install it by following the instructions [here](https://nixos.org/download/).
Ensure [flakes are enabled](https://nixos.wiki/wiki/Flakes#Enable_flakes_permanently) in your Nix configuration.
All directories contain their own Nix flake, which can be used to re-run code with the exact same environment as I did.
If projects reference a local version of Analysis-Utilities in the flake, they are not yet finished.
Contact me for access to data.
