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

    thread maps\mp\gametypes\jmp::mapfixes();
}
```
