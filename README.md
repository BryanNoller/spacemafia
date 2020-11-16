# spacemafia

injectable shared library

## Included Dependencies

[kiero](https://github.com/Rebzzel/kiero) - Universal graphical hook
* [1.2.10](https://github.com/Rebzzel/kiero/archive/9c60cb381cf4ef8b46eea2e1b1b2a2b9afb63076.zip) source code

[Dear ImGui](https://github.com/ocornut/imgui) - Graphical User interface
* [v1.79](https://github.com/ocornut/imgui/archive/v1.79.zip) source code

## Notes

Currently configured for DirectX 11

Dependencies included are unmodified, except for having [KIERO_INCLUDE_D3D11](include/kiero/kiero.h#L10) and [KIERO_USE_MINHOOK](include/kiero/kiero.h#L14) config options set in [kiero.h](include/kiero/kiero.h)
