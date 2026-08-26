{...}: {
  projectRootFile = "flake.nix";
  programs.alejandra.enable = true;
  programs.clang-format.enable = true;
  programs.prettier.enable = true;
  settings.global.excludes = [
    "*.md"
    "*.ttf"
    "*.txt"
    ".envrc"
    ".gitignore"
    ".clang-format"
  ];
}
