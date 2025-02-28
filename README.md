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

  * **Exposes Current ReShade Preset data:**  Provides access to the file path of the currently active ReShade preset.
  * **Reports Global Effect Toggle State:**  Indicates whether ReShade effects are globally enabled or disabled.
  * **Lightweight and Minimal Impact:** Designed to be efficient and have minimal performance overhead on ReShade and the game.

## Installation

1.  Ensure you have [ReShade with Add-on Support](https://reshade.me/) already installed for your game.
2.  Download the latest release of Shadingway from the [Releases page](https://www.google.com).
3.  Extract the downloaded archive.
4.  Place the `Shadingway.addon` file into your ReShade `addons` folder. This folder is typically located in the same directory as your game executable, within the `reshade-shaders` folder (e.g., `[game_folder]\reshade-shaders\addons`).

## How to Use

Shadingway itself does not have a user interface or directly perform any actions within ReShade or the game. Instead, it **works in the background, exposing ReShade data by writing to a JSON file.**

**Data Output:**

Shadingway writes ReShade information to a file named `shadingway.addon-state.json`. This file is located in your game's installation folder (the same folder where your game executable is located).

**File Updates:**

The `shadingway.addon-state.json` file is updated every frame while the game is running and ReShade is active.

**JSON File Content:**

The `shadingway.addon-state.json` file contains a JSON object with the following key-value pairs:

```json
{
  "effects": {
    "enabled": true
  },
  "preset": {
    "collection": "Witch's Presets",
    "name": "Witch'sMoonForGameplay",
    "path": "C:\\Games\\SquareEnix\\FINAL FANTASY XIV - A Realm Reborn\\game\\reshade-presets\\Witch's Presets\\Witch'sMoonForGameplay.ini"
  }
}
```

## Contributing

Contributions are welcome\!  If you have ideas for new features, improvements, or bug fixes, please feel free to open issues or pull requests on this GitHub repository.
