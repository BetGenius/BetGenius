# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.4.1] - 2023-12-21

#### Changed
 - The point multiplication algorithm used for ECDH operations (module `ecdh`) was replaced with a slightly faster one.
 - Optional handwritten x86_64 assembly for field operations was removed because modern C compilers are able to output more efficient assembly. This change results in a significant speedup of some library functions when handwritten x86_64 assembly is enabled (`--with-asm=x86_64` in GNU Autotools, `-DSECP256K1_ASM=x86_64` in CMake), which is the default on x86_64. Benchmarks with GCC 10.5.0 show a 10% speedup for `secp256k1_ecdsa_verify` and `secp256k1_schnorrsig_verify`.

#### ABI Compatibility
The ABI is backward compatible with versions 0.4.0 and 0.3.x.

[0.4.1]: https://github.com/BetGenius/secp256k1/releases/v0.4.1
