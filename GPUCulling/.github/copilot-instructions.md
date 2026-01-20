# Copilot Instructions

## General Guidelines
- You are expected to follow the coding standards and best practices established by the development team.
- You are making a game engine with focus on rendering features and performance.
- Prefer modern C++ features and idioms where appropriate.
- Prefer assert and assertm for error checking instead of exceptions and if checks at the start of the file. However, use if checks in situations where recovery is possible.
- Avoid using raw pointers; prefer smart pointers (e.g., `std::unique_ptr`, `std::shared_ptr`) for memory management.

## Code Style
- Member variables should be prefixed with `m_`.
- Static variables should be prefixed with `s_`.
- Use camelCase for variable and function names.
- Constant variables should be in all uppercase with underscores.
- Follow naming conventions

## Project-Specific Rules
- For C++ source files in this project, use `#include "stdafx.h"` instead of `#include "../stdafx.h"` at the top of files. The project is configured to find `stdafx.h` from the include paths, so the relative path `../` is not needed.