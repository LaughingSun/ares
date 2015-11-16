# Announcement #

Hi all,

I'd like to announce the very first release of AresEd. Before getting too excited however I'd like to warn everyone that this is still an alpha release. It is far from a finished product and there are many known (and very likely unknown) bugs. It is also not feature complete.

# What? #

Now what is it? AresEd is a 3D game creation kit based on Crystal Space and CEL (http://www.crystalspace3d.org). It is Open Source (MIT license) and 100% free software. With AresEd it will be possible to make full working 3D games without any coding. All from within the AresEd visual editor. AresEd is **not** a modeller however. Before you can use AresEd you still have to use Blender (for example) to make your 3D models. AresEd can import these models and allows the game designer to place them in the world and additionally the game designer can define game logic.

# Game Logic #

Game logic is defined using state machines (the CEL quest concept) and the CEL message system. Objects in the world can be associated to 'entities' and entities can react to messages and in return send out other messages. With associated quests (state machines) you can 'program' the entity to behave different depending on various situations. In future versions of AresEd it will also be possible to attach python scripts to entities for more advanced game logic possibilities but the intention is that as much as possible can be done using the graphical quest editor.

# Missing Features #
As I said earlier, AresEd is not complete. Here is a list of a few major features that are still lacking:

  * There is as of yet no support for sound.
  * The actor system is not really powerful yet. For now AresEd is mostly uses for single player/first person view games.
  * The physics actor is not usable right now for real games as it gets stuck and unstable very easily. It is mainly included for demonstrations purposes and it needs a lot more work. Note that the simple collision detection based actor works much better and is suitable for small games.
  * The latest Google Summer of Code brought a new Bullet based physics plugin in Crystal Space and this one includes a nice physics based actor. Next version of AresEd will use that one.
  * Support for the CEL AI system and pathfinding.
  * Here is the list of known issues: http://code.google.com/p/ares/issues/list

But other than that it is actually possible to make simple games using AresEd. Included with this release there are five examples. Of these 'mysterygame' is the most game-like example. It includes a puzzle that you have to solve before a gate can be fully opened.

# Game Player #

In addition to AresEd this package also includes a game player called Ares. While you can play your game directly from within AresEd, if you want to distribute your game it will be easier to do that with Ares instead.

# Links #

Check the feature downloads on this page.

The linux binary is temporary. It was built on a 64-bit kubuntu and as such might not be usable on other distributions. In the near future we will provide more compatible binaries. In the mean time it might be easier to compile Ares from Source. You can find instructions on how to Build Ares here: [AresBuilding](AresBuilding.md)

# Info #

After installation there are two executables: AresEd and Ares. Pressing 'F1' in AresEd will bring up online help.

# Feedback #
Please let me know if you find any issues (that are not already listed in the list of issues above). Also if you have questions on how to use AresEd then feel free to ask me.

I'd like to stress that AresEd is in heavy development. This is an early version and it is likely that a lot will change in the future still.

Additionally I'm looking for people who want to help develop on AresEd. I think it is a very interesting project and there is lots of opportunity for other people to help with development.