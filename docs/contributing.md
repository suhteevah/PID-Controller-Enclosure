# Contributing to ESP-Nail

## How to Contribute

We welcome contributions of all kinds:

- **Firmware**: Bug fixes, new features, optimizations
- **Enclosure designs**: 3D printable remixes, new materials
- **Documentation**: Guides, translations, tutorials
- **Testing**: Hardware testing, bug reports, PID tuning data
- **Community**: Build photos, profile presets, coil recommendations

## Development Setup

1. Install [PlatformIO](https://platformio.org/) (VS Code extension recommended)
2. Clone the repo: `git clone https://github.com/suhteevah/PID-Controller-Enclosure.git`
3. Open `firmware/v2/` in VS Code with PlatformIO
4. Build: `pio run -e single`
5. Run tests: `pio test -e test`

## Pull Request Process

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/my-feature`
3. Make your changes
4. Run tests: `pio test -e test`
5. Commit with descriptive messages
6. Push and open a Pull Request
7. Describe what changed and why

## Code Style

- C++11, Arduino framework
- 4-space indentation (no tabs)
- Opening brace on same line: `if (x) {`
- `_camelCase` for private members
- `CamelCase` for classes and enums
- `UPPER_SNAKE` for constants and macros
- Header guards: `#pragma once`
- Keep functions under 50 lines when possible

## Commit Messages

- Use imperative mood: "Add WiFi manager" not "Added WiFi manager"
- First line: brief summary (under 72 chars)
- Body: explain *why*, not just *what*

## Reporting Bugs

Open an issue with:
- Model and firmware version
- Steps to reproduce
- Expected vs actual behavior
- Serial output (if relevant)
- Photos (if hardware issue)

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
