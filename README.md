# Shadingway

> Shadingway: Exposing ReShade internals to empower your tools.

-----

## Description

**Shadingway** is a ReShade addon designed to act as a bridge between ReShade and other external tools.  It doesn't modify or enhance ReShade's visual effects directly. Instead, **Shadingway exposes ReShade's configuration and runtime information**, allowing other applications and addons to access and utilize this data.

This addon is primarily intended for:

  * **Tool Developers:**  Create tools that integrate with ReShade, reacting to preset changes, effect toggles, and other ReShade states.
  * **Advanced ReShade Users:**  Build custom workflows and integrations that leverage ReShade's internal data for automation or advanced control.

Shadingway provides a way to programmatically access information that is normally only available within the ReShade UI, opening up possibilities for powerful integrations and extensions.

## Features

  * **Exposes Current ReShade Preset Path:**  Provides access to the file path of the currently active ReShade preset.
  * **Exposes Current ReShade Preset Folder:**  Reveals the directory where the active preset is located.
  * **Reports Global Effect Toggle State:**  Indicates whether ReShade effects are globally enabled or disabled.
  * **Provides Access to Effect Parameter Values:** (Future Feature - *optional to include if planned, otherwise remove*) Allows tools to read the current values of ReShade effect parameters (uniforms).
  * **Lightweight and Minimal Impact:** Designed to be efficient and have minimal performance overhead on ReShade and the game.

## Installation

1.  Ensure you have [ReShade](https://www.google.com/url?sa=E&source=gmail&q=https://www.google.com/url?sa=E%26source=gmail%26q=https://reshade.me/) already installed for your game.
2.  Download the latest release of Shadingway from the [Releases page](https://www.google.com/url?sa=E&source=gmail&q=https://www.google.com/url?sa=E%26source=gmail%26q=link-to-your-releases-page-here). (*Replace with your actual releases page link when available*)
3.  Extract the downloaded archive.
4.  Place the `Shadingway.dll` file into your ReShade "addons" folder. This folder is typically located in the same directory as your game executable, within the `reshade-shaders` folder (e.g., `[game_folder]\reshade-shaders\addons`).
5.  Ensure that addons are enabled in your `ReShade.ini` file.  Look for the `[GENERAL]` section and make sure `EffectSearchPaths` includes a path to the "addons" folder, and `PerformanceMode` is set to `0` or `1` (not `2`).

## How to Use

Shadingway itself does not have a user interface or directly perform any actions within ReShade or the game. Instead, it **works in the background, exposing ReShade data by writing to a JSON file.**

**Data Output:**

Shadingway writes ReShade information to a file named `shadingway-state.json`. This file is located in your game's installation folder (the same folder where your game executable is located).

**File Updates:**

The `shadingway-state.json` file is updated every frame while the game is running and ReShade is active.

**JSON File Content:**

The `shadingway-state.json` file contains a JSON object with the following key-value pairs:

```json
{
  "preset_path": "...",      // Path to the currently active ReShade preset file
  "preset_folder": "...",    // Folder containing the active preset
  "effects_enabled": true/false, // Global effects toggle state (true = enabled, false = disabled)
}
```

**For Tool Developers:**

1.  **Locate `shadingway-state.json`:** Your external tool or addon needs to locate the `shadingway-state.json` file in the game's installation folder. You can typically determine the game installation folder by finding the directory where the game executable is running.
2.  **Read and Parse JSON:**  Implement JSON parsing in your tool to read the contents of `shadingway-state.json`.
3.  **Access ReShade Information:**  Extract the desired ReShade configuration and runtime data from the parsed JSON object (e.g., `preset_path`, `effects_enabled`).
4.  **Utilize ReShade Information:**  Use the retrieved ReShade data within your tool to implement your desired functionality.

## Planned Features (Future Development)

  * **Expose More ReShade Settings:**  Expand the data exposed to include more ReShade configuration options and runtime states.
  * **Customizable Data Output:**  Allow users or tools to configure which data points are exposed and how they are formatted.
  * **Event-Based Data Updates:**  Implement event-driven data updates for more efficient real-time data access (e.g., trigger an event when the preset changes instead of constant polling).

## Contributing

Contributions are welcome\!  If you have ideas for new features, improvements, or bug fixes, please feel free to open issues or pull requests on this GitHub repository.

## Credits

  * Developed by [Your GitHub Username/Name]

-----

**Empower your tools with ReShade data using Shadingway\!**
