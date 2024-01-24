<!--
SPDX-FileCopyrightText: 2023 thecheeseman

SPDX-License-Identifier: CC0-1.0
-->

# json2gsc

JSON file parser/converter utility for the CoD1 JumpIt jump mod.

It takes `json` files that contain individual map settings and converts them
to a `gsc` format that the jump mod is expecting.

## Requires

- C++20
- [json](https://github.com/nlohmann/json)
- [argh!](https://github.com/adishavit/argh)

## Usage

```bash
json2gsc [input folder] [out file]

by default [input folder] = "mapconfigs"
           [out file] = "settings.gsc"

$ json2gsc cfgs my_settings.gsc
```

## Build

```bash
git clone --recurse-submodules https://github.com/thecheeseman/json2gsc

cd json2gsc
mkdir Build

cmake . -B Build
cmake --build Build
```

## Example

For example, the file `jm_pillz.json`

```json
{
  "map": {
      "name": "jm_pillz"
  },
  "timelimit": 12,
  "difficulty": "medium",
  "healspots": [
    [-1083, -37, -207],
    [880, -1034, -195]
  ],
  "grenadespots": [
    [-1084, 286, -207]
  ],
  "panzerspots": [
    [983, -527, -303]
  ],
  "disablesavespots": [
    [-166, 400, 70, 90, 448, 200]
  ],
  "enablesavespots": [
    [90, 378, 70, -166, 326, 200]
  ]
}
```

Will be parsed and then spliced into the `settings.gsc` file:

```c
set_level_difficulties()
{
    // ...
    level.difficulty["jm_pillz"] = "medium";
    // ...
}

map_setup()
{
    mapame = toLower(level.mapname);
    switch(mapname) {
        // ...
        case "jm_pillz":
            level.maptitle = "jm_pillz";
            level.mapauthor = "unknown";
            level.timelimit = 12;
            healspots[0] = (-1083, -37, -207);
            healspots[1] = (880, -1034, -195);
            grenadespots[0] = (-1084, 286, -207);
            panzerspots[0] = (983, -527, -303);
            save_disable_aabb[0][0] = (-166, 400, 70);
            save_disable_aabb[0][1] = (90, 448, 200);
            save_enable_aabb[0][0] = (-166, 326, 70);
            save_enable_aabb[0][1] = (90, 378, 200);
        break;
        // ...
    }

    level.mapsettings = [];

    if(isDefined(ladderjumps))
        level.mapsettings["ladderjumps"] = ladderjumps;
    if(isDefined(healspots))
        level.mapsettings["healspots"] = healspots;
    if(isDefined(grenadespots))
        level.mapsettings["grenadespots"] = grenadespots;
    if(isDefined(panzerspots))
        level.mapsettings["panzerspots"] = panzerspots;
    if(isDefined(save_disable_aabb))
        level.mapsettings["save_disable_aabb"] = save_disable_aabb;
    if(isDefined(save_enable_aabb))
        level.mapsettings["save_enable_aabb"] = save_enable_aabb;

    thread maps\mp\gametypes\jmp::mapfixes();
}
```

The AABB vertices that define save enable/disable areas will also be redefined if needed so the first vetex is closest to the origin and the second one is the farthest.
