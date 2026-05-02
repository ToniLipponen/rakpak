I'm thinking about abstracting common build related things into a "build-options" section.

The reasoning behind this is that I want to get away from platform conditional inclusion of compiler/linker flags.

These could be defined on project level, profile level and target level.
Narrower scope inherits from the wider one, and overrides the wider one.

Might look something like this.
```json
"build-options": {
    "cpp-standard": 17,
    "c-standard": 11,
    "warnings": "all",
    "warnings-as-errors": true,
    "opt-level": 2,
    "lto": true,
    "arch": "native",
    "debug-info": false, 
}
```