# About HeeksCAD
HeeksCAD is a free and open source cross-platform 3D CAD application created by Dan Heeks. The solid modeling is provided by Open CASCADE; the graphical user interface is made using wxWidgets.

# Features
* Import solid models or drawings from STEP, IGES, DXF...
* Save to IGES, STEP, STL, HPGL or even G-Code with the HeeksCNC plugin (see [HeeksCNC repository] (https://github.com/Heeks/heekscnc) for more information)
* Draw construction geometry (lines and arcs).
* Create new primitive solids or make solids by extruding a sketch or by making a lofted solid between sketches.
* Modify solids using blending, or boolean operations.
* Use the geometric constraints solver to create accurate drawings from rough sketches.

# Installing
The latest binaries can soon be downloaded from https://github.com/Heeks/heekscad/downloads
However, as HeeksCAD is rapidly progressing, you might be better off building the application yourself from source.

# Building
Instructions are in the Wiki for:
* [Windows] (https://github.com/Heeks/heekscad/wiki)
* [Ubuntu] (https://github.com/Heeks/heekscad/wiki)
* [Debian] (https://github.com/Heeks/heekscad/wiki)
* [OpenSUSE] (https://github.com/Heeks/heekscad/wiki)
* [Fedora]()

# Next steps

## Using HeeksCAD
See usage strategies in [Wiki] (https://github.com/Heeks/heekscad/wiki)

## How to deal with bugs
Yes, despite its looks, the software is not finished yet. If you encounter a bug please report it at...
* ..the [github issue tracker] (https://github.com/Heeks/heekscad/issues)
* ..and/or join #heekscad on irc.freenode.net
* ..or better yet, get your hands dirty and read the next point. :-)

## Develop
Help develop HeeksCAD by forking and sending in pull requests - everyone profits from it!

# Add-On repositories
* [HeeksCNC] (https://github.com/Heeks/heekscnc): For generating G-Code
* [HeeksArts] (https://github.com/Heeks/heeksart): Adds free-form Bezier triangle meshes, to create smooth and natural shapes
* [HeeksPython] (https://github.com/Heeks/heekspython): A plugin to enable python interaction in HeeksCAD. Users can script actions, create/modify objects, and create parametric geometry. 

# Translations
Currently, HeeksCAD has been (partially) translated into these languages: German, French, Italian, Slovak, Korean and Chinese.
Please fork and help making those translations perfect!

# Donations
Donations are welcome, if you enjoy the software.

# License
New BSD License. This means you can take all my work and use it for your own commercial application. Do what you want with it. Any changes to wxWidgets or Open CASCADE are subject to their licenses.
