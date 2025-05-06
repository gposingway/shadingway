# Shadingway v${VERSION}

> Shadingway: Exposing ReShade internals to empower your tools.

## Installation Instructions

1. **Requirements:**
   - [ReShade with Add-on Support](https://reshade.me/) must be already installed for your game
   - This addon requires ReShade 5.0 or newer

2. **Installation Steps:**
   - Download the `Shadingway${VERSION}.zip` file from this release
   - Extract the ZIP file to get the `shadingway.addon` file
   - Place the `shadingway.addon` file into your game's installation folder (the same folder where the game executable is located)
   - Start your game with ReShade enabled
   - The addon will automatically be loaded by ReShade

## What's New in This Release

${RELEASE_NOTES}

## Data Output

Shadingway writes ReShade information to a file named `shadingway.addon-state.json`. This file is located in your game's installation folder and contains:

```json
{
  "effects": {
    "enabled": true
  },
  "preset": {
    "collection": "Your Preset Collection",
    "name": "YourPresetName",
    "path": "C:\\Path\\To\\Your\\Game\\reshade-presets\\Collection\\Preset.ini"
  }
}
```

## Troubleshooting

- If the addon doesn't load, ensure your ReShade version supports addons
- If no JSON file is created, check that ReShade itself is working correctly
- For support, please open an issue on the GitHub repository

## Contributing

Contributions are welcome! If you have ideas for new features, improvements, or bug fixes, please open issues or pull requests on the GitHub repository.