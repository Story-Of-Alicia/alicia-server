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
     - "./config:/opt/alicia-server/config"
     - "./data:/opt/alicia-server/data"
     - "./logs:/opt/alicia-server/logs"
```

#### Server configuration
By default the game uses ports `10030`-`10033`. The default configuration of the server should be sufficient for development purposes. If not, contact us. Todo tutorial.

### Registering users (DATASOURCE FOR PROTOTYPE ONLY)
To add users to your server, open the data directory and create a **JSON** file for each user you want to register. Name of the file should be the username they'll use to log in. 

Example of a user file (`rgnter.json`):

```json5
// Filename: rgnter.json
{"name": "rgnter", "token": "myamazingpassword", "characterUid": 0}
```

## Client
### Client
To connect to the server, you'll need patched version of the client. This is because we had to patch some undesirable behaviour. See patches section.

[You can download the production version from here](https://drive.proton.me/urls/XX1GHBQGEW#WXILQk8JS08f).

### Configuring game
There's two game available game configurations in the production version:
- **development**
  - lobby server address: `aliciadev`
  - lobby server port: `10030`
- **production**
  - lobby server address: `system.storyofalicia.com`
  - lobby server port: `10030`

Please notice the development server's configuration uses `aliciadev` hostname. You must add this hostname to your hosts file (`C:\Windows\System32\drivers\etc\hosts`) and point it to an IP where the server is listening on. Usually that is going to be `127.0.0.1` (loopback).

You can't configure the game directly, unless you have the development version of the game. Todo tutorial.

### Launching game
To launch the game:

- You must tell the client the game configuration.
  - Configure the game configuration in the launcher's `settings.json` (located in the game folder). 
  - To launch the game in development, use the `-GameID 3` switch.
  - To launch the game in production, use the `-GameID 4` switch.
- You must tell the client your credentials.
  - Configure these game credentials in the launcher's `settings.json` (located in the game folder).
  - Change `webInfoContent.LoginId` to your username.
  - Change `webInfoContent.AuthKey` to your password.

Once all of this is set, you can simply run the launcher as Administrator and enjoy your game!

### Patches
- Disable hackshield
- Disable RcScrTxt localization limitations
- Allow character creator in every login mode
