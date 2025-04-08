Download the [Discord Social SDK](https://discord.com/developers/docs/discord-social-sdk/overview) and extract it to the
lib directory.

You should have a tree structure that looks like this once done:

```
lib
└── discord_social_sdk
    ├── bin
    │   ├── debug
    │   └── release
    ├── include
    └── lib
        ├── debug
        └── release
            └── discord_partner_sdk.xcframework
                ├── ios-arm64
                │   └── discord_partner_sdk.framework
                │       ├── Headers
                │       └── Modules
                └── ios-arm64-simulator
                    └── discord_partner_sdk.framework
                        ├── Headers
                        └── Modules
```