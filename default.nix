{
  lib,
  stdenv,
  freetype,
  fontconfig,
  libGL,
  wayland,
  wayland-protocols,
  libxkbcommon,
  zig_0_13,
  pkg-config,
  scdoc,
  installShellFiles,
  lua,
}:
stdenv.mkDerivation (finalAttrs: {
  pname = "eeyelop";
  version = "0.1.0";

  src = ./.;

  dontConfigure = true;
  dontInstall = true;
  doCheck = false;

  nativeBuildInputs = [
    zig_0_13
    wayland
    wayland-protocols
    libGL
    libxkbcommon
    freetype
    fontconfig
    lua
  ];

  buildInputs = [
    pkg-config
    scdoc
    installShellFiles
  ];

  buildPhase = ''
    mkdir -p .cache
    zig build install --cache-dir $(pwd)/.zig-cache --global-cache-dir $(pwd)/.cache -Dcpu=baseline -Doptimize=ReleaseSafe --prefix $out
  '';

  meta = with lib; {
    description = "Fancy keyboard driven notification daemon";
    mainProgram = "eeyelop";
    homepage = "https://github.com/unixpariah/eeyelop";
    license = licenses.mit;
    maintainers = with maintainers; [unixpariah];
    platforms = platforms.unix;
  };
})
