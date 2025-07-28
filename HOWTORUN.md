# How to run the server & client
## Server
### Docker (recommended)
The repository builds & publishes master branch to a docker image. To use this server you'll have to simply run a container with the image that we publish.

Here's a quick set-up through **docker compose**.
```yaml
services:
 server:
   image: 'ghcr.io/story-of-alicia/alicia-server:latest'
   ports:
     - "10030-10033:10030-10033"
   volumes:
     - "./server:/var/lib/alicia-server"
```

#### Server configuration
By default, the game uses ports `10030`-`10033`. The default configuration of the server should be sufficient for development purposes. If not, contact us. Todo tutorial.

### Registering users (DATASOURCE FOR PROTOTYPE ONLY)
To add users to your server, open the data directory and create a **JSON** file for each user you want to register. Name of the file should be the username they'll use to log in. 

Example of a user file (`rgnter.json`):

```json5
// Filename: rgnter.json
{"name": "rgnter", "token": "myamazingpassword", "characterUid": 0}
```

## Client
### Client
[You can download the production game version installer from here]([https://drive.proton.me/urls/XX1GHBQGEW#WXILQk8JS08f](https://drive.proton.me/urls/37WM215Q1R#NlzxAZSg7VFC))([gdrive mirror](https://drive.google.com/file/d/1FMt4C1AmpP_8_1aItuYphvxWec9o2w6N/view)). Game contains some required patches (see Patches section).

The installator contains launcher and the game itself. The default installation directory is `%appdata%/Story of Alicia`. 

### Game configuration
There's three available game configurations in the production version:
- **development** (ID: `2`)
  - lobby server address: `aliciadev`
  - lobby server port: `10030`
- **prototype** (ID: `3`)
  - lobby server address: `prototype.storyofalicia.com`
  - lobby server port: `10030`
- **production** (ID: `4`)
  - lobby server address: `system.storyofalicia.com`
  - lobby server port: `10030`

Development configuration is meant for connecting to a server that is hosted hosted locally. Please notice the development configuration uses `aliciadev` hostname. You must add this hostname to your hosts file (`C:\Windows\System32\drivers\etc\hosts`) on the computer you're running the client on, and point it to an IP where the server is listening on. Usually that is going to be `127.0.0.1` (loopback).

Prototype configuration is meant for connecting to the online prototype. Currently hosted by @Chrommd, more information available at the [prototype's website](https://bruhvrum.github.io/registertest/).

Production configuration is meant for connecting to the officialy released server. Not available yet.

### Launching game in development configuration

Locate the launcher's configuration file ( `settings.json`). If the installer was used, this is usually `%appdata%/Story of Alicia` unless explicitly changed. If different path was used in the installer, you can consult the `HKCU\Software\Story Of Alicia\Default` registry key as it points to the location where the game was installed.

Modify the `executableArguments` field in the configuration file, specify the development game configuration and specify your user's credentials. This is done through `ID` and `OP` program arguments which indicate username and token respectively. Example:

```bash
game/Alicia.exe =GameID 3 -ID [<username>] -OP [<password>]
```

After saving the settings, simply launch the `alicia-launcher-cli.exe`

### Patches
- Disable hackshield
- Disable RcScrTxt localization limitations
- Allow character creator in every login mode
