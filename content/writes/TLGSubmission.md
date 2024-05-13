+++
title = 'TLG Submission'
date = 2024-05-09T10:05:31+07:00
toc = false
+++

This blog post is a daily journal on setting up and finishing the test from Tavernlight Games

The test are generally broken down to two parts, first is testing your lua & cpp knowledge (Q1-Q4), the second part is creating feature on the forgotten server game (Q5-Q7). The person at Tavernlight Games were kind enough to provide with two links [TFS server](https://github.com/otland/forgottenserver), and [OTC client](https://github.com/edubart/otclient) to play the game. 

## First Part Q1 - Q4
My knowledge on Lua leans more to practical scripting, I never investigate how it works under the hood, so before taking the task, I need to refresh up my Lua knowledge and also investigate Lua performance characteristics. From the looks of the question, I was expected to fix / improve the Lua implementation. So I dabbled around on Lua resource on the net for a refresher, to name a few http://lua-users.org/wiki/OptimisationTips and https://www.lua.org/gems/sample.pdf

### Q1
```lua
local function releaseStorage(player)
   player:setStorageValue(1000, -1)
end

function onLogout(player)
   if player:getStorageValue(1000) == 1 then
      addEvent(releaseStorage, 1000, player)
   end
   return true
end
```

At first I thought that **addEvent** second parameter is also the parameter that is passed as an input to releaseStorage, since the 1000 values looks like a login status value. But after checking on the TLS source code, turns out the second param of **addEvent** is used as a delay in milliseconds, and player is the only parameter that is passed around. So I tried to make it clear that it is a player storage key and add a enum value so future keys could go into the **PlayerStorageKeys**

```lua
-- In this context, assuming this is a login logout logic so 1000 should be used as loginStatus
PlayerStorageKeys = {
   loginStatus = 1000,
   -- other keys for player can go below..
}

local function releaseStorage(player)
   player:setStorageValue(PlayerStorageKeys.loginStatus, -1)
end

function onLogout(player)
   if player:getStorageValue(PlayerStorageKeys.loginStatus) == 1 then
      addEvent(releaseStorage, 1000, player)
   end

   return true
end
```

### Q2
```lua
function printSmallGuildNames(memberCount)
   -- this method is supposed to print names of all guilds that have less than memberCount max members
   local selectGuildQuery = "SELECT name FROM guilds WHERE max_members < %d;"
   local resultId = db.storeQuery(string.format(selectGuildQuery, memberCount))
   local guildName = result.getString("name")
   print(guildName)
end
```

I'm not sure if the above query is correct, since I can't seem to find **max_members** on the guilds table using TFS 1.4 schema. The only available columns is id, **name**, **ownerid**, **creationdata**, **motd**, **description**, **guild_logo**, **create_ip**, and **balance**. But assuming **max_members** is available, the function become like this.

```lua
function printSmallGuildNames(memberCount)
   assert(type(memberCount) == "number")

   local resultId = db.storeQuery(string.format("SELECT `name` FROM `guilds` WHERE `max_members` < %d", memberCount))
   if not resultId then
      print("No guild found with member less than " .. tostring(memberCount))
   else
      print("Guild names")
      repeat
         local guildName = result.getString(resultId, "name")
         print(guildName)
      until not result.next(resultId)
   end

   result.free(resultId)
end
```

### Q3
```lua
function do_sth_with_PlayerParty(playerId, membername)
   player = Player(playerId)
   local party = player:getParty()

   for k, v in pairs(party:getMembers()) do
      if v == Player(membername) then
         party:removeMember(Player(membername))
      end
   end
end
```

I renamed the function to make it clear what the function really is doing. Changed it into camelCase camelCase, some lua style guide use snake_case because of standard library but I use camelCase to make it consistent with the previous question. **party:getMembers** return array, so we need to check with **ipairs**. Lastly we remove the player from the party based on the **membername**

```lua
function removePlayerFromParty(playerId, membername)
   local player = Player(playerId)
   local party = player:getParty()

   if party then
      for _, member in ipairs(party:getMembers()) do
         local checkName = member:getName()
         if checkName == memberName then
            party:removeMember(Player(membername))
         end
      end
   end
end
```

### Q4
```cpp
void Game::addItemToPlayer(const std::string &recipient, uint16_t itemId) {
  Player *player = g_game.getPlayerByName(recipient);
  if (!player) {
    player = new Player(nullptr);
    if (!IOLoginData::loadPlayerByName(player, recipient)) {
      return;
    }
  }

  Item *item = Item::CreateItem(itemId);
  if (!item) {
    return;
  }

  g_game.internalAddItem(player->getInbox(), item, INDEX_WHEREEVER,
                         FLAG_NOLIMIT);

  if (player->isOffline()) {
    IOLoginData::savePlayer(player);
  }
}
```

There are two answer that I tried on this question. One is deleting the player instance if **getPlayerByName** returning invalid ptr. The second is passing local player to "fill" values, so we don't need to allocate, this is a lot of assumptions given the above logic corner case handle giving item to an offline player. Most of the reason are commented below with the answers.

```cpp
// Answer 1
void Game::addItemToPlayer(const std::string &recipient, uint16_t itemId) {
  Player *player = g_game.getPlayerByName(recipient);
  if (!player) {
    player = new Player(nullptr);
    if (!IOLoginData::loadPlayerByName(player, recipient)) {
      delete player;
      return;
    }
  }

  Item *item = Item::CreateItem(itemId);
  if (!item) {
    return;
  }

  // Not sure if item also allocates after CreateItem above, but if it is, then
  // it *might* need to be freed if this function internalAddItem don't take
  // ownership and simply copies item
  g_game.internalAddItem(player->getInbox(), item, INDEX_WHEREEVER,
                         FLAG_NOLIMIT);

  if (player->isOffline()) {
    // From looking at the logic of the flow from the beginning of the function
    // it seems to assume that if player is logged in and online, it would never
    // allocate (g_game.getPlayerByName would return valid online player).
    // Hence deleting the created player object only happens if player isOffline
    // but at that point, we could use loadPlayerByName to cache bool value so
    // we don't check if player->isOffline again, assuming loadPlayerByName also
    // returns logged in user Not really sure about the flow of the function
    // here.. so at minimum player should be deleted here if it's deleted
    // outside here, it would risk always creating player object if the player
    // is online
    IOLoginData::savePlayer(player);
    delete player;
  }

  // Should be enabled if internalAddItem doesn't take ownership and copies Item
  // delete item;
}

// Answer 2, alternatively, we could consider also doing this
// since player might just be used to fill "values", so no need to allocate
void Game::addItemToPlayer(const std::string &recipient, uint16_t itemId) {
  Player tmpPlayer = {};

  Player *player = g_game.getPlayerByName(recipient);
  if (!player) {
    if (!IOLoginData::loadPlayerByName(&tmpPlayer, recipient)) {
      return;
    }

    player = &tmpPlayer;
  }

  Item *item = Item::CreateItem(itemId);
  if (!item) {
    return;
  }

  g_game.internalAddItem(player->getInbox(), item, INDEX_WHEREEVER,
                         FLAG_NOLIMIT);

  if (player->isOffline()) {
    IOLoginData::savePlayer(player);
  }
}
```

## Second Part Setting Up Everything
This is a big writeup, mostly because I need to set up the server first. Upon reading the docs to setup the server, I need to also setup DB service to run alongside the server, in which TFS wiki recommends using [Uniform Server Zero](https://www.uniformserver.com/), apparently it's a web server which include a DB service and a db management tools. 

So I grabbed and install the latest Uniform Server ZeroXV [here](https://sourceforge.net/projects/miniserver/files). Most of the steps to setup the server are already well explained [here](https://docs.otland.net/ots-guide/running-your-first-ot-server/setting-up-your-first-server) so make sure to follow all steps on explained on the link. 

One thing to note that, you need to use the **schema.sql** on the TFS github repo, I used the schema.sql from 1.4 forgotten server releases. After a succesfull import, your db should look like this.

![TFS Imported Schema](/tfs_imported_schema.png). 

Finally, rename **config.lua.dist** into **config.lua**, and edit **mysqlUser**, **mysqlPass**, and **mysqlDatabase** to the newly created db. Looks like all is good and ready, now try to run the TFS.exe again... but I got this error!

`MySQL Error Message: Plugin caching_sha2_password could not be loaded:`
`The specified module could not be found. Library path is 'caching_sha2_password.dll',` 

Apparently latest MySql use a brand new authentication method that is not supported with TFS 1.4. In order to mitigate this, run query below

`"ALTER USER 'yourusername'@'localhost' IDENTIFIED WITH mysql_native_password BY 'yourpassword';"` 
Change **yourusername** and **yourpassword** with the one you use when setting up mysql.

After changing the default auth, try running the server executables again..
![TFS Server Running](/tfs_running.png) 
TFS server seems to be working correctly now!

No we need to create account and character in order to play the game. Continue following the steps on otland docs, I'm using [Gesior](https://github.com/gesior/Gesior2012/tree/TFS-1.4) since it seems to be the only correct AAC to work with TFS 1.4. After following the necessary create account steps, go ahead and open the www page of your localhost Gesior again, you'll be seeing this page. 
![TFS Create Account](/tfs_create_account.png) 

Go ahead and create an account on that page.

Now it's time to build otclient, checkout otclient repo and build it following instructions [here](https://github.com/edubart/otclient/wiki/Compiling-on-Windows). When trying to compile the build on windows with visual studio 2022 on Release, I got this error 

`Cannot open include file: 'openssl/rsa.h': No such file or directory`	

Since I use vcpkg, this could be easily fixed by installing the package with `vcpkg install openssl-windows:x64-windows`

After a successful build, go ahead and run the OTCClient exe, you'll be seeing this  
![TFS OTClient](/tfs_otpclient.png) 

Use the account name and password that you created on the Gesior page previously, Fill out the server and port (7171), and client version (1098), don't forget to [download the spr and dat files ](https://github.com/EPuncker/1098extended) and put them on the otpclient folder **otpclient/data/things/1098/**

At this point, in order to play the game, you need to run the TFS server locally and use the OTClient to connect to your server, try to familiarize with the UI and the game first.

So while checking on the Q5-Q7, somehow I got the feeling that I should also compile TFS not using prereleased binary, on to try compiling the TFS then.
Instruction to compile TFS with vcpkg seems straightforward [here](https://github.com/otland/forgottenserver/wiki/Compiling-on-Windows-%28vcpkg%29), unfortunately tag v1.4, v1.4.1, and v1.4.2 all fails to compile, only master that could compiled easily, but running master is not suitable since it is using a different protocol with the current OTClient that I use, so I need to make 1.4 works.

There are a couple of issues, one linker error and two compile error. 

The linker error is this
`1>cryptopp.lib(oaep.cpp.obj) : error LNK2001: unresolved external symbol __std_find_trivial_1`, so since TFS doesn't pin the libraries that it is used, vcpkg install will pull the latest version and that latest version doesn't work with TFS 1.4, this might be a bit of trouble in the future since another updated lib could potentially break the build. 

Anyway.. **cryptopp.lib** needs to be pinned to a specific version, but of course [there is no easy way to pin vcpkg library via command line](https://github.com/microsoft/vcpkg/discussions/25622), good job vcpkg. Fix it by adding vcpkg.json on directory vc17, the version that I use for cryptopp.lib is 8.7.0.

Now there are two more compile error from **iomapserialize.cpp** and **iomarket.cpp**, both are coming from a combination of fmt lib and the compiler msvc version that is used, in order to quickly fix this, just replace those two files with the one from [here](https://otland.net/threads/errors-when-compiling-tfs-v1-4x-v1-4-v1-4-1-v1-4-2.288569/post-2749377);

Congrats! Now you could change the server yourself! Now on to finally finishing Q5 to Q7

## Second Part Q5-Q7

### Q5
Creating a spells is quite straightforward, I just need to add new entry on spells.xml and implement the spell with what I wanted. For the **spellid** value I pick it's value that is not conflicting with other already defined **spellid**.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<spells>
	<instant group="attack" spellid="200" name="Wind Tornado" words="frigo" level="1" mana="0" premium="1" selftarget="1" cooldown="10" groupcooldown="10" needlearn="0" script="attack/wind_tornado.lua" />
</spells>
```

For the Lua implementation, I created a big combat area for the spells to target. If I use ICETORNADO, the spells spawned doesn't seem to actually correctly target the combat area, so I need to adjust it one by one. The gist of the logic is **onCastSpell**, it will trigger **onTargetTile** callback in which it would send the magic effect 5 times with random delay with the help of **addEvent**.
```lua
local area = createCombatArea({
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 1, 1, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 0, 1, 0, 1, 1, 0 },
	{ 1, 1, 1, 1, 3, 1, 1, 1, 1 },
	{ 0, 1, 1, 0, 1, 0, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 1, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 0 }
})

function spellCallback(cid, position, count)
	if Creature(cid) then
		if count > 0 or math.random(0, 1) == 1 then
			position:sendMagicEffect(CONST_ME_ICETORNADO)
			doAreaCombat(cid, COMBAT_ICEDAMAGE, position, 0, -10, -10, CONST_ME_ICETORNADO)
		end

		if count < 5 then
			count = count + 1
			addEvent(spellCallback, math.random(500, 1000), cid, position, count)
		end
	end
end

function onTargetTile(creature, position)
	spellCallback(creature:getId(), position, 0)
end

local combat = Combat()
combat:setArea(area)
combat:setCallback(CALLBACK_PARAM_TARGETTILE, "onTargetTile")

function onCastSpell(creature, variant)
	return combat:execute(creature, variant)
end
```

The spells above trigger with **frigo**, to make it easy to test, I set it to level 1 with no mana requirement and low cooldown. See the spells in action on the video below

{{< youtube l7yjoKy8hc0 >}}

### Q6
This is a tough one, since from the example video I need to implement changes on both TFS and the OTClient, on TFS I need a way to hook into movement callback, in OTClient side I need to implement shaders for the mirage effect. In order to test the movement, first I created a spell that would move the player, and according to the player direction, it will then try to move until it cannot move anymore. All seems to be working as expected, but it's a little cumbersome since we need to execute the spell before the movement effect will happen. 

I tried to look around the code to find where I could implement the movement callback, on TFS 1.4 there are no callback that can handle movement of player, so I need to implement one. I went ahead and add a new **EVENT_CALLBACK** on **event_callbacks.lua**. 
```lua
EVENT_CALLBACK_ONMOVE = 19

---

["onMove"] = EVENT_CALLBACK_ONMOVE,
```
adding that and adjusting the rest of the callback values since it's now shifted. After that add a new player event on **events.xml**
```xml
   <event class="Player" method="onMove" enabled="1" />
```

Now on events.h and events.cpp we will need to add a new function entry to handle onMove. 
```cpp
//events.h
    int32_t playerOnMove = -1;
    void eventPlayerOnMove(Player* player, const Position& position, Direction direction);

//events.cpp
void Events::eventPlayerOnMove(Player *player, const Position &position,
                               Direction direction) {
	// Player:onMove(position, direction) or Player.onMove(self,
	// position, direaction)
	if (info.playerOnMove == -1) {
	  return;
	}

	if (!scriptInterface.reserveScriptEnv()) {
	  std::cout << "[Error - Events::eventPlayerOnMove] Call stack overflow"
	            << std::endl;
	  return;
	}

	ScriptEnvironment *env = scriptInterface.getScriptEnv();
	env->setScriptId(info.playerOnMove, &scriptInterface);

	lua_State *L = scriptInterface.getLuaState();
	scriptInterface.pushFunction(info.playerOnMove);

	LuaScriptInterface::pushUserdata<Player>(L, player);
	LuaScriptInterface::setMetatable(L, -1, "Player");

	LuaScriptInterface::pushPosition(L, position);
	lua_pushnumber(L, direction);

	scriptInterface.callVoidFunction(3);
}
```

On the Lua side, we also need to add a new hook to handle onMove, add this changes to player.lua
```lua
function Player:onMove(position, direction)
	if hasEventCallback(EVENT_CALLBACK_ONMOVE) then
		EventCallback(EVENT_CALLBACK_ONMOVE, self, position, direction)
	end
end

```

We then need to call the function on the cpp side to trigger the onMove event, on game.cpp, add the event callback on function internalMoveCreature, we need to also check if it's actually the player that doing the movement.
```cpp
ReturnValue Game::internalMoveCreature(Creature& creature, Tile& toTile, uint32_t flags /*= 0*/)
{
/// omitted for brevity

	map.moveCreature(creature, toTile);

	Player* player = creature.getPlayer();
	if (player)
	{
		Position fromPosition = player->getTile()->getPosition();
		Direction direction = player->getDirection();
		g_events->eventPlayerOnMove(player, fromPosition, direction);
	}
```

Now finally, we need to implement the Lua callback that will handle the onMove callback. Add player_onMove.lua on data\scripts\eventcallbacks\player
```lua
local ec = EventCallback

local outfit = nil

ec.onMove = function(self, position, direction)
	position:getNextPosition(direction)

	local toTile = Tile(position)
	while toTile and toTile:isWalkable() do
		position:sendMagicEffect(CONST_ME_POFF)
		self:teleportTo(position)
		position:getNextPosition(direction)
		toTile = Tile(position)
	end
end

ec:register()
```

For the most part, the movement callback is done, now we move on to implement the shader. 
Unfortunately, I found no easy way to duplicate the effects demonstrated on the example. I tried on the OTClient cpp code calling multiple **internalDrawOutfit()** after a move event but the effect is not on the quality I want to achieve. There should also be a way to do a custom shader via creating a new shader on **shadermanager.cpp**, and invoking it via **sendMagicEffect** from Lua, but I can't find a way to hook up **Effect::drawEffect** to use the shader and access the creature outfit draw buffer to correctly draw it with a different offset and opacity, it all ends down to **rawGetThingType->draw()** which cannot be set with it's type easily. 

So at this point any attempt on the shader would ends up hacky and improper, so after a lot of consideration, I decided to implement it from Lua onMove callback to add some effect. **player_onMove.lua** after the added effect changes would look like this.
```lua
local ec = EventCallback

local outfit = nil

ec.onMove = function(self, position, direction)
	position:getNextPosition(direction)

	local toTile = Tile(position)
	while toTile and toTile:isWalkable() do
		position:sendMagicEffect(CONST_ME_POFF)
		self:teleportTo(position)
		position:getNextPosition(direction)
		toTile = Tile(position)
	end

	outfit = self:getOutfit()
	rainbowEffect(self:getId(), outfit.lookType, 0)
end

ec:register()

function rainbowEffect(cid, lookType, index)
	local creature = Creature(cid)
	if index < 3 then
		local colors = { 94, 77, 79, 82, 87, 90 }
		creature:setOutfit({
			lookType = lookType,
			lookHead = colors[((index) % 6) + 1],
			lookBody = colors[((index + 1) % 6) + 1],
			lookLegs = colors[((index + 2) % 6) + 1],
			lookFeet = colors[((index + 3) % 6) + 1]
		})
		addEvent(rainbowEffect, 50, cid, lookType, index + 1)
	else
		creature:setOutfit(outfit)
	end
end
```

I added rainbowEffect which will set the creature outfit randomly 3 times with different color before finally setting it back to it's initial color.

Video for the result is below.

{{< youtube XvMfxb2GPQU >}}

### Q7
Following tutorial on [here](https://github.com/edubart/otclient/wiki/Module-Tutorial#creating-a-new-module) I was able to create a new module for OTClient. In order to do this, I need to make 3 files **.lua**, **.otmod** and **.otui** files. All of this is done on OTClient side. First, we create the **.otmod** file which is a register files that would hook up the lua script.
```css
Module
  name: client_jumpwindow
  description: Jump Window
  author: ernesernesto
  website: -
  version: 1
  sandboxed: true
  scripts: [ jumpwindow.lua ]
  @onLoad: init()
  @onUnload: terminate()
```
sandboxed mean the script won't pollute the global lua env, so in theory we don't need to local every function declaration / variables. This otmod file would use **jumpwindow.lua**, and at onLoad it will call **init()** and **onUnload** it will call t**erminate()**.

For the **otui** file, it is used as a way to declare your ui, see it as a layouting descriptor files just like css on web. The contents itself is self explanatory. We have a window derived from MainWindow, with window title Jump, and one Button with id **jumpButton** and text Jump. When the button is click it will call **onClickButton()**.

```css
MainWindow
  !text: tr('Jump')
  size: 450 450

  Button
    id: jumpButton
    !text: tr('Jump!')
    width: 90
    @onClick: onClickButton()
```

Finally here is the content of lua file.

```lua
jumpWindow = nil
jumpWindowButton = nil
jumpButton = nil

windowPos = nil
windowSize = nil
buttonSize = nil

function init()
  jumpWindow = g_ui.displayUI('jumpwindow.otui')
  jumpWindow:hide()

  jumpWindowButton = modules.client_topmenu.addLeftButton('jumpWindowButton',
    tr('Jump Window'), '/client_jumpwindow/jumpwindow', toggle)
  jumpButton = jumpWindow:getChildById('jumpButton')

  windowSize = jumpWindow:getSize()
  buttonSize = jumpButton:getSize()

  cycleEvent(updatePos, 50)
end

function terminate()
  jumpWindowButton:destroy()
  jumpWindow = nil
end

function toggle()
  if jumpWindow:isVisible() then
    hide()
  else
    show()
  end
end

function hide()
  jumpWindow:hide()
end

function show()
  jumpWindow:show()
  jumpWindow:raise()
  jumpWindow:focus()
end

function updatePos()
  windowPos = jumpWindow:getPosition()

  pos = jumpButton:getPosition()
  pos.x = pos.x - 10

  if pos.x < windowPos.x then
    pos.x = windowPos.x + windowSize.width - buttonSize.width
    pos.y = pos.y - 50
  end

  pos.y = clampButtonYPos(pos.y, windowPos.y)

  jumpButton:setPosition(pos)
end

function onClickButton()
  windowPos = jumpWindow:getPosition()

  pos = jumpButton:getPosition()
  pos.y = pos.y - 100
  pos.y = clampButtonYPos(pos.y, windowPos.y)

  jumpButton:setPosition(pos)
end

function clampButtonYPos(yPos, windowYPos)
  if yPos - 100 < windowYPos then
    yPos = windowYPos + windowSize.height - 50
  end

  return yPos
end
```
First, we need a way to show the jumpWindow window on the main OTClient. This is done by calling 
```lua
  jumpWindowButton = modules.client_topmenu.addLeftButton('jumpWindowButton', tr('Jump Window'), '/client_jumpwindow/jumpwindow', toggle)
```
Clicking the jumpWindowButton would then call **toggle** function.
We will implement **onClickButton** hook when the jump button inside jumpWindow is clicked. We do this by calling 
```lua
  jumpButton:setPosition()
```

Finally, to make the jumpButton update, we use **cycleEvent** which will update the position of the button on 50ms interval.
To view the result in action, see video below

{{< youtube p9QPxIn2PzQ >}}


Thank you for reading this far! For complete code changes history, please see the github repo [here](https://github.com/ernesernesto/tlg)
