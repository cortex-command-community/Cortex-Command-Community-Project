# Self-Learning Ronin

Adds the **Ronin Learning Defense** activity (pick it in the Activities menu, 1 human player vs. 1 CPU team).

You place your brain, and a Ronin commander appears on the far side of the map with a small garrison. From then on
the AI defends its commander, sends squads out to destroy you, and learns from every fight.

## Game version

`Index.ini` contains `SupportedGameVersion = 7.0.0-dev`, which is what this repository builds (see `meson.build`).
The engine refuses mods whose version does not match a development build exactly. If you play a different build,
change that line to the version shown in the game (for a release build `X.Y.0`, with X the major version and Y the
lowest minor version the mod needs).

## What the AI learns

Every decision is learned separately for 9 situations (how many soldiers you have: 0-2 / 3-5 / 6+, and how far away
your brain is: near / mid / far):

| Decision | Options |
|----------|---------|
| Sortie target | your brain, nearest soldier, weakest soldier, or either of the first two approached from behind |
| Squad size | 2 - 6 |
| Retreat point | never, at 34% survivors, at 60% survivors |
| Heavy / light mix | 0%, 34%, 67%, 100% heavies |
| Weapon group | primary, CQB, heavy, light, explosive, sniper |
| Defense | 30% / 50% / 70% / 90% of the army never leaves the base |

After every sortie the result (damage dealt, kills, own losses) is scored and written into the AI's memory. The
result of the whole match is added as a weak extra signal. The AI explores a lot at first and gradually settles on
what works, but never stops exploring completely, so it keeps adapting if you change your tactics.
Learning is gradual: every sortie is one data point, so the more (and longer) you play, the better it gets.

The console (`~`) prints one line per sortie so you can watch it learn.

## Memory

Stored in `SelfLearningRonin.rte/Memory.txt` (plain text, ignored by git). Delete it to reset the AI,
copy it to give the same trained AI to a friend.

The difficulty slider changes the AI's income and how many soldiers it can field.

---

## По-русски

Новый режим **Ronin Learning Defense**. Ты ставишь мозг, на другом конце карты появляется командир ронинов
с гарнизоном. ИИ защищает командира, отправляет отряды уничтожить тебя и учится на каждой вылазке:
цель, размер отряда, момент отступления, тяжёлые/лёгкие бойцы, оружие и доля армии, которая остаётся
защищать базу. Память лежит в `SelfLearningRonin.rte/Memory.txt`; удалишь файл, ИИ начнёт с нуля.
Обучение постепенное: каждая вылазка это один опыт, поэтому чем больше и дольше играешь, тем сильнее ИИ.
