with import <nixpkgs> {};

let

cov_test = (target: env: env.mkDerivation {
  name = "rodos-${target}";
  src = ./.;
  buildInputs = [ cmake lcov ninja ];
  cmakeFlags = [
    "-DCMAKE_TOOLCHAIN_FILE=../cmake/port/${target}.cmake"
    "-DEXECUTABLE=ON"
    "-DCOVERAGE=ON"
    "-DCMAKE_BUILD_TYPE=Debug"
  ];
  installPhase =
    ''
      ninja coverage_collect test-report
      mkdir $out
      mv coverage $out/
      mv coverage.info $out/
      mv test-suite/test-report.txt $out/
    '';
});

compile_only = (target: env: env.mkDerivation {
  name = "rodos-${target}";
  src = ./.;
  nativeBuildInputs = [ cmake ninja gcc-arm-embedded ];
  cmakeFlags = [
    "-DCMAKE_TOOLCHAIN_FILE=../cmake/port/${target}.cmake"
    "-DEXECUTABLE=ON"
  ];
  installPhase =
    ''
      mkdir $out
      touch $out/did-compile
    '';
});

in

{
  linux-x86 = cov_test "linux-x86" pkgsi686Linux.stdenv;
  linux-makecontext = cov_test "linux-makecontext" pkgsi686Linux.stdenv;
  posix = cov_test "posix" pkgsi686Linux.stdenv;
  discovery = compile_only "discovery" pkgs.gcc-arm-embedded.stdenv;
  discovery_f429 = compile_only "discovery_f429" pkgs.gcc-arm-embedded.stdenv;
  skith = compile_only "skith" pkgs.gcc-arm-embedded.stdenv;
  raspberrypi3 = compile_only "raspberrypi3" pkgs.gcc-arm-embedded.stdenv;
  sf2 = compile_only "sf2" pkgs.gcc-arm-embedded.stdenv;
  nucleo_l496zg = compile_only "nucleo_l496zg" pkgs.gcc-arm-embedded.stdenv;
  va41620 = compile_only "va41620" pkgs.gcc-arm-embedded.stdenv;
}
