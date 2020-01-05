# KeyMouse
**KeyMouse** is a program which works like the chrome plugin **vimium**, but is designed for other programs(like windows file browser).

![demo](https://i.imgur.com/HxaxNYu.gif)

## Usage

- Select mode: Press <kbd>Alt</kbd> + <kbd>;</kbd> to enter select mode, then press tag to select item.
- Fast Select mode: <kbd>Alt</kbd> + <kbd>J</kbd> to enter fast select mode. This mode may fail in some situations.
- Esc Select mode: Press <kbd>Esc</kbd> to esc.
- Disable: Press <kbd>Alt</kbd> + <kbd>[</kbd> to enable/disable the program.
- Scroll down: Press <kbd>J</kbd>.
- Scroll up: Press <kbd>K</kbd>.

> if KeyMouse doesn't work, try to run as Administrator.

## Configration
**KeyMouse** supports configuration for hot keys and profile.you can put your configuration in `config.json`(put in the path the same as `KeyMouse.exe`). there is a typical configuration:
```
{
    "profile":
    {
        "runOnStartUp": false,
        "backgroundColor": "#CCFFCC",
        "fontColor": "#000000",
        "fontSize": 10,
        "font": "Arial Rounded MT Bold"
    },
    "keybindings":
    {
        "toggleEnable": "alt+[",
        "scrollUp": "k",
        "scrollDown": "j",
        "selectMode": "alt+;",
        "escape": "esc",
        "fastSelectMode": "alt+j"
    }
}
```
### options
The options of profile is listed below:

| Options       | Type           | Default  | Description|
| ------------- | ------------- | ----------- |----------|
| runOnStartUp | bool | `false` |When this option is `true`, the app will be launched when windows starts up.|
| backgroundColor| string| `#CCFFCC` |The background color of hints. Use RGB format|
| fontColor| string| `#000000` |The font color of hint. Use RGB format|
| fontSize| int| `10`| The font size of hint. |
| font| string| `Arial Rounded MT Bold`| The font type of hint. You can find the font name supported by your system through **Settings->Personalization->fonts**. Make sure to use the full name of font.|

The hot keys support `alt`, `shift`, `ctrl`, `win` and most keys on the keyboard. Please use lowercase and use `+` to connect different keys. some typical keybings: `alt+j`, `shift+alt+j`, `f11`.

The opttion of hot keys is listed below:

| Options       | Type           | Default  | Description|
| ------------- | ------------- | ----------- |----------|
| toggleEnable| string| `alt+[`| Enable/Disable this app.|
| scrollUp| string| `k`| Scroll up.|
| scrollDown| string| `j`| Scroll down.|
| selectMode| string| `alt+;`| Enter select mode.|
| fastSelectMode| string| `alt+j`| Enter fast select mode. the difference between select mode and fast select mode is that FSM will continually enter select mode. may fail in some situations.|
|escape| string| `esc`| Leave select mode.|


## Build
### Prerequisities
1. require json lib form [nlohmann/json](https://github.com/nlohmann/json).

you can either add `--recurse-submodules` option when cloning this repo or get a copy from `nlohmann/json/single_include/nlohmann/json.hpp` to `/KeyMouse/KeyMouse/json/single_include/nlohmann/json.hpp`.
### Step
1. run `git clone --recurse-submodules https://github.com/iscooool/KeyMouse.git`.
2. Install Visual Studio and Import this project.
3. Ensure to build this project in x64.
